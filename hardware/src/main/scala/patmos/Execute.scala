/*
 * Execution stage of Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._

import Constants._

class Execute() extends Module {
  val io = IO(new ExecuteIO())

  val enableCop = Wire(Bool(), true.B)
  io.ena_out := enableCop

  val exReg = Reg(new DecEx())
  when(enableCop && io.ena_in) {
    exReg := io.decex
    when(io.flush || io.brflush) {
      exReg.flush()
      exReg.relPc := io.decex.relPc
    }
  }

  def alu(func: UInt, op1: UInt, op2: UInt): UInt = {
    val result = Wire(UInt(width = DATA_WIDTH))
    val scaledOp1 = op1 << Mux(func === FUNC_SHADD2, 2.U,
                               Mux(func === FUNC_SHADD, 1.U, 0.U))
    val sum = scaledOp1 + op2
    result := sum // some default
    val shamt = op2(4, 0).asUInt
    val srOp = Mux(func === FUNC_SRA, op1(DATA_WIDTH-1), 0.U) ## op1
    // This kind of decoding of the ALU op in the EX stage is not efficient,
    // but we keep it for now to get something going soon.
    switch(func) {
      is(FUNC_ADD)    { result := sum }
      is(FUNC_SUB)    { result := op1 - op2 }
      is(FUNC_XOR)    { result := (op1 ^ op2).asUInt }
      is(FUNC_SL)     { result := (op1 << shamt)(DATA_WIDTH-1, 0).asUInt }
      is(FUNC_SR, FUNC_SRA) { result := (srOp.asSInt >> shamt).asUInt }
      is(FUNC_OR)     { result := (op1 | op2).asUInt }
      is(FUNC_AND)    { result := (op1 & op2).asUInt }
      is(FUNC_NOR)    { result := (~(op1 | op2)).asUInt }
      is(FUNC_SHADD)  { result := sum }
      is(FUNC_SHADD2) { result := sum }
    }
    result
  }

  def comp(func: UInt, op1: UInt, op2: UInt): Bool = {
    val op1s = op1.asSInt
    val op2s = op2.asSInt
    val bitMsk = UInt(1) << op2(4, 0).asUInt
    // Is this nicer than the switch?
    // Some of the comparison function (equ, subtract) could be shared
    val eq = op1 === op2
    val lt = op1s < op2s
    val ult = op1 < op2
    MuxLookup(func.asUInt, Bool(false), Array(
      (CFUNC_EQ,    eq),
      (CFUNC_NEQ,   !eq),
      (CFUNC_LT,    lt),
      (CFUNC_LE,    lt | eq),
      (CFUNC_ULT,   ult),
      (CFUNC_ULE,   ult | eq),
      (CFUNC_BTEST, (op1 & bitMsk) =/= 0.U)))
  }

  def pred(func: UInt, op1: Bool, op2: Bool): Bool = {
    MuxLookup(func.asUInt, Bool(false), Array(
      (PFUNC_OR, op1 | op2),
      (PFUNC_AND, op1 & op2),
      (PFUNC_XOR, op1 ^ op2),
      (PFUNC_NOR, ~(op1 | op2))))
  }

  // data forwarding
  val fwReg  = Reg(Vec(2*PIPE_COUNT, UInt(3.W)))
  val fwSrcReg  = Reg(Vec(2*PIPE_COUNT, UInt(width = log2Up(PIPE_COUNT))))
  val memResultDataReg = Reg(Vec(PIPE_COUNT, UInt(DATA_WIDTH.W)))
  val exResultDataReg  = Reg(Vec(PIPE_COUNT, UInt(DATA_WIDTH.W)))
  val op = Wire(Vec(2*PIPE_COUNT, UInt(DATA_WIDTH.W)))

  // precompute forwarding
  for (i <- 0 until 2*PIPE_COUNT) {
    fwReg(i) := "b000".U(3.W)
    fwSrcReg(i) := 0.U
    for (k <- 0 until PIPE_COUNT) {
      when(io.decex.rsAddr(i) === io.memResult(k).addr && io.memResult(k).valid) {
        fwReg(i) := "b010".U(3.W)
        fwSrcReg(i) := UInt(k)
      }
    }
    for (k <- 0 until PIPE_COUNT) {
      when(io.decex.rsAddr(i) === io.exResult(k).addr && io.exResult(k).valid) {
        fwReg(i) := "b001".U(3.W)
        fwSrcReg(i) := UInt(k)
      }
    }    
  }
  for (i <- 0 until PIPE_COUNT) {
    when(io.decex.immOp(i)) {
      fwReg(2*i+1) := "b100".U(3.W)
    }
  }

  when (!enableCop || !io.ena_in) {
    fwReg := fwReg
    fwSrcReg := fwSrcReg
  }
  when (enableCop && io.ena_in) {
    memResultDataReg := io.memResult.map(_.data)
    exResultDataReg := io.exResult.map(_.data)
  }

  // forwarding multiplexers
  for (i <- 0 until PIPE_COUNT) {
    op(2*i) := Mux(fwReg(2*i)(0), exResultDataReg(fwSrcReg(2*i)),
                   Mux(fwReg(2*i)(1), memResultDataReg(fwSrcReg(2*i)),
                       exReg.rsData(2*i)))

    op(2*i+1) := Mux(fwReg(2*i+1)(0), exResultDataReg(fwSrcReg(2*i+1)),
                     Mux(fwReg(2*i+1)(1), memResultDataReg(fwSrcReg(2*i+1)),
                         Mux(fwReg(2*i+1)(2), exReg.immVal(i),
                             exReg.rsData(2*i+1))))
  }

  // predicates
  val predReg = Reg(Vec(PRED_COUNT, Bool()))

  val doExecute = Wire(Vec(PIPE_COUNT, Bool()))
  for (i <- 0 until PIPE_COUNT) {
    doExecute(i) := Mux(io.flush, false.B,
                        predReg(exReg.pred(i)(PRED_BITS-1, 0)) ^ exReg.pred(i)(PRED_BITS))
  }

  // return information
  val retBaseReg = Reg(UInt(DATA_WIDTH.W))
  val retOffReg = Reg(UInt(DATA_WIDTH.W))
  val saveRetOff = Reg(Bool())
  val saveND = Reg(Bool())

  // exception return information
  val excBaseReg = Reg(UInt(width = DATA_WIDTH))
  val excOffReg = Reg(UInt(width = DATA_WIDTH))

  // MS: maybe the multiplication should be in a local component?

  // multiplication result registers
  val mulLoReg = Reg(UInt(width = DATA_WIDTH))
  val mulHiReg = Reg(UInt(width = DATA_WIDTH))

  // multiplication pipeline registers
  val mulLLReg    = Reg(UInt(width = DATA_WIDTH))
  val mulLHReg    = Reg(SInt(width = DATA_WIDTH+1))
  val mulHLReg    = Reg(SInt(width = DATA_WIDTH+1))
  val mulHHReg    = Reg(UInt(width = DATA_WIDTH))

  val mulPipeReg = Reg(Bool())

  // multiplication only in first pipeline
  when(io.ena_in) {
    mulPipeReg := exReg.aluOp(0).isMul && doExecute(0)

    val signed = exReg.aluOp(0).func === MFUNC_MUL

    val op1H = Cat(Mux(signed, op(0)(DATA_WIDTH-1), 0.U(1.W)),
                   op(0)(DATA_WIDTH-1, DATA_WIDTH/2)).asSInt
    val op1L = op(0)(DATA_WIDTH/2-1, 0)
    val op2H = Cat(Mux(signed, op(1)(DATA_WIDTH-1), 0.U(1.W)),
                   op(1)(DATA_WIDTH-1, DATA_WIDTH/2)).asSInt
    val op2L = op(1)(DATA_WIDTH/2-1, 0)

    mulLLReg := op1L * op2L
    mulLHReg := op1L * op2H
    mulHLReg := op1H * op2L
    mulHHReg := (op1H * op2H).asUInt

    val mulResult = (Cat(mulHHReg, mulLLReg).asSInt
                     + Cat(mulHLReg, SInt(0, width = DATA_WIDTH/2)).asSInt
                     + Cat(mulLHReg, SInt(0, width = DATA_WIDTH/2)).asSInt)

    when(mulPipeReg) {
      mulHiReg := mulResult(2*DATA_WIDTH-1, DATA_WIDTH)
      mulLoReg := mulResult(DATA_WIDTH-1, 0)
    }
  }

  // interface to the stack cache
  io.exsc.op := sc_OP_NONE
  io.exsc.opData := 0.U
  io.exsc.opOff := Mux(exReg.immOp(0), exReg.immVal(0), op(0))

  // stack control instructions
  when(!io.brflush && doExecute(0)) {
    io.exsc.op := exReg.stackOp
  }

  // dual-issue operations
  for (i <- 0 until PIPE_COUNT) {

    val aluResult = alu(exReg.aluOp(i).func, op(2*i), op(2*i+1))
    val compResult = comp(exReg.aluOp(i).func, op(2*i), op(2*i+1))

    val bcpyPs = predReg(exReg.aluOp(i).func(PRED_BITS-1, 0)) ^ exReg.aluOp(i).func(PRED_BITS);
    val shiftedPs = ((UInt(0, DATA_WIDTH-1) ## bcpyPs) << op(2*i+1)(4, 0))(DATA_WIDTH-1, 0)
    val maskedOp = op(2*i) & ~(UInt(1, width = DATA_WIDTH) << op(2*i+1)(4, 0))(DATA_WIDTH-1, 0)
    val bcpyResult = maskedOp | shiftedPs

    // predicate operations
    val ps1 = predReg(exReg.predOp(i).s1Addr(PRED_BITS-1,0)) ^ exReg.predOp(i).s1Addr(PRED_BITS)
    val ps2 = predReg(exReg.predOp(i).s2Addr(PRED_BITS-1,0)) ^ exReg.predOp(i).s2Addr(PRED_BITS)
    val predResult = pred(exReg.predOp(i).func, ps1, ps2)

    when((exReg.aluOp(i).isCmp || exReg.aluOp(i).isPred) && doExecute(i)) {
      predReg(exReg.predOp(i).dest) := Mux(exReg.aluOp(i).isCmp, compResult, predResult)
    }
    predReg(0) := Bool(true)

    // special registers
    when(exReg.aluOp(i).isMTS && doExecute(i)) {
      io.exsc.opData := op(2*i)

      switch(exReg.aluOp(i).func) {
        is(SPEC_FL) {
          for (j <- 0 until PRED_COUNT) {
            predReg(j) := op(2*i)(j)
          }
          //predReg := op(2*i)(PRED_COUNT-1, 0)
          predReg(0) := Bool(true)
        }
        is(SPEC_SL) {
          mulLoReg := op(2*i)
        }
        is(SPEC_SH) {
          mulHiReg := op(2*i)
        }
        is(SPEC_ST) {
          io.exsc.op := sc_OP_SET_ST
        }
        is(SPEC_SS) {
          io.exsc.op := sc_OP_SET_MT
        }
        is(SPEC_SRB) {
          retBaseReg := op(2*i)
        }
        is(SPEC_SRO) {
          retOffReg := op(2*i)
        }
        is(SPEC_SXB) {
          excBaseReg := op(2*i)
        }
        is(SPEC_SXO) {
          excOffReg := op(2*i)
        }
      }
    }
    val mfsResult = Wire(UInt())
    mfsResult := 0.U(DATA_WIDTH.W)
    switch(exReg.aluOp(i).func) {
      is(SPEC_FL) {
        mfsResult := Cat(0.U((DATA_WIDTH-PRED_COUNT).W), predReg.asUInt()).asUInt()
      }
      is(SPEC_SL) {
        mfsResult := mulLoReg
      }
      is(SPEC_SH) {
        mfsResult := mulHiReg
      }
      is(SPEC_ST) {
        mfsResult := io.scex.stackTop
      }
      is(SPEC_SS) {
        mfsResult := io.scex.memTop
      }
      is(SPEC_SRB) {
        mfsResult := retBaseReg
      }
      is(SPEC_SRO) {
        mfsResult := retOffReg
      }
      is(SPEC_SXB) {
        mfsResult := excBaseReg
      }
      is(SPEC_SXO) {
        mfsResult := excOffReg
      }
    }

    // result
    io.exmem.rd(i).addr := exReg.rdAddr(i)
    io.exmem.rd(i).valid := exReg.wrRd(i) && doExecute(i)
    io.exmem.rd(i).data := Mux(exReg.aluOp(i).isMFS, mfsResult,
                               Mux(exReg.aluOp(i).isBCpy, bcpyResult,
                                   aluResult))
  }

  // load/store
  io.exmem.mem.load := exReg.memOp.load && doExecute(0)
  io.exmem.mem.store := exReg.memOp.store && doExecute(0)
  io.exmem.mem.hword := exReg.memOp.hword
  io.exmem.mem.byte := exReg.memOp.byte
  io.exmem.mem.zext := exReg.memOp.zext
  io.exmem.mem.typ := exReg.memOp.typ
  io.exmem.mem.addr := op(0) + exReg.immVal(0)
  io.exmem.mem.data := op(1)

  // call/return
  io.exmem.mem.call := exReg.call && doExecute(0)
  io.exmem.mem.ret  := exReg.ret && doExecute(0)
  io.exmem.mem.brcf := exReg.brcf && doExecute(0)
  io.exmem.mem.trap := exReg.trap && doExecute(0)
  io.exmem.mem.xcall := exReg.xcall && doExecute(0)
  io.exmem.mem.xret := exReg.xret && doExecute(0)
  io.exmem.mem.xsrc := exReg.xsrc
  io.exmem.mem.nonDelayed := exReg.nonDelayed
  io.exmem.mem.illOp := exReg.illOp

  val doCallRet = (exReg.call || exReg.ret || exReg.brcf ||
                   exReg.xcall || exReg.xret) && doExecute(0)

  val brcfOff = Mux(exReg.immOp(0), 0.U, op(1).asUInt)
  val callRetAddr = Mux(exReg.call || exReg.xcall, UInt(0),
                        Mux(exReg.brcf, brcfOff,
                            Mux(exReg.xret, excOffReg, retOffReg)))

  val callBase = Mux(exReg.immOp(0), exReg.callAddr, op(0).asUInt)
  val callRetBase = Mux(exReg.call || exReg.xcall || exReg.brcf, callBase,
                        Mux(exReg.xret, excBaseReg, retBaseReg))

  io.exmem.mem.callRetBase := callRetBase
  io.exmem.mem.callRetAddr := callRetAddr

  // return information
  when(exReg.call && doExecute(0)) {
    retBaseReg := Cat(exReg.base, 0.U(2.W))
  }
  // the offset is saved when the call is already in the MEM statge
  saveRetOff := exReg.call && doExecute(0) && io.ena_in
  saveND := exReg.nonDelayed

  // exception return information
  when(exReg.xcall && doExecute(0)) {
    excBaseReg := Cat(exReg.base, 0.U(2.W))
    excOffReg := Cat(exReg.relPc, 0.U(2.W))
  }

  // branch
  io.exfe.doBranch := exReg.jmpOp.branch && doExecute(0)
  val target = Mux(exReg.immOp(0),
                   exReg.jmpOp.target,
                   op(0)(DATA_WIDTH-1, 2).asUInt - exReg.jmpOp.reloc)
  io.exfe.branchPc := target
  io.brflush := exReg.nonDelayed && exReg.jmpOp.branch && doExecute(0)

  // pass on PC
  io.exmem.pc := exReg.pc
  io.exmem.base := exReg.base
  io.exmem.relPc := exReg.relPc

  //call/return for icache
  io.exicache.doCallRet := doCallRet
  io.exicache.callRetBase := callRetBase(31,2)
  io.exicache.callRetAddr := callRetAddr(31,2)

  // coprocessor handling
  if (COP_COUNT > 0) {
    val copStartedReg = RegInit(false.B)
    io.copOut.map(_.defaults())
    when(!io.flush && doExecute(0)) {
      when(exReg.copOp.isCop) {
        io.copOut(exReg.copOp.copId).ena_in := io.ena_in
        io.copOut(exReg.copOp.copId).trigger := io.ena_in && !copStartedReg
        io.copOut(exReg.copOp.copId).isCustom := exReg.copOp.isCustom
        io.copOut(exReg.copOp.copId).read := exReg.wrRd(0)
        io.copOut(exReg.copOp.copId).funcId := exReg.copOp.funcId
        io.copOut(exReg.copOp.copId).opAddr(0) := exReg.rsAddr(0)
        io.copOut(exReg.copOp.copId).opAddr(1) := exReg.rsAddr(1)
        io.copOut(exReg.copOp.copId).opData(0) := op(0)
        io.copOut(exReg.copOp.copId).opData(1) := op(1)
        io.copOut(exReg.copOp.copId).opAddrCop(0) := exReg.copOp.rsAddrCop(0)
        io.copOut(exReg.copOp.copId).opAddrCop(1) := exReg.copOp.rsAddrCop(1)
    
        enableCop := io.copIn(exReg.copOp.copId).ena_out
        io.exmem.rd(0).data := io.copIn(exReg.copOp.copId).result

        when(io.ena_in) {
          copStartedReg := true.B
          when(enableCop) {
            copStartedReg := false.B
          }
        }
      }
    }
  }

  // suppress writes to special registers
  when(!enableCop || !io.ena_in) {
    predReg := predReg
    mulLoReg := mulLoReg
    mulHiReg := mulHiReg
    retBaseReg := retBaseReg
    retOffReg := retOffReg
    excBaseReg := excBaseReg
    excOffReg := excOffReg
  }

  // saveRetOff overrides io.ena_in for writes to retOffReg
  when(saveRetOff) {
    retOffReg := Cat(Mux(saveND, exReg.relPc, io.feex.pc), 0.U(2.W))
  }

  // reset at end to override any computations
  when(reset) {
    exReg.flush()
    predReg(0) := true.B
  }
}

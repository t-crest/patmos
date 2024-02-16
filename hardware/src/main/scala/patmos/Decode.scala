/*
 * Decode stage of Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import chisel3._
import chisel3.util._

import Constants._

class Decode() extends Module {
  val io = IO(new DecodeIO())



  val rf = Module(new RegisterFile())
  // register file is connected with unregistered instruction word
  rf.io.rfRead.rsAddr(0) := io.fedec.instr_a(16, 12)
  rf.io.rfRead.rsAddr(1) := io.fedec.instr_a(11, 7)
  if (PIPE_COUNT > 1) {
    rf.io.rfRead.rsAddr(2) := io.fedec.instr_b(16, 12)
    rf.io.rfRead.rsAddr(3) := io.fedec.instr_b(11, 7)
  }
  rf.io.ena := io.ena
  // RF write from write back stage
  rf.io.rfWrite <> io.rfWrite

  // register input from fetch stage
  val decReg = Reg(new FeDec())
  when(io.ena) {
    decReg := io.fedec
    when(io.flush) {
      decReg.flush()
      decReg.relPc := io.fedec.relPc
    }
  }

  // default values
  io.decex.defaults()

  // forward RF addresses and data
  io.decex.rsAddr(0) := decReg.instr_a(16, 12)
  io.decex.rsAddr(1) := decReg.instr_a(11, 7)
  if (PIPE_COUNT > 1) {
    io.decex.rsAddr(2) := decReg.instr_b(16, 12)
    io.decex.rsAddr(3) := decReg.instr_b(11, 7)
  }

  io.decex.rsData(0) := rf.io.rfRead.rsData(0)
  io.decex.rsData(1) := rf.io.rfRead.rsData(1)
  if (PIPE_COUNT > 1) {
    io.decex.rsData(2) := rf.io.rfRead.rsData(2)
    io.decex.rsData(3) := rf.io.rfRead.rsData(3)
  }

  val decoded = Wire(Vec(PIPE_COUNT, Bool()))
  decoded := VecInit(Seq.fill(PIPE_COUNT)(false.B))

  // Decoding of dual-issue operations
  val dual = decReg.instr_a(INSTR_WIDTH - 1) && decReg.instr_a(26, 22) =/= OPCODE_ALUL;
  for (i <- 0 until PIPE_COUNT) {
    val instr   = if (i == 0) { decReg.instr_a } else { decReg.instr_b }
    val opcode  = instr(26, 22)
    val opc     = instr(6, 4)
    val isValid = if (i == 0) { true.B } else { dual }

    val immVal = Wire(UInt())
    // Default value for immediates
    immVal := Cat(0.U, instr(11, 0))

    // ALU register
    io.decex.aluOp(i).func := instr(3, 0)

    // ALU immediate
    when(opcode(4, 3) === OPCODE_ALUI) {
      io.decex.aluOp(i).func := Cat(0.U, instr(24, 22))
      io.decex.immOp(i) := isValid
      io.decex.wrRd(i) := isValid
      decoded(i) := true.B
    }
    // Other ALU
    when(opcode === OPCODE_ALU) {
      switch(opc) {
        is(OPC_ALUR) {
          io.decex.wrRd(i) := isValid
          decoded(i) := true.B
        }
        is(OPC_ALUU) {
          io.decex.wrRd(i) := isValid
          decoded(i) := true.B
        }
        is(OPC_ALUM) {
          io.decex.aluOp(i).isMul := isValid
          decoded(i) := true.B
        }
        is(OPC_ALUC) {
          io.decex.aluOp(i).isCmp := isValid
          decoded(i) := true.B
        }
        is(OPC_ALUCI) {
          io.decex.aluOp(i).isCmp := isValid
          io.decex.immOp(i) := isValid
          immVal := Cat(0.U, instr(11, 7))
          decoded(i) := true.B
        }
        is(OPC_ALUP) {
          io.decex.aluOp(i).isPred := isValid
          decoded(i) := true.B
        }
        is(OPC_ALUB) {
          io.decex.wrRd(i) := isValid
          io.decex.aluOp(i).isBCpy := isValid
          io.decex.immOp(i) := isValid
          immVal := Cat(0.U, instr(11, 7))
          decoded(i) := true.B
        }
      }
    }
    // Special registers
    when(opcode === OPCODE_SPC) {
      switch(opc) {
        is(OPC_MTS) {
          io.decex.aluOp(i).isMTS := isValid
          decoded(i) := true.B
        }
        is(OPC_MFS) {
          io.decex.aluOp(i).isMFS := isValid
          io.decex.wrRd(i) := isValid
          decoded(i) := true.B
        }
      }
    }

    // Default immediate value
    io.decex.immVal(i) := immVal

    // Predicates
    io.decex.predOp(i).func := Cat(instr(3), instr(0))
    io.decex.predOp(i).s1Addr := instr(15, 12)
    io.decex.predOp(i).s2Addr := instr(10, 7)
    io.decex.predOp(i).dest := instr(19, 17)
    io.decex.pred(i) := instr(30, 27)

    // Default destination
    io.decex.rdAddr(i) := instr(21, 17)
  }

  // Decoding of additional operations for first pipeline
  val instr = decReg.instr_a
  val opcode = instr(26, 22)
  val func = instr(3, 0)

  val ldsize = instr(11, 9)
  val ldtype = instr(8, 7)
  val stsize = instr(21, 19)
  val sttype = instr(18, 17)
  val stcfun = instr(21, 18)

  val dest = Wire(UInt(REG_BITS.W))
  val longImm = Wire(Bool())

  val isMem = Wire(Bool())
  val isStack = Wire(Bool())

  val isSTC = Wire(Bool())
  val stcImm = Cat(0.U, instr(17, 0), 0.U(2.W))

  // Long immediates set this
  longImm := false.B

  // Load/stores and stack control operations set this
  isMem := false.B
  isStack := false.B
  isSTC := false.B

  // Everything except calls uses the default
  dest := instr(21, 17)

  // ALU long immediate (Bit 31 is set as well)
  when(opcode === OPCODE_ALUL && instr(6, 4) === 0.U) {
    io.decex.aluOp(0).func := func
    io.decex.immOp(0) := true.B
    longImm := true.B
    io.decex.wrRd(0) := true.B
    decoded(0) := true.B
  }
  // Stack control
  when(opcode === OPCODE_STC) {
    switch(stcfun) {
      is(STC_SRES) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_RES
        io.decex.immOp(0) := true.B
        decoded(0) := true.B
      }
      is(STC_SENS) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_ENS
        io.decex.immOp(0) := true.B
        decoded(0) := true.B
      }
      is(STC_SENSR) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_ENS
        decoded(0) := true.B
      }
      is(STC_SFREE) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_FREE
        io.decex.immOp(0) := true.B
        decoded(0) := true.B
      }
      is(STC_SSPILL) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_SPILL
        io.decex.immOp(0) := true.B
        decoded(0) := true.B
      }
      is(STC_SSPILLR) {
        isSTC := true.B
        io.decex.stackOp := sc_OP_SPILL
        decoded(0) := true.B
      }
    }
  }
  
  // Control-flow operations
  when(opcode === OPCODE_CFL_TRAP) {
    io.decex.trap := true.B
    io.decex.xsrc := instr(EXC_SRC_BITS-1, 0)
    decoded(0) := true.B
  }
  when(opcode === OPCODE_CFL_CALL || opcode === OPCODE_CFL_CALLND) {
    io.decex.immOp(0) := true.B
    io.decex.call := true.B
    io.decex.nonDelayed := opcode === OPCODE_CFL_CALLND
    decoded(0) := true.B
  }
  when(opcode === OPCODE_CFL_BR || opcode === OPCODE_CFL_BRND) {
    io.decex.immOp(0) := true.B
    io.decex.jmpOp.branch := true.B
    io.decex.nonDelayed := opcode === OPCODE_CFL_BRND
    decoded(0) := true.B
  }
  when(opcode === OPCODE_CFL_BRCF || opcode === OPCODE_CFL_BRCFND) {
    io.decex.immOp(0) := true.B
    io.decex.brcf := true.B
    io.decex.nonDelayed := opcode === OPCODE_CFL_BRCFND
    decoded(0) := true.B
  }
  when(opcode === OPCODE_CFL_CFLR || opcode === OPCODE_CFL_CFLRND) {
    switch(func) {
      is(JFUNC_RET) {
        io.decex.ret := true.B
        decoded(0) := true.B
      }
      is(JFUNC_XRET) {
        io.decex.xret := true.B
        decoded(0) := true.B
      }
      is(JFUNC_CALL) {
        io.decex.call := true.B
        decoded(0) := true.B
      }
      is(JFUNC_BR) {
        io.decex.jmpOp.branch := true.B
        decoded(0) := true.B
      }
      is(JFUNC_BRCF) {
        io.decex.brcf := true.B
        decoded(0) := true.B
      }
    }
    io.decex.nonDelayed := opcode === OPCODE_CFL_CFLRND
  }

  val shamt = Wire(UInt())
  shamt := 0.U
  // load
  when(opcode === OPCODE_LDT) {
    isMem := true.B
    io.decex.memOp.load := true.B
    io.decex.wrRd(0) := true.B
    switch(ldsize) {
      is(MSIZE_W) {
        shamt := 2.U
      }
      is(MSIZE_H) {
        shamt := 1.U
        io.decex.memOp.hword := true.B
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := true.B
      }
      is(MSIZE_HU) {
        shamt := 1.U
        io.decex.memOp.hword := true.B
        io.decex.memOp.zext := true.B
      }
      is(MSIZE_BU) {
        io.decex.memOp.byte := true.B
        io.decex.memOp.zext := true.B
      }
    }
    io.decex.memOp.typ := ldtype;
    when(ldtype === MTYPE_C && io.exc.local) {
      io.decex.memOp.typ := MTYPE_L
    }
    when(ldtype === MTYPE_S) {
      isStack := true.B
    }
    decoded(0) := true.B
  }
  // store
  when(opcode === OPCODE_STT) {
    isMem := true.B
    io.decex.memOp.store := true.B
    switch(stsize) {
      is(MSIZE_W) {
        shamt := 2.U
      }
      is(MSIZE_H) {
        shamt := 1.U
        io.decex.memOp.hword := true.B
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := true.B
      }
    }
    io.decex.memOp.typ := sttype;
    when(sttype === MTYPE_C && io.exc.local) {
      io.decex.memOp.typ := MTYPE_L
    }
    when(sttype === MTYPE_S) {
      isStack := true.B
    }
    decoded(0) := true.B
  }

  if(COP_COUNT > 0)
  {
    when(opcode === OPCODE_COP)
    {
      val copId = instr(6, 4)
      io.decex.copOp.isCop := true.B

      //custom
      when(instr(0) === COP_CUSTOM_BIT)
      {
        io.decex.copOp.isCustom := true.B
        io.decex.copOp.copId := copId
        io.decex.copOp.funcId := Cat("b00".U , instr(3, 1))
        io.decex.wrRd(0) := true.B
        decoded(0) := true.B
      }.otherwise
      {
        when(instr(1) === COP_READ_BIT)
        {
          //read
          io.decex.copOp.rsAddrCop(0) := instr(3)
          io.decex.copOp.copId := copId
          io.decex.copOp.funcId := instr(11, 7)
          io.decex.wrRd(0) := true.B
          decoded(0) := true.B
        }.otherwise
        {
          //write
          io.decex.copOp.rsAddrCop(0) := instr(3)
          io.decex.copOp.rsAddrCop(1) := instr(2)
          io.decex.copOp.copId := copId
          io.decex.copOp.funcId := instr(21, 17)
          io.decex.wrRd(0) := false.B
          decoded(0) := true.B
        }
      }
    }
  }

  // Offset for loads/stores
  val addrImm = Wire(UInt())
  addrImm := Cat(0.U, instr(6, 0))
  switch(shamt) {
    is(1.U) { addrImm := Cat(0.U, instr(6, 0), 0.U(1.W)) }
    is(2.U) { addrImm := Cat(0.U, instr(6, 0), 0.U(2.W)) }
  }

  // Non-default immediate value
  when (isSTC || isStack || isMem || longImm) {
    io.decex.immVal(0) := Mux(isSTC, stcImm,
                              Mux(isStack, addrImm,
                                  Mux(isMem, addrImm,
                                      decReg.instr_b)))
  }
  // we could mux the imm / register here as well

  // Immediate for absolute calls
  io.decex.callAddr := Cat(0.U, instr(21, 0), 0.U(2.W))

  // Immediate for branch is sign extended, not extended for call
  // PC-relative value is precomputed here
  io.decex.jmpOp.target := decReg.pc + Cat(Fill(PC_SIZE - 22, instr(21)), instr(21, 0))
  io.decex.jmpOp.reloc := decReg.reloc

  // Pass on PC
  io.decex.pc := decReg.pc
  io.decex.base := decReg.base
  io.decex.relPc := decReg.relPc

  // Set destination address
  io.decex.rdAddr(0) := dest

  // Disable register write on register 0
  for (i <- 0 until PIPE_COUNT) {
    when(io.decex.rdAddr(i) === "b00000".U) {
      io.decex.wrRd(i) := false.B
    }
  }

  // Illegal operation
  io.decex.illOp := !Mux(dual, decoded.reduce(_&_), decoded(0))

  // Trigger exceptions
  val inDelaySlot = Reg(UInt(2.W))

  when(io.exc.exc ||
       (io.exc.intr && inDelaySlot === 0.U)) {
    io.decex.defaults()
    io.decex.pred(0) := 0.U
    io.decex.xcall := true.B
    io.decex.xsrc := io.exc.src
    io.decex.callAddr := io.exc.addr
    io.decex.immOp(0) := true.B
    io.decex.base := Mux(io.exc.exc, io.exc.excBase, decReg.base)
    io.decex.relPc := Mux(io.exc.exc, io.exc.excAddr, decReg.relPc)
  }

  // Update delay slot information
  when(io.ena) {
    val decDelaySlot = inDelaySlot - 1.U
    inDelaySlot := Mux(io.flush, 1.U,
                       Mux(io.decex.call || io.decex.ret || io.decex.brcf ||
                           io.decex.xcall || io.decex.xret, 3.U,
                           Mux(io.decex.jmpOp.branch, 2.U,
                               Mux(io.decex.aluOp(0).isMul,
                                   Mux(inDelaySlot > 1.U, decDelaySlot, 1.U),
                                   Mux(inDelaySlot =/= 0.U, decDelaySlot, 0.U)))))
  }

  // reset at end to override any computations
  when(reset.asBool) { decReg.flush() }
}

/*
   Copyright 2013 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Execution stage of Patmos.
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package patmos

import Chisel._
import Node._

import Constants._

class Execute() extends Module {
  val io = new ExecuteIO()

  val exReg = Reg(init = DecEx.resetVal)
  when(io.ena) {
    exReg := io.decex
    when(io.flush || io.brflush) {
      exReg.reset()
      exReg.relPc := io.decex.relPc
    }
  }

  def alu(func: Bits, op1: UInt, op2: UInt): Bits = {
    val result = UInt(width = DATA_WIDTH)
    val scaledOp1 = op1 << Mux(func === FUNC_SHADD2, UInt(2),
                               Mux(func === FUNC_SHADD, UInt(1),
                                   UInt(0)))
    val sum = scaledOp1 + op2
    result := sum // some default
    val shamt = op2(4, 0).toUInt
    // This kind of decoding of the ALU op in the EX stage is not efficient,
    // but we keep it for now to get something going soon.
    switch(func) {
      is(FUNC_ADD)    { result := sum }
      is(FUNC_SUB)    { result := op1 - op2 }
      is(FUNC_XOR)    { result := (op1 ^ op2).toUInt }
      is(FUNC_SL)     { result := (op1 << shamt).toUInt }
      is(FUNC_SR)     { result := (op1 >> shamt).toUInt }
      is(FUNC_SRA)    { result := (op1.toSInt >> shamt).toUInt }
      is(FUNC_OR)     { result := (op1 | op2).toUInt }
      is(FUNC_AND)    { result := (op1 & op2).toUInt }
      is(FUNC_NOR)    { result := (~(op1 | op2)).toUInt }
      is(FUNC_SHADD)  { result := sum }
      is(FUNC_SHADD2) { result := sum }
    }
    result
  }

  def comp(func: Bits, op1: UInt, op2: UInt): Bool = {
    val op1s = op1.toSInt
    val op2s = op2.toSInt
    val bitIdx = op2(4, 0).toUInt
    // Is this nicer than the switch?
    // Some of the comparison function (equ, subtract) could be shared
    val eq = op1 === op2
    val lt = op1s < op2s
    val ult = op1 < op2
    MuxLookup(func.toUInt, Bool(false), Array(
      (CFUNC_EQ,    eq),
      (CFUNC_NEQ,   !eq),
      (CFUNC_LT,    lt),
      (CFUNC_LE,    lt | eq),
      (CFUNC_ULT,   ult),
      (CFUNC_ULE,   ult | eq),
      (CFUNC_BTEST, op1(bitIdx))))
  }

  def pred(func: Bits, op1: Bool, op2: Bool): Bool = {
    MuxLookup(func.toUInt, Bool(false), Array(
      (PFUNC_OR, op1 | op2),
      (PFUNC_AND, op1 & op2),
      (PFUNC_XOR, op1 ^ op2),
      (PFUNC_NOR, ~(op1 | op2))))
  }

  // data forwarding
  val fwMemReg = Vec.fill(2*PIPE_COUNT) { Vec.fill(PIPE_COUNT) { Reg(init = Bool(false)) } }
  val fwExReg  = Vec.fill(2*PIPE_COUNT) { Vec.fill(PIPE_COUNT) { Reg(init = Bool(false)) } }
  val memResultDataReg = Vec.fill(PIPE_COUNT) { Reg(Bits(width = DATA_WIDTH)) }
  val exResultDataReg  = Vec.fill(PIPE_COUNT) { Reg(Bits(width = DATA_WIDTH)) }
  val op = Vec.fill(2*PIPE_COUNT) { Bits(width = 32) }

  // precompute forwarding
  when (io.ena) {
    for (i <- 0 until 2*PIPE_COUNT) {
      for (k <- 0 until PIPE_COUNT) {
        fwMemReg(i)(k) := Bool(false)
        when(io.decex.rsAddr(i) === io.memResult(k).addr && io.memResult(k).valid) {
          fwMemReg(i)(k) := Bool(true)
        }
        fwExReg(i)(k) := Bool(false)
        when(io.decex.rsAddr(i) === io.exResult(k).addr && io.exResult(k).valid) {
          fwExReg(i)(k) := Bool(true)
        }
      }
    }
    for (k <- 0 until PIPE_COUNT) {
      memResultDataReg(k) := io.memResult(k).data
      exResultDataReg(k) := io.exResult(k).data
    }
  }
  // forwarding multiplexers
  for (i <- 0 until 2*PIPE_COUNT) {
    op(i) := exReg.rsData(i)
    for (k <- 0 until PIPE_COUNT) {
      when(fwMemReg(i)(k)) {
        op(i) := memResultDataReg(k)
      }
    }
    for (k <- 0 until PIPE_COUNT) {
      when(fwExReg(i)(k)) {
        op(i) := exResultDataReg(k)
      }
    }
  }

  for (i <- 0 until PIPE_COUNT) {
    when(exReg.immOp(i)) {
      op(2*i+1) := exReg.immVal(i)
    }
  }

  // predicates
  val predReg = Vec.fill(PRED_COUNT) { Reg(init = Bool(false)) }

  val doExecute = Vec.fill(PIPE_COUNT) { Bool() }
  for (i <- 0 until PIPE_COUNT) {
    doExecute(i) := Mux(io.flush, Bool(false),
                        predReg(exReg.pred(i)(PRED_BITS-1, 0)) ^ exReg.pred(i)(PRED_BITS))
  }

  // stack registers
  val stackTopReg = Reg(init = UInt(0, DATA_WIDTH))
  val stackSpillReg = Reg(init = UInt(0, DATA_WIDTH))
  io.exdec.sp := stackTopReg

  // return information
  val retBaseReg = Reg(init = UInt(0, DATA_WIDTH))
  val retOffReg = Reg(init = UInt(0, DATA_WIDTH))
  val saveRetOff = Reg(init = Bool(false))
  val saveND = Reg(init = Bool(false))

  // exception return information
  val excBaseReg = Reg(init = UInt(0, DATA_WIDTH))
  val excOffReg = Reg(init = UInt(0, DATA_WIDTH))

  // MS: maybe the multiplication should be in a local component?

  // multiplication result registers
  val mulLoReg = Reg(init = UInt(0, DATA_WIDTH))
  val mulHiReg = Reg(init = UInt(0, DATA_WIDTH))

  // multiplication pipeline registers
  val mulLLReg    = Reg(init = UInt(0, DATA_WIDTH))
  val mulLHReg    = Reg(init = UInt(0, DATA_WIDTH))
  val mulHLReg    = Reg(init = UInt(0, DATA_WIDTH))
  val mulHHReg    = Reg(init = UInt(0, DATA_WIDTH))

  val signLHReg = Reg(init = Bits(0))
  val signHLReg = Reg(init = Bits(0))

  val mulPipeReg = Reg(init = Bool(false))

  // multiplication only in first pipeline
  when(io.ena) {
    mulPipeReg := exReg.aluOp(0).isMul && doExecute(0)

    val signed = exReg.aluOp(0).func === MFUNC_MUL

    val op1H = Cat(Mux(signed, Fill(DATA_WIDTH/2, op(0)(DATA_WIDTH-1)), Bits(0)),
                   op(0)(DATA_WIDTH-1, DATA_WIDTH/2))
    val op1L = Cat(Bits(0, width = DATA_WIDTH/2),
                   op(0)(DATA_WIDTH/2-1, 0))
    val op2H = Cat(Mux(signed, Fill(DATA_WIDTH/2, op(1)(DATA_WIDTH-1)), Bits(0)),
                   op(1)(DATA_WIDTH-1, DATA_WIDTH/2))
    val op2L = Cat(Bits(0, width = DATA_WIDTH/2),
                   op(1)(DATA_WIDTH/2-1, 0))

    mulLLReg := op1L * op2L
    mulLHReg := op1L * op2H
    mulHLReg := op1H * op2L
    mulHHReg := op1H * op2H

    signHLReg := op1H(DATA_WIDTH/2)
    signLHReg := op2H(DATA_WIDTH/2)

    val mulResult = (Cat(mulHHReg, mulLLReg)
                     + Cat(Fill(DATA_WIDTH/2, signHLReg),
                           mulHLReg, UInt(0, width = DATA_WIDTH/2))
                     + Cat(Fill(DATA_WIDTH/2, signLHReg),
                           mulLHReg, UInt(0, width = DATA_WIDTH/2)))

    when(mulPipeReg) {
      mulHiReg := mulResult(2*DATA_WIDTH-1, DATA_WIDTH)
      mulLoReg := mulResult(DATA_WIDTH-1, 0)
    }
  }

  // dual-issue operations
  for (i <- 0 until PIPE_COUNT) {

    val aluResult = alu(exReg.aluOp(i).func, op(2*i), op(2*i+1))
    val compResult = comp(exReg.aluOp(i).func, op(2*i), op(2*i+1))

    val bcpyPs = predReg(exReg.aluOp(i).func(PRED_BITS-1, 0)) ^ exReg.aluOp(i).func(PRED_BITS);
    val bcpyResult = (op(2*i) & ~(UInt(1) << op(2*i+1))) | (bcpyPs << op(2*i+1))

    // predicate operations
    val ps1 = predReg(exReg.predOp(i).s1Addr(PRED_BITS-1,0)) ^ exReg.predOp(i).s1Addr(PRED_BITS)
    val ps2 = predReg(exReg.predOp(i).s2Addr(PRED_BITS-1,0)) ^ exReg.predOp(i).s2Addr(PRED_BITS)
    val predResult = pred(exReg.predOp(i).func, ps1, ps2)

    when((exReg.aluOp(i).isCmp || exReg.aluOp(i).isPred) && doExecute(i) && io.ena) {
      predReg(exReg.predOp(i).dest) := Mux(exReg.aluOp(i).isCmp, compResult, predResult)
    }
    predReg(0) := Bool(true)

    // stack register handling
    when(exReg.aluOp(i).isSTC && doExecute(i)) {
      io.exdec.sp := op(2*i+1).toUInt()
      when (io.ena) {
        stackTopReg := op(2*i+1).toUInt()
      }
    }

    // special registers
    when(exReg.aluOp(i).isMTS && doExecute(i)) {
      switch(exReg.aluOp(i).func) {
        is(SPEC_ST) {
          io.exdec.sp := op(2*i).toUInt()
        }
      }
    }
    when(exReg.aluOp(i).isMTS && doExecute(i) && io.ena) {
      switch(exReg.aluOp(i).func) {
        is(SPEC_FL) {
          predReg := op(2*i)(PRED_COUNT-1, 0).toBits()
          predReg(0) := Bool(true)
        }
        is(SPEC_SL) {
          mulLoReg := op(2*i).toUInt()
        }
        is(SPEC_SH) {
          mulHiReg := op(2*i).toUInt()
        }
        is(SPEC_ST) {
          stackTopReg := op(2*i).toUInt()
        }
        is(SPEC_SS) {
          stackSpillReg := op(2*i).toUInt()
        }
        is(SPEC_SRB) {
          retBaseReg := op(2*i).toUInt()
        }
        is(SPEC_SRO) {
          retOffReg := op(2*i).toUInt()
        }
        is(SPEC_SXB) {
          excBaseReg := op(2*i).toUInt()
        }
        is(SPEC_SXO) {
          excOffReg := op(2*i).toUInt()
        }
      }
    }
    val mfsResult = UInt();
    mfsResult := UInt(0, DATA_WIDTH)
    switch(exReg.aluOp(i).func) {
      is(SPEC_FL) {
        mfsResult := Cat(Bits(0, DATA_WIDTH-PRED_COUNT), predReg.toBits()).toUInt()
      }
      is(SPEC_SL) {
        mfsResult := mulLoReg
      }
      is(SPEC_SH) {
        mfsResult := mulHiReg
      }
      is(SPEC_ST) {
        mfsResult := stackTopReg
      }
      is(SPEC_SS) {
        mfsResult := stackSpillReg
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

  val brcfOff = Mux(exReg.immOp(0), UInt(0), op(1).toUInt)
  val callRetAddr = Mux(exReg.call || exReg.xcall, UInt(0),
                        Mux(exReg.brcf, brcfOff,
                            Mux(exReg.xret, excOffReg, retOffReg)))

  val callBase = Mux(exReg.immOp(0), exReg.callAddr, op(0).toUInt)
  val callRetBase = Mux(exReg.call || exReg.xcall || exReg.brcf, callBase,
                        Mux(exReg.xret, excBaseReg, retBaseReg))

  io.exmem.mem.callRetBase := callRetBase
  io.exmem.mem.callRetAddr := callRetAddr

  // return information
  val baseReg = Reg(init = UInt(4, DATA_WIDTH))
  when(exReg.call && doExecute(0) && io.ena) {
    retBaseReg := baseReg
  }
  // the offset is saved when the call is already in the MEM statge
  saveRetOff := exReg.call && doExecute(0) && io.ena
  saveND := exReg.nonDelayed
  when(saveRetOff) {
    retOffReg := Cat(Mux(saveND, exReg.relPc, io.feex.pc), Bits("b00").toUInt)
  }
  // exception return information
  when(exReg.xcall && doExecute(0) && io.ena) {
    excBaseReg := baseReg
    excOffReg := Cat(exReg.relPc, Bits("b00").toUInt)
  }
  // remember base address
  when(doCallRet && io.ena) {
    baseReg := callRetBase
  }

  // branch
  io.exfe.doBranch := exReg.jmpOp.branch && doExecute(0)
  val target = Mux(exReg.immOp(0),
                   exReg.jmpOp.target,
                   op(0)(DATA_WIDTH-1, 2).toUInt - exReg.jmpOp.reloc)
  io.exfe.branchPc := target
  io.brflush := exReg.nonDelayed && exReg.jmpOp.branch && doExecute(0)

  // pass on PC
  io.exmem.pc := exReg.pc
  io.exmem.relPc := exReg.relPc

  //call/return for mcache
  io.exmcache.doCallRet := doCallRet
  io.exmcache.callRetBase := io.exmem.mem.callRetBase(31,2)
  io.exmcache.callRetAddr := io.exmem.mem.callRetAddr(31,2)

}

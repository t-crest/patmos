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
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 * Current Fmax on the DE2-70 is 84 MHz
 *   from forward address comparison to EXMEM register rd
 *   Removing the not so nice ALU instrcutions gives 96 MHz
 *   Drop just rotate: 90 MHz
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

class Execute() extends Component {
  val io = new ExecuteIO()

  val exReg = Reg(new DecEx())
  when(io.ena) {
    exReg := io.decex
  }
  // no access to io.decex after this point!!!

  def alu(func: Bits, op1: UFix, op2: UFix): Bits = {
    val result = UFix(width = DATA_WIDTH)
    val sum = op1 + op2
    result := sum // some default 0 default biggest, fastest. sum default slower smallest
    val shamt = op2(4, 0).toUFix
    // This kind of decoding of the ALU op in the EX stage is not efficient,
    // but we keep it for now to get something going soon.
    switch(func) {
      is(FUNC_ADD)    { result := sum }
      is(FUNC_SUB)    { result := op1 - op2 }
      is(FUNC_XOR)    { result := (op1 ^ op2).toUFix }
      is(FUNC_SL)     { result := (op1 << shamt).toUFix }
      is(FUNC_SR)     { result := (op1 >> shamt).toUFix }
      is(FUNC_SRA)    { result := (op1.toFix >> shamt).toUFix }
      is(FUNC_OR)     { result := (op1 | op2).toUFix }
      is(FUNC_AND)    { result := (op1 & op2).toUFix }
      is(FUNC_NOR)    { result := (~(op1 | op2)).toUFix }
      // TODO: shadd shift shall be in it's own operand MUX
      is(FUNC_SHADD)  { result := (op1 << UFix(1)) + op2 }
      is(FUNC_SHADD2) { result := (op1 << UFix(2)) + op2 }
    }
    result
  }

  def comp(func: Bits, op1: UFix, op2: UFix): Bool = {
    val op1s = op1.toFix
    val op2s = op2.toFix
    val shamt = op2(4, 0).toUFix
    // Is this nicer than the switch?
    // Some of the comparison function (equ, subtract) could be shared
    MuxLookup(func, Bool(false), Array(
      (CFUNC_EQ,    (op1 === op2)),
      (CFUNC_NEQ,   (op1 != op2)),
      (CFUNC_LT,    (op1s < op2s)),
      (CFUNC_LE,    (op1s <= op2s)),
      (CFUNC_ULT,   (op1 < op2)),
      (CFUNC_ULE,   (op1 <= op2)),
      (CFUNC_BTEST, ((op1 & (Bits(1) << shamt)) != UFix(0)))))
  }

  def pred(func: Bits, op1: Bool, op2: Bool): Bool = {
    MuxLookup(func, Bool(false), Array(
      (PFUNC_OR, op1 | op2),
      (PFUNC_AND, op1 & op2),
      (PFUNC_XOR, op1 ^ op2),
      (PFUNC_NOR, ~(op1 | op2))))
  }

  // data forwarding
  val fwEx0 = exReg.rsAddr(0) === io.exResult.addr && io.exResult.valid
  val fwMem0 = exReg.rsAddr(0) === io.memResult.addr && io.memResult.valid
  val ra = Mux(fwEx0, io.exResult.data, Mux(fwMem0, io.memResult.data, exReg.rsData(0)))
  val fwEx1 = exReg.rsAddr(1) === io.exResult.addr && io.exResult.valid
  val fwMem1 = exReg.rsAddr(1) === io.memResult.addr && io.memResult.valid
  val rb = Mux(fwEx1, io.exResult.data, Mux(fwMem1, io.memResult.data, exReg.rsData(1)))

  val op2 = Mux(exReg.immOp, exReg.immVal, rb)
  val op1 = ra
  val aluResult = alu(exReg.aluOp.func, op1, op2)
  val compResult = comp(exReg.aluOp.func, op1, op2)

  // predicates
  val predReg = Vec(PRED_COUNT) { Reg(resetVal = Bool(false)) }

  val ps1 = predReg(exReg.predOp.s1Addr(PRED_BITS-1,0)) ^ exReg.predOp.s1Addr(PRED_BITS)
  val ps2 = predReg(exReg.predOp.s2Addr(PRED_BITS-1,0)) ^ exReg.predOp.s2Addr(PRED_BITS)
  val predResult = pred(exReg.predOp.func, ps1, ps2)

  val doExecute = predReg(exReg.pred(PRED_BITS-1, 0)) ^ exReg.pred(PRED_BITS)

  when((exReg.aluOp.isCmp || exReg.aluOp.isPred) && doExecute && io.ena) {
    predReg(exReg.predOp.dest) := Mux(exReg.aluOp.isCmp, compResult, predResult)
  }
  predReg(0) := Bool(true)

  // multiplication
  val mulLoReg = Reg(resetVal = UFix(0, DATA_WIDTH))
  val mulHiReg = Reg(resetVal = UFix(0, DATA_WIDTH))

  val mulLL    = Reg(resetVal = UFix(0, DATA_WIDTH))
  val mulLH    = Reg(resetVal = UFix(0, DATA_WIDTH))
  val mulHL    = Reg(resetVal = UFix(0, DATA_WIDTH))
  val mulHH    = Reg(resetVal = UFix(0, DATA_WIDTH))

  val mulBuf = Reg(resetVal = UFix(0, 2*DATA_WIDTH))
  
  val mulPipe = Vec(3) { Reg(resetVal = Bool(false)) }

  when(io.ena) {
	mulPipe(0) := exReg.aluOp.isMul && doExecute
	mulPipe(1) := mulPipe(0)
	mulPipe(2) := mulPipe(1)

	val signed = exReg.aluOp.func === MFUNC_MUL

	val op1H = op1(DATA_WIDTH-1, DATA_WIDTH/2)
	val op1L = op1(DATA_WIDTH/2-1, 0)
	val op2H = op2(DATA_WIDTH-1, DATA_WIDTH/2)
	val op2L = op2(DATA_WIDTH/2-1, 0)

	mulLL := op1L.toUFix * op2L.toUFix
	mulLH := op1L.toUFix * op2H.toUFix
	mulHL := op1H.toUFix * op2L.toUFix
	mulHH := op1H.toUFix * op2H.toUFix

	when(signed) {
	  mulLL := (op1L.toUFix * op2L.toUFix).toUFix
	  mulLH := (op1L.toUFix * op2H.toFix).toUFix
	  mulHL := (op1H.toFix * op2L.toUFix).toUFix
	  mulHH := (op1H.toFix * op2H.toFix).toUFix
	}

	mulBuf := (Cat(mulHH, mulLL)
			   + Cat(mulHL, UFix(0, width = DATA_WIDTH/2))
			   + Cat(mulLH, UFix(0, width = DATA_WIDTH/2)))

	when(mulPipe(1)) {
	  mulHiReg := mulBuf(2*DATA_WIDTH-1, DATA_WIDTH)
	  mulLoReg := mulBuf(DATA_WIDTH-1, 0)
	}
  }

  // stack registers
  val stackTopReg = Reg(resetVal = UFix(0, DATA_WIDTH))
  val stackSpillReg = Reg(resetVal = UFix(0, DATA_WIDTH))
  io.exdec.sp := stackTopReg
  when(exReg.aluOp.isSTC && doExecute && io.ena) {
	io.exdec.sp := op2.toUFix()
	stackTopReg := op2.toUFix()
  }

  // special registers
  when(exReg.aluOp.isMTS && doExecute && io.ena) {
	switch(exReg.aluOp.func) {
	  is(SPEC_FL) {
		predReg := op1(PRED_COUNT-1, 0).toBits()
		predReg(0) := Bool(true)
	  }
	  is(SPEC_SL) {
		mulLoReg := op1.toUFix()
	  }
	  is(SPEC_SH) {
		mulHiReg := op1.toUFix()
	  }
	  is(SPEC_ST) {
		io.exdec.sp := op1.toUFix()
		stackTopReg := op1.toUFix()
	  }
	  is(SPEC_SS) {
		stackSpillReg := op1.toUFix()
	  }
	}
  }
  val mfsResult = UFix();
  mfsResult := UFix(0, DATA_WIDTH)
  switch(exReg.aluOp.func) {
	is(SPEC_FL) {
	  mfsResult := Cat(Bits(0, DATA_WIDTH-PRED_COUNT), predReg.toBits()).toUFix()
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
  }

  // result
  io.exmem.rd.addr := exReg.rdAddr(0)
  io.exmem.rd.data := Mux(exReg.aluOp.isMFS, mfsResult, aluResult)
  io.exmem.rd.valid := exReg.wrReg && doExecute
  // load/store
  io.exmem.mem.load := exReg.memOp.load && doExecute
  io.exmem.mem.store := exReg.memOp.store && doExecute
  io.exmem.mem.hword := exReg.memOp.hword
  io.exmem.mem.byte := exReg.memOp.byte
  io.exmem.mem.zext := exReg.memOp.zext
  io.exmem.mem.addr := op1 + exReg.immVal
  io.exmem.mem.data := op2
  io.exmem.mem.call := exReg.call && doExecute
  io.exmem.mem.ret  := exReg.ret && doExecute
  // call/return
  val callAddr = Mux(exReg.immOp, exReg.callAddr, op1.toUFix)
  io.exmem.mem.callRetAddr := Mux(exReg.call, callAddr, op1 + op2)
  io.exmem.mem.callRetBase := Mux(exReg.call, callAddr, op1.toUFix)
  // branch
  io.exfe.doBranch := exReg.jmpOp.branch && doExecute
  val target = Mux(exReg.immOp, exReg.jmpOp.target, op1(DATA_WIDTH-1, 2).toUFix)
  io.exfe.branchPc := target
  
  io.exmem.pc := exReg.pc
  io.exmem.predDebug := predReg
}

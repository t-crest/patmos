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
 * Connection definitions for the pipe stages.
 * 
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

class FeDec() extends Bundle() {
  val instr_a = Bits(width = INSTR_WIDTH)
  val instr_b = Bits(width = INSTR_WIDTH)
  val b_valid = Bool() // not yet used
  val pc = UFix(width = PC_SIZE)
}

class AluOp() extends Bundle() {
  val func = Bits(width = 4)
  val isMul = Bool()
  val isCmp = Bool()
  val isPred = Bool()
  val isMTS = Bool()
  val isMFS = Bool()
  val isSTC = Bool()
}

class PredOp() extends Bundle() {
  val func = Bits(width = 2) // as they have a strange encoding
  val dest = Bits(width = PRED_BITS)
  val s1Addr = Bits(width = PRED_BITS+1)
  val s2Addr = Bits(width = PRED_BITS+1)
}

class JmpOp() extends Bundle() {
  val branch = Bool()
  val target = UFix(width = PC_SIZE)
}

class MemOp() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ  = Bits(width = 2)
}

class DecEx() extends Bundle() {
  val pc = UFix(width = PC_SIZE)
  val pred =  Vec(PIPE_COUNT) { Bits(width = PRED_BITS+1) }
  val aluOp = Vec(PIPE_COUNT) { new AluOp() }
  val predOp = Vec(PIPE_COUNT) { new PredOp() }
  val jmpOp = new JmpOp()
  val memOp = new MemOp()

  // the register fields are very similar to RegFileRead
  // maybe join the structures
  val rsAddr = Vec(2*PIPE_COUNT) { Bits(width = REG_BITS) }
  val rsData = Vec(2*PIPE_COUNT) { Bits(width = DATA_WIDTH) }
  val rdAddr = Vec(PIPE_COUNT) { Bits(width = REG_BITS) }
  val immVal = Vec(PIPE_COUNT) { Bits(width = DATA_WIDTH) }
  val immOp  = Vec(PIPE_COUNT) { Bool() }
  val wrReg  = Vec(PIPE_COUNT) { Bool() }

  val callAddr = UFix(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
}

class Result() extends Bundle() {
  val addr = Bits(INPUT, REG_BITS)
  val data = Bits(INPUT, DATA_WIDTH)
  val valid = Bool(INPUT)
}

class MemIn() extends Bundle() {
  val load = Bool()
  val store = Bool()
  val hword = Bool()
  val byte = Bool()
  val zext = Bool()
  val typ = Bits(width = 2)
  val addr = Bits(width = DATA_WIDTH)
  val data = Bits(width = DATA_WIDTH)
  val call = Bool()
  val ret = Bool()
  val callRetAddr = UFix(width = DATA_WIDTH)
  val callRetBase = UFix(width = DATA_WIDTH)
}

class ExDec() extends Bundle() {
  val sp = UFix(width = DATA_WIDTH)
}

class ExMem() extends Bundle() {
  val rd = Vec(PIPE_COUNT) { new Result() }
  val mem = new MemIn()
  val pc = UFix(width = PC_SIZE)
  // just for debugging
  val predDebug = Vec(8) { Bool() }
}

class ExFe() extends Bundle() {
  val doBranch = Bool()
  val branchPc = UFix(width = PC_SIZE)
}

class MemFe() extends Bundle() {
  val doCallRet = Bool()
  val callRetPc = UFix(width = PC_SIZE)
  val callRetBase = UFix(width = PC_SIZE)
  // for ISPM write
  val store = Bool()
  val addr = Bits(width = DATA_WIDTH)
  val data = Bits(width = DATA_WIDTH)
}

class FeMem() extends Bundle() {
  val pc = UFix(width = PC_SIZE)
}

class MemWb() extends Bundle() {
  val rd = Vec(PIPE_COUNT) { new Result() }
  // do we need this? probably not.
  // maybe drop unused pc fields
  // maybe nice for debugging?
  val pc = UFix(width = PC_SIZE)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B
  val rsAddr = Vec(2*PIPE_COUNT) { Bits(INPUT, REG_BITS) }
  val rsData = Vec(2*PIPE_COUNT) { Bits(OUTPUT, DATA_WIDTH) }
}

class RegFileIO() extends Bundle() {
  val ena = Bool(INPUT)
  val rfRead = new RegFileRead()
  val rfWrite = Vec(PIPE_COUNT) { new Result() }
  val rfDebug = Vec(REG_COUNT) { Bits(OUTPUT, DATA_WIDTH) }
}

class FetchIO extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asOutput
  // PC for returns
  val femem = new FeMem().asOutput
  // branch from EX
  val exfe = new ExFe().asInput
  // call from MEM
  val memfe = new MemFe().asInput
}

class DecodeIO() extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asInput
  val decex = new DecEx().asOutput
  val exdec = new ExDec().asInput
  val rfWrite =  Vec(PIPE_COUNT) { new Result() }
}

class ExecuteIO() extends Bundle() {
  val ena = Bool(INPUT)
  val decex = new DecEx().asInput
  val exdec = new ExDec().asOutput
  val exmem = new ExMem().asOutput
  // forwarding inputs
  val exResult = Vec(PIPE_COUNT) { new Result() }
  val memResult = Vec(PIPE_COUNT) { new Result() }
  // branch for FE
  val exfe = new ExFe().asOutput
}

class SimpCon() extends Bundle() {
  val rd = Bool(OUTPUT)
  val wr = Bool(OUTPUT)
  val address = Bits(OUTPUT, ADDR_WIDTH)
  val wrData = Vec(BYTES_PER_WORD) { Bits(OUTPUT, BYTE_WIDTH) }
  val byteEna = Bits(OUTPUT, BYTES_PER_WORD)
  val rdData = Vec(BYTES_PER_WORD) { Bits(INPUT, BYTE_WIDTH) }
  val rdyCnt = Bits(INPUT, 2)
}

class UartPinIO() extends Bundle() {
  val tx = UFix(OUTPUT, 1)
  val rx = UFix(INPUT, 1)  
}

class UartIO() extends Bundle() {
  val rd = UFix(INPUT, 1)
  val wr = UFix(INPUT, 1) 
  val address = UFix(INPUT, 1)
  val data_in = UFix(INPUT, 8)
  val rd_data = UFix(OUTPUT, 8)
  val pins = new UartPinIO()
}

class InOutIO() extends Bundle() {
  val memInOut = new SimpCon().flip
  val uartPins = new UartPinIO()
  val led = Bits(OUTPUT, 8)
}

class MemoryIO() extends Bundle() {
  val ena = Bool(INPUT)
  val exmem = new ExMem().asInput
  val memwb = new MemWb().asOutput
  val memfe = new MemFe().asOutput
  val femem = new FeMem().asInput
  // for result forwarding
  val exResult = Vec(PIPE_COUNT) { new Result().flip }
  // local and global accesses
  val localInOut = new SimpCon()
  val globalInOut = new SimpCon()
}

class WriteBackIO() extends Bundle() {
  val ena = Bool(INPUT)
  val memwb = new MemWb().asInput
  // wb result (unregistered)
  val rfWrite = Vec(PIPE_COUNT) { new Result().flip }
  // for result forwarding (register)
  val memResult =  Vec(PIPE_COUNT) { new Result().flip }
}

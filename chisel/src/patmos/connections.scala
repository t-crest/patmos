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
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._


class FeDec() extends Bundle() {
  val instr_a = Bits(width = 32)
  //  val instr_b = Bits(width=32)
  //  val b_valid = Bool()
  val pc = UFix(width = Constants.PC_SIZE)
}

class DecEx() extends Bundle() {
  val pc = UFix(width = Constants.PC_SIZE)
  val func = Bits(width = 4)
  // the register fields are very similar to RegFileRead
  // maybe join the structures
  val rsAddr = Vec(2) { Bits(width=5) }
  val rsData = Vec(2) { Bits(width=32) }
  val rdAddr = Vec(1) { Bits(width=5) }
  val immVal = Bits(width=32)
  val immOp = Bool()
  // wrReg? or wrEn? or valid? We use now all three at different places ;-)
  val wrReg = Bool()
}

class Result() extends Bundle() {
  val addr = Bits(INPUT, 5)
  val data = Bits(INPUT, 32)
  val valid = Bool(INPUT)
}

class ExMem() extends Bundle() {
  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class MemWb() extends Bundle() {
  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class WbFinal() extends Bundle() {
  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B (not yet done)
  val rsAddr = Vec(2) { Bits(INPUT, 5) }
  val rsData = Vec(2) { Bits(OUTPUT, 32) }
}

class RegFileIO() extends Bundle() {
  val ena = Bool(INPUT)
  val rfRead = new RegFileRead()
  val rfWrite = new Result()
}

class FetchIO extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asOutput
}

class DecodeIO() extends Bundle() {
  val ena = Bool(INPUT)
  val fedec = new FeDec().asInput
  val decex = new DecEx().asOutput
  val rfWrite = new Result()
}

class ExecuteIO() extends Bundle() {
  val ena = Bool(INPUT)
  val decex = new DecEx().asInput
  val exmem = new ExMem().asOutput
  // forwarding inputs
  val exResult = new Result()
  val memResult = new Result()
}

class MemoryIO() extends Bundle() {
  val ena = Bool(INPUT)
  val exmem = new ExMem().asInput
  val memwb = new MemWb().asOutput
  // for result forwarding
  val exResult = new Result().flip
}

class WriteBackIO() extends Bundle() {
  val ena = Bool(INPUT)
  val memwb = new MemWb().asInput
  // do we have any useful out from WB?
  val out = new WbFinal().asOutput
  // wb result (unregistered)
  val rfWrite = new Result().flip
  // for result forwarding (register)
  val memResult = new Result().flip
}

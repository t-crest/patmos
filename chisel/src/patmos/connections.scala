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
 * Port definitions for the pipe stages.
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
  val rsAddr = Vec(2) { Bits(width=5) }
  val rsData = Vec(2) { Bits(width=32) }
}

class Result() extends Bundle() {
  val addr = Bits(width=5)
  val data = Bits(width=32)
  val valid = Bool()
}

class ExMem() extends Bundle() {
//  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class MemWb() extends Bundle() {
//  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class WbFinal() extends Bundle() {
//  val rd = new Result()
  val pc = UFix(width = Constants.PC_SIZE)
}

class RegFileRead() extends Bundle() {
  // first two are for pipeline A, second two for pipeline B (not yet done)
  val rsAddr = Vec(2) { Bits(INPUT, 5) }
  val rsData = Vec(2) { Bits(OUTPUT, 32) }
}

class RegFileWrite() extends Bundle() {
  val wrAddr = Bits(INPUT, 5)
  val wrData = Bits(INPUT, 32)
  val wrEn = Bool(INPUT)
}

class RegFileIO() extends Bundle() {
  val rfRead = new RegFileRead()
  val rfWrite = new RegFileWrite()
}

class FetchIO extends Bundle() {
  val out = new FeDec().asOutput
}

class DecodeIO() extends Bundle() {
  val in = new FeDec().asInput
  val out = new DecEx().asOutput
  val rfRead = new RegFileRead().flip
}

class ExecuteIO() extends Bundle() {
  val in = new DecEx().asInput
  val out = new ExMem().asOutput
}

class MemoryIO() extends Bundle() {
  val in = new ExMem().asInput
  val out = new MemWb().asOutput
}

class WriteBackIO() extends Bundle() {
  val in = new MemWb().asInput
  val out = new WbFinal().asOutput
  val rfWrite = new RegFileWrite().flip
}

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
 * Decode stage of Patmos.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package patmos

import Chisel._
import Node._

class Decode() extends Component {
  val io = new DecodeIO()

  val rf = new RegisterFile()

  // register file is connected with unregistered instruction word
  rf.io.rfRead.rsAddr(0) := io.fedec.instr_a(16, 12)
  rf.io.rfRead.rsAddr(1) := io.fedec.instr_a(11, 7)
  rf.io.ena := io.ena
  // RF write from write back stage
  rf.io.rfWrite <> io.rfWrite

  val decReg = Reg(new FeDec())
  when(io.ena) {
    decReg := io.fedec
  }

  val instr = decReg.instr_a
  // keep it in a way that is easy to refactor into a function for
  // dual issue decode

  val func = Bits(width = 4)
  // Start with some useful defaults
  io.decex.immOp := Bool(false)
  io.decex.aluOp := Bool(false)
  io.decex.cmpOp := Bool(false)
  io.decex.unaryOp := Bool(false)
  io.decex.branch := Bool(false)
  // Is this the best default value?
  // TODO: maybe turn it around - default not doing anything
  io.decex.wrReg := Bool(true)

  // ALU register and long immediate
  func := instr(3, 0)
  // ALU immediate
  when(instr(26, 25) === Bits("b00")) {
    func := Cat(Bits(0), instr(24, 22))
    io.decex.immOp := Bool(true)
    io.decex.aluOp := Bool(true)
  }
  // Other ALU
  when(instr(26, 22) === Bits("b01000")) {
    switch(instr(6, 4)) {
      is(Bits("b000")) { io.decex.aluOp := Bool(true) }
      is(Bits("b001")) { io.decex.unaryOp := Bool(true) }
      is(Bits("b010")) {} // multiply
      is(Bits("b011")) {
        io.decex.cmpOp := Bool(true)
        io.decex.wrReg := Bool(false)
      }
      is(Bits("b100")) {} // predicate
    }
  }
  // We do not need such a long immediate for branches.
  // So this is waste of opcode space
  when(instr(26, 24) === Bits("b110")) {
    io.decex.wrReg := Bool(false)
    // But on a call we will save in register 31 => need to change the target register
    switch(instr(23, 22)) {
      is(Bits("b00")) { /* io.decex.call := Bool(true) */ }
      is(Bits("b01")) { io.decex.branch := Bool(true) }
      is(Bits("b10")) { /* io.decex.brcf := Bool(true) */ }
      // where is register indirect?
    }
  }

  // Immediate is not sign extended...
  io.decex.immVal := Cat(Bits(0), instr(11, 0))
  // we could mux the imm / register here as well

  // Immediate for branch is sign extended, not extended for call
  // We can do the address calculation already here
  io.decex.branchPc := decReg.pc + Cat(Fill(Constants.PC_SIZE-22, instr(21)), instr(21, 0))
  // On a call just take the value

  // Disable register write on register 0
  when(decReg.instr_a(21, 17) === Bits("b00000")) {
    io.decex.wrReg := Bool(false)
  }

  io.decex.pc := decReg.pc
  io.decex.func := func
  io.decex.pd := decReg.instr_a(19, 17)
  io.decex.pred := decReg.instr_a(30, 27)
  // forward RF addresses and data
  io.decex.rsAddr(0) := decReg.instr_a(16, 12)
  io.decex.rsAddr(1) := decReg.instr_a(11, 7)
  io.decex.rsData(0) := rf.io.rfRead.rsData(0)
  io.decex.rsData(1) := rf.io.rfRead.rsData(1)

  io.decex.rdAddr(0) := decReg.instr_a(21, 17)
}
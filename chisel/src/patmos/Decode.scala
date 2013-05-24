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

import Constants._

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

  val opcode  = instr(26, 22)
  val opc     = instr(6, 4)
  val ldsize  = instr(11, 9)
  val ldtype  = instr(8, 7)
  val stsize  = instr(21, 19)
  val sttype  = instr(18, 17)

  val func    = Bits(width = 4)
  val dest    = Bits(width = REG_BITS)
  val longImm = Bool()

  // Start with some useful defaults
  io.decex.immOp := Bool(false)
  io.decex.aluOp.isCmp := Bool(false)
  io.decex.aluOp.isPred := Bool(false)
  io.decex.jmpOp.branch := Bool(false)
  io.decex.call := Bool(false)
  io.decex.ret := Bool(false)
  io.decex.memOp.load := Bool(false)
  io.decex.memOp.store := Bool(false)
  io.decex.memOp.hword := Bool(false)
  io.decex.memOp.byte := Bool(false)
  io.decex.memOp.zext := Bool(false)
  io.decex.wrReg := Bool(false)

  // Long immediates set this
  longImm := Bool(false)

  // Everything except calls uses the default
  dest := decReg.instr_a(21, 17)

  // ALU register and long immediate
  func := instr(3, 0)

  // ALU immediate
  when(opcode(4, 3) === OPCODE_ALUI) {
    func := Cat(Bits(0), instr(24, 22))
    io.decex.immOp := Bool(true)
    io.decex.wrReg := Bool(true)
  }
  // Other ALU
  when(opcode === OPCODE_ALU) {
    switch(opc) {
      is(OPC_ALUR) { io.decex.wrReg := Bool(true) }
      is(OPC_ALUU) { io.decex.wrReg := Bool(true) }
      is(OPC_ALUM) {} // multiply
      is(OPC_ALUC) { io.decex.aluOp.isCmp := Bool(true) }
      is(OPC_ALUP) { io.decex.aluOp.isPred := Bool(true) }
    }
  }
  // ALU long immediate (Bit 31 is set as well)
  when(opcode === OPCODE_ALUL) {
    io.decex.immOp := Bool(true)
    longImm := Bool(true)
    io.decex.wrReg := Bool(true)
  }
  // Control-flow operations
  when(opcode === OPCODE_CFL_CALL) {
    io.decex.immOp := Bool(true)
    io.decex.call := Bool(true)
    io.decex.wrReg := Bool(true)
	dest := Bits("b11111")
  }
  when(opcode === OPCODE_CFL_BR) {
    io.decex.immOp := Bool(true)
	io.decex.jmpOp.branch := Bool(true)
  }
  when(opcode === OPCODE_CFL_BRCF) {
    io.decex.immOp := Bool(true)
    io.decex.call := Bool(true)
  }
  when(opcode === OPCODE_CFL_CFLI) {
	switch(func) {
	  is(JFUNC_CALL) {
		io.decex.call := Bool(true)
		io.decex.wrReg := Bool(true)
		dest := Bits("b11111")
	  }
	  is(JFUNC_BR) {
		io.decex.jmpOp.branch := Bool(true)
	  }
	  is(JFUNC_BRCF) {
		io.decex.call := Bool(true)
	  }
	}
  }
  when(opcode === OPCODE_CFL_RET) {
    io.decex.ret := Bool(true)
  }

  val isMem = Bool()
  val shamt = UFix()
  shamt := UFix(0)
  isMem := Bool(false)
  // load
  when(opcode === OPCODE_LDT) {
    isMem := Bool(true)
    io.decex.memOp.load := Bool(true)
    io.decex.wrReg := Bool(true)
    switch(ldsize) {
      is(MSIZE_W) {
        shamt := UFix(2)
      }
      is(MSIZE_H) {
        shamt := UFix(1)
        io.decex.memOp.hword := Bool(true)
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := Bool(true)
      }
      is(MSIZE_HU) {
        shamt := UFix(1)
        io.decex.memOp.hword := Bool(true)
        io.decex.memOp.zext := Bool(true)
      }
      is(MSIZE_BU) {
        io.decex.memOp.byte := Bool(true)
        io.decex.memOp.zext := Bool(true)
      }
      // ignore split load for now
    }
  }
  // store
  when(opcode === OPCODE_STT) {
    isMem := Bool(true)
    io.decex.memOp.store := Bool(true)
    switch(stsize) {
      is(MSIZE_W) {
        shamt := UFix(2)
      }
      is(MSIZE_H) {
        shamt := UFix(1)
        io.decex.memOp.hword := Bool(true)
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := Bool(true)
      }
    }
  }
  // we could merge the shamt of load and store when not looking into split load

  val addrImm = Bits()
  addrImm := Cat(Bits(0), instr(6, 0))
  switch(shamt) {
    is(UFix(1)) { addrImm := Cat(Bits(0), instr(6, 0), Bits(0, width = 1)) }
    is(UFix(2)) { addrImm := Cat(Bits(0), instr(6, 0), Bits(0, width = 2)) }
  }

  // Immediate is not sign extended
  // Maybe later split immediate for ALU and address calculation
  io.decex.immVal := Mux(isMem, addrImm,
						 Mux(longImm, decReg.instr_b,
							 Cat(Bits(0), instr(11, 0))))
  // we could mux the imm / register here as well
  
  // Immediate for absolute calls
  io.decex.callAddr := Cat(Bits(0), instr(21, 0), Bits("b00")).toUFix()

  // Immediate for branch is sign extended, not extended for call
  // We can do the address calculation already here
  io.decex.jmpOp.target := decReg.pc + Cat(Fill(PC_SIZE - 22, instr(21)), instr(21, 0))
  // On a call just take the value

  io.decex.pc := decReg.pc
  io.decex.aluOp.func := func

  io.decex.predOp.func := Cat(decReg.instr_a(3), decReg.instr_a(0))
  io.decex.predOp.s1Addr := decReg.instr_a(15, 12)
  io.decex.predOp.s2Addr := decReg.instr_a(10, 7)
  io.decex.predOp.dest := decReg.instr_a(19, 17)

  io.decex.pred := decReg.instr_a(30, 27)
  // forward RF addresses and data
  io.decex.rsAddr(0) := decReg.instr_a(16, 12)
  io.decex.rsAddr(1) := decReg.instr_a(11, 7)
  io.decex.rsData(0) := rf.io.rfRead.rsData(0)
  io.decex.rsData(1) := rf.io.rfRead.rsData(1)

  io.decex.rdAddr(0) := dest

  // Disable register write on register 0
  when(io.decex.rdAddr(0) === Bits("b00000")) {
    io.decex.wrReg := Bool(false)
  }
}

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
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package patmos

import Chisel._
import Node._

import Constants._

class Decode() extends Module {
  val io = new DecodeIO()

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
  val decReg = Reg(init = FeDec.resetVal)
  when(io.ena) {
    decReg := io.fedec
    when(io.flush) {
      decReg.reset()
      decReg.relPc := io.fedec.relPc
    }
  }

  // default values
  io.decex.reset()

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

  val decoded = Bool()
  decoded := Bool(false)

  // Decoding of dual-issue operations
  val dual = decReg.instr_a(INSTR_WIDTH-1) && decReg.instr_a(26, 22) != OPCODE_ALUL;
  for (i <- 0 until PIPE_COUNT) {
	val instr   = if (i == 0) { decReg.instr_a } else { decReg.instr_b }
	val opcode  = instr(26, 22)
	val opc     = instr(6, 4)
	val isValid = if (i == 0) { Bool(true) } else { dual }

	val immVal = Bits()
	// Default value for immediates
	immVal := Cat(Bits(0), instr(11, 0))

	// ALU register
	io.decex.aluOp(i).func := instr(3, 0)

	// ALU immediate
	when(opcode(4, 3) === OPCODE_ALUI) {
      io.decex.aluOp(i).func := Cat(Bits(0), instr(24, 22))
      io.decex.immOp(i) := isValid
      io.decex.wrRd(i) := isValid
      decoded := Bool(true)
	}
	// Other ALU
	when(opcode === OPCODE_ALU) {
      switch(opc) {
		is(OPC_ALUR) {
		  io.decex.wrRd(i) := isValid
		  decoded := Bool(true)
		}
		is(OPC_ALUU) {
		  io.decex.wrRd(i) := isValid 
		  decoded := Bool(true)
		}
		is(OPC_ALUM) {
		  io.decex.aluOp(i).isMul := isValid
		  decoded := Bool(true)
		}
		is(OPC_ALUC) {
		  io.decex.aluOp(i).isCmp := isValid
		  decoded := Bool(true)
		}
		is(OPC_ALUCI) {
		  io.decex.aluOp(i).isCmp := isValid
		  io.decex.immOp(i) := isValid
		  immVal := Cat(Bits(0), instr(11, 7))
		  decoded := Bool(true)
		}
		is(OPC_ALUP) {
		  io.decex.aluOp(i).isPred := isValid
		  decoded := Bool(true)
		}
      }
	}
	// Special registers
	when(opcode === OPCODE_SPC) {
	  switch(opc) {
		is(OPC_MTS) {
		  io.decex.aluOp(i).isMTS := isValid
		  decoded := Bool(true)
		}
		is(OPC_MFS) {
		  io.decex.aluOp(i).isMFS := isValid
		  io.decex.wrRd(i) := isValid
		  decoded := Bool(true)
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
  val instr   = decReg.instr_a
  val opcode  = instr(26, 22)
  val func    = instr(3, 0)

  val ldsize  = instr(11, 9)
  val ldtype  = instr(8, 7)
  val stsize  = instr(21, 19)
  val sttype  = instr(18, 17)
  val stcfun  = instr(21, 18)

  val dest    = Bits(width = REG_BITS)
  val longImm = Bool()

  val isMem   = Bool()
  val isStack = Bool()

  val isSTC   = Bool()
  val stcVal  = Bits(width = DATA_WIDTH)
  val stcImm  = Cat(Bits(0), instr(17, 0), Bits("b00")).toUInt()

  // Long immediates set this
  longImm := Bool(false)

  // Load/stores and stack control operations set this
  isMem := Bool(false)
  isStack := Bool(false)
  isSTC := Bool(false)
  stcVal := io.exdec.sp

  // Everything except calls uses the default
  dest := instr(21, 17)

  // ALU long immediate (Bit 31 is set as well)
  when(opcode === OPCODE_ALUL) {
	io.decex.aluOp(0).func := func
    io.decex.immOp(0) := Bool(true)
    longImm := Bool(true)
    io.decex.wrRd(0) := Bool(true)
    decoded := Bool(true)
  }
  // Stack control
  when(opcode === OPCODE_STC) {
	switch(stcfun) {
	  is(STC_SRES) {
		io.decex.aluOp(0).isSTC := Bool(true)
		isSTC := Bool(true)
		io.decex.immOp(0) := Bool(true)
		stcVal := io.exdec.sp - stcImm
		decoded := Bool(true)
	  }
	  is(STC_SFREE) {
		io.decex.aluOp(0).isSTC := Bool(true)
		isSTC := Bool(true)
		io.decex.immOp(0) := Bool(true)
		stcVal := io.exdec.sp + stcImm
		decoded := Bool(true)
	  }
	  is(STC_SENS) {
		// TODO: ignored for now
		decoded := Bool(true)
	  }
	  is(STC_SENSR) {
		// TODO: ignored for now
		decoded := Bool(true)
	  }
	  is(STC_SSPILL) {
		// TODO: ignored for now
		decoded := Bool(true)
	  }
	  is(STC_SSPILLR) {
		// TODO: ignored for now
		decoded := Bool(true)
	  }
	}
  }
  // Control-flow operations
  when(opcode === OPCODE_CFL_TRAP) {
    io.decex.trap := Bool(true)
    io.decex.xsrc := instr(EXC_SRC_BITS-1, 0)
    decoded := Bool(true)
  }
  when(opcode === OPCODE_CFL_CALL || opcode === OPCODE_CFL_CALLND) {
    io.decex.immOp(0) := Bool(true)
    io.decex.call := Bool(true)
    io.decex.nonDelayed := opcode === OPCODE_CFL_CALLND
	decoded := Bool(true)
  }
  when(opcode === OPCODE_CFL_BR || opcode === OPCODE_CFL_BRND) {
    io.decex.immOp(0) := Bool(true)
	io.decex.jmpOp.branch := Bool(true)
    io.decex.nonDelayed := opcode === OPCODE_CFL_BRND
	decoded := Bool(true)
  }
  when(opcode === OPCODE_CFL_BRCF || opcode === OPCODE_CFL_BRCFND) {
    io.decex.immOp(0) := Bool(true)
    io.decex.brcf := Bool(true)
    io.decex.nonDelayed := opcode === OPCODE_CFL_BRCFND
	decoded := Bool(true)
  }
  when(opcode === OPCODE_CFL_CFLR || opcode === OPCODE_CFL_CFLRND) {
	switch(func) {
	  is(JFUNC_RET) {
		io.decex.ret := Bool(true)
		decoded := Bool(true)
	  }
	  is(JFUNC_XRET) {
		io.decex.xret := Bool(true)
		decoded := Bool(true)
	  }
	  is(JFUNC_CALL) {
		io.decex.call := Bool(true)
		decoded := Bool(true)
	  }
	  is(JFUNC_BR) {
		io.decex.jmpOp.branch := Bool(true)
		decoded := Bool(true)
	  }
	  is(JFUNC_BRCF) {
		io.decex.brcf := Bool(true)
		decoded := Bool(true)
	  }
	}
	io.decex.nonDelayed := opcode === OPCODE_CFL_CFLRND
  }

  val shamt = UInt()
  shamt := UInt(0)
  // load
  when(opcode === OPCODE_LDT) {
    isMem := Bool(true)
    io.decex.memOp.load := Bool(true)
    io.decex.wrRd(0) := Bool(true)
    switch(ldsize) {
      is(MSIZE_W) {
        shamt := UInt(2)
      }
      is(MSIZE_H) {
        shamt := UInt(1)
        io.decex.memOp.hword := Bool(true)
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := Bool(true)
      }
      is(MSIZE_HU) {
        shamt := UInt(1)
        io.decex.memOp.hword := Bool(true)
        io.decex.memOp.zext := Bool(true)
      }
      is(MSIZE_BU) {
        io.decex.memOp.byte := Bool(true)
        io.decex.memOp.zext := Bool(true)
      }
      // ignore split load for now
    }
	io.decex.memOp.typ := ldtype;
	when(ldtype === MTYPE_S) {
	  isStack := Bool(true)
	}
	decoded := Bool(true)
  }
  // store
  when(opcode === OPCODE_STT) {
    isMem := Bool(true)
    io.decex.memOp.store := Bool(true)
    switch(stsize) {
      is(MSIZE_W) {
        shamt := UInt(2)
      }
      is(MSIZE_H) {
        shamt := UInt(1)
        io.decex.memOp.hword := Bool(true)
      }
      is(MSIZE_B) {
        io.decex.memOp.byte := Bool(true)
      }
    }
	io.decex.memOp.typ := sttype;
	when(sttype === MTYPE_S) {
	  isStack := Bool(true)
	}
	decoded := Bool(true)
  }

  // Offset for loads/stores
  val addrImm = Bits()
  addrImm := Cat(Bits(0), instr(6, 0))
  switch(shamt) {
    is(UInt(1)) { addrImm := Cat(Bits(0), instr(6, 0), Bits(0, width = 1)) }
    is(UInt(2)) { addrImm := Cat(Bits(0), instr(6, 0), Bits(0, width = 2)) }
  }

  // Non-default immediate value
  when (isSTC || isStack || isMem || longImm) {
    io.decex.immVal(0) := Mux(isSTC, stcVal,
							  Mux(isStack, addrImm + io.exdec.sp,
								  Mux(isMem, addrImm,
									  decReg.instr_b)))
  }
  // we could mux the imm / register here as well
  
  // Immediate for absolute calls
  io.decex.callAddr := Cat(Bits(0), instr(21, 0), Bits("b00")).toUInt()

  // Immediate for branch is sign extended, not extended for call
  // PC-relative value is precomputed here
  io.decex.jmpOp.target := decReg.pc + Cat(Fill(PC_SIZE - 22, instr(21)), instr(21, 0))
  io.decex.jmpOp.reloc := decReg.reloc

  // Pass on PC
  io.decex.pc := decReg.pc
  io.decex.relPc := decReg.relPc

  // Set destination address
  io.decex.rdAddr(0) := dest

  // Disable register write on register 0
  for (i <- 0 until PIPE_COUNT) {
	when(io.decex.rdAddr(i) === Bits("b00000")) {
      io.decex.wrRd(i) := Bool(false)
	}
  }
  
  // Illegal operation
  io.decex.illOp := !decoded

  // Trigger exceptions
  val inDelaySlot = Reg(UInt(width = 2))

  when(io.exc.exc ||
       (io.exc.intr && inDelaySlot === UInt(0))) {
    io.decex.reset()
    io.decex.pred(0) := Bits(0)
    io.decex.xcall := Bool(true)
    io.decex.xsrc := io.exc.src
    io.decex.callAddr := io.exc.addr
    io.decex.immOp(0) := Bool(true)
    io.decex.relPc := Mux(io.exc.exc, io.exc.excAddr, decReg.relPc)
  }

  // Update delay slot information
  when(io.ena && !io.flush) {
    val decDelaySlot = inDelaySlot - UInt(1)
    inDelaySlot := Mux(io.decex.call || io.decex.ret || io.decex.brcf ||
                       io.decex.xcall || io.decex.xret, UInt(3),
                       Mux(io.decex.jmpOp.branch, UInt(2),
                           Mux(io.decex.aluOp(0).isMul,
                               Mux(inDelaySlot > UInt(1), decDelaySlot, UInt(1)),
                               Mux(inDelaySlot != UInt(0), decDelaySlot, UInt(0)))))
  }
}

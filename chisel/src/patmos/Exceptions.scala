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
 * Exceptions for Patmos.
 * 
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package patmos

import Chisel._
import Node._

import Constants._

import ocp._

class Exceptions extends Component {
  val io = new ExcIO()

  val EXC_ADDR_WIDTH = 8

  val masterReg = Reg(io.ocp.M)

  val status = Reg(resetVal = Bits(0, width = DATA_WIDTH))
  val mask   = Reg(resetVal = Bits(0, width = DATA_WIDTH))
  val source = Reg(resetVal = Bits(0, width = DATA_WIDTH))

  val vec    = Mem(EXC_COUNT) { UFix(width = DATA_WIDTH) }
  val vecDup = Mem(EXC_COUNT) { UFix(width = DATA_WIDTH) }

  // Latches for incoming exceptions and interrupts
  val excPend     = Vec(EXC_COUNT) { Bool() }
  val excPendReg  = Vec(EXC_COUNT) { Reg(resetVal = Bool(false)) }
  val intrPend    = Vec(EXC_COUNT) { Bool() }
  val intrPendReg = Vec(EXC_COUNT) { Reg(resetVal = Bool(false)) }  
  excPend := excPendReg
  intrPend := intrPendReg

  // Default OCP response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := Bits(0, width = DATA_WIDTH)

  // Handle OCP reads and writes
  when(masterReg.Cmd === OcpCmd.RD) {
	io.ocp.S.Resp := OcpResp.DVA
	
	switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
	  is(Bits("b000000")) { io.ocp.S.Data := status }
	  is(Bits("b000001")) { io.ocp.S.Data := mask }
	  is(Bits("b000011")) { io.ocp.S.Data := source }
	  is(Bits("b000010")) { io.ocp.S.Data := intrPendReg.toBits }
	}
	when(masterReg.Addr(EXC_ADDR_WIDTH-1) === Bits("b1")) {
	  io.ocp.S.Data := vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2))
	}
  }

  when(masterReg.Cmd === OcpCmd.WRNP) {
	io.ocp.S.Resp := OcpResp.DVA
	switch(masterReg.Addr(EXC_ADDR_WIDTH-1, 2)) {
	  is(Bits("b000000")) { status := masterReg.Data }
	  is(Bits("b000001")) { mask := masterReg.Data }
	  is(Bits("b000011")) { source := masterReg.Data }
	  is(Bits("b000010")) {
		for(i <- 0 until EXC_COUNT) {
		  intrPend(i) := masterReg.Data(i)
		}
	  }
	}
	when(masterReg.Addr(EXC_ADDR_WIDTH-1) === Bits("b1")) {
	  vec(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.toUFix
	  vecDup(masterReg.Addr(EXC_ADDR_WIDTH-2, 2)) := masterReg.Data.toUFix
	}
  }

  // Acknowledgement of exception
  when(io.memexc.call) {
	excPend(io.memexc.src) := Bool(false)
	intrPend(io.memexc.src) := Bool(false)
	source := io.memexc.src
	status := status << UFix(1)
  }
  // Return from exception
  when(io.memexc.ret) {
	status := status >> UFix(1)
  }

  // Latch interrupt pins
  for (i <- 0 until INTR_COUNT) {
	when(io.intrs(i)) {
	  intrPend(16+i) := Bool(true)
	}
  }

  // Trigger internal exceptions
  val excAddr = Reg(resetVal = UFix(0, width = PC_SIZE))
  when(io.memexc.exc) {
	excPend(io.memexc.src) := Bool(true)
	excAddr := io.memexc.excAddr
  }

  // Latch new pending flags
  excPendReg := excPend
  intrPendReg := intrPend

  // Compute next exception source
  val src = Bits(width = EXC_SRC_BITS)
  val srcReg = Reg(src)
  src := Bits(0)
  for (i <- 0 until EXC_COUNT) {
	when(intrPend(i) && (mask(i) === Bits(1))) { src := Bits(i) }
  }
  for (i <- 0 until EXC_COUNT) {
	when(excPend(i)) { src := Bits(i) }
  }

  // Create signals to decode stage
  val exc = Reg(excPend.toBits != Bits(0))
  val intr = Reg((intrPend.toBits & mask) != Bits(0))

  io.excdec.exc  := exc
  io.excdec.intr := intr && status(0) === Bits(1)
  io.excdec.addr := vecDup(srcReg)
  io.excdec.src  := srcReg

  io.excdec.excAddr := excAddr
}

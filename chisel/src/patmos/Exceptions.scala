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

  val masterReg = Reg(io.ocp.M)

  val status = Reg(resetVal = Bits(0, width = DATA_WIDTH))
  val mask   = Reg(resetVal = Bits(0, width = DATA_WIDTH))
  val pend   = Reg(resetVal = Bits(0, width = DATA_WIDTH))
  val source = Reg(resetVal = Bits(0, width = DATA_WIDTH))

  val vec = Vec(EXC_COUNT) { Reg(resetVal = UFix(0, width = DATA_WIDTH)) }

  val excAddr = Reg(resetVal = UFix(0, width = PC_SIZE))

  // Default response
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := Bits(0, width = DATA_WIDTH)

  when(masterReg.Cmd === OcpCmd.RD) {
	io.ocp.S.Resp := OcpResp.DVA
	
	switch(masterReg.Addr(5, 2)) {
	  is(Bits("b0000")) { io.ocp.S.Data := status }
	  is(Bits("b0001")) { io.ocp.S.Data := mask }
	  is(Bits("b0010")) { io.ocp.S.Data := pend }
	  is(Bits("b0011")) { io.ocp.S.Data := source }
	}
	when(masterReg.Addr(5) === Bits("b1")) {
	  io.ocp.S.Data := vec(masterReg.Addr(4, 2))
	}
  }

  when(masterReg.Cmd === OcpCmd.WRNP) {
	io.ocp.S.Resp := OcpResp.DVA
	switch(masterReg.Addr(5, 2)) {
	  is(Bits("b0000")) { status := masterReg.Data }
	  is(Bits("b0001")) { mask   := masterReg.Data }
	  is(Bits("b0010")) { pend   := masterReg.Data }
	  is(Bits("b0011")) { source := masterReg.Data }
	}
	when(masterReg.Addr(5) === Bits("b1")) {
	  vec(masterReg.Addr(4, 2)) := masterReg.Data.toUFix
	}
  }

  val trapLatch = Reg(resetVal = Bool(false))
  val faultLatch = Reg(resetVal = Bool(false))
  val intrLatch = Reg(resetVal = Bool(false))

  // Creaty a dummy "interrupt" for testing
  val cnt = Reg(resetVal = UFix(1, width = 10))
  cnt := cnt + UFix(1)
  when(cnt === UFix(0)) {
	intrLatch := Bool(true)
  }

  when(io.memexc.trap) {
	trapLatch := Bool(true)
	excAddr := io.memexc.excAddr
  }

  when(io.memexc.memFault) {
	faultLatch := Bool(true)
	excAddr := io.memexc.excAddr
  }

  when(io.memexc.call) {
	trapLatch := Bool(false)
	faultLatch := Bool(false)
	intrLatch := Bool(false)
	status := status << UFix(1)
  }
  when(io.memexc.ret) {
	status := status >> UFix(1)
  }

  io.excdec.exc  := trapLatch || faultLatch
  io.excdec.intr := intrLatch && status(0) === Bits(1)

  io.excdec.addr := Mux(trapLatch, vec(0),
						Mux(faultLatch, vec(1),
							vec(2)))
  io.excdec.excAddr := excAddr
}

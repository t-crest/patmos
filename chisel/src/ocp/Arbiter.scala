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
 * Arbiter for OCP burst slaves.
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 * 
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * 
 */

package ocp

import Chisel._
import Node._

import scala.collection.mutable.HashMap

class Arbiter(cnt: Int) extends Component {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = Vec(cnt) { new OcpBurstSlavePort(32, 32, 4) }
    val slave = new OcpBurstMasterPort(32, 32, 4)
  }

  val turnReg = Reg(resetVal = UFix(0, log2Up(cnt)))
  val s_idle :: s_busy :: Nil = Enum(2) { UFix() }
  val stateReg = Reg(resetVal = s_idle)
  
  // TODO def turn

  val master = io.master(turnReg).M
  val slave = io.slave.S
  when(stateReg === s_idle) {
    when(master.Cmd != OcpCmd.IDLE) {
      stateReg := s_busy
    }
      .otherwise {
        turnReg := Mux(turnReg === UFix(cnt - 1), UFix(0), turnReg + UFix(1))
      }
  }
  when(stateReg === s_busy) {
      // TODO count the DVAs for the read burst
      when(slave.Resp === OcpResp.DVA) {
        turnReg := Mux(turnReg === UFix(cnt - 1), UFix(0), turnReg + UFix(1))
        stateReg := s_idle
      }
    stateReg := s_idle
  }

  io.slave.M := master
  for (i <- 0 to cnt-1) {
    io.master(i).S.CmdAccept := Bits(0)
    io.master(i).S.DataAccept := Bits(0)
    // we could forward the data to all masters
    io.master(i).S.Resp := Bits(0)
    io.master(i).S.Data := Bits(0)
  }
  io.master(turnReg).S := slave
}



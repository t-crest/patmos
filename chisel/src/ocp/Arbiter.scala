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

class Arbiter(cnt: Int, burstLength: Int) extends Component {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = Vec(cnt) { new OcpBurstSlavePort(32, 32, burstLength) }
    val slave = new OcpBurstMasterPort(32, 32, burstLength)
  }

  val turnReg = Reg(resetVal = UFix(0, log2Up(cnt)))
  val burstCntReg = Reg(resetVal = UFix(0, log2Up(burstLength)))

  val sIdle :: sRead :: sWrite :: Nil = Enum(3) { UFix() }
  val stateReg = Reg(resetVal = sIdle)

  // TODO def turn

  val master = io.master(turnReg).M
  val slave = io.slave.S

  when(stateReg === sIdle) {
    when(master.Cmd != OcpCmd.IDLE) {
      when(master.Cmd === OcpCmd.RD) {
        stateReg := sRead
      }
      when(master.Cmd === OcpCmd.WR) {
        stateReg := sWrite
        burstCntReg := UFix(0)
      }
    }
      .otherwise {
        turnReg := Mux(turnReg === UFix(cnt - 1), UFix(0), turnReg + UFix(1))
      }
  }
  when(stateReg === sWrite) {
    // Just wait on the DVA after the write
    when(slave.Resp === OcpResp.DVA) {
      turnReg := Mux(turnReg === UFix(cnt - 1), UFix(0), turnReg + UFix(1))
      stateReg := sIdle
    }
  }
  when(stateReg === sRead) {
    // For read we have to count the DVAs
    when(slave.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + UFix(1)
      when(burstCntReg === UFix(burstLength) - UFix(1)) {
        turnReg := Mux(turnReg === UFix(cnt - 1), UFix(0), turnReg + UFix(1))
        stateReg := sIdle
      }
    }
  }

  io.slave.M := master
  for (i <- 0 to cnt - 1) {
    io.master(i).S.CmdAccept := Bits(0)
    io.master(i).S.DataAccept := Bits(0)
    io.master(i).S.Resp := Bits(0)
    // we forward the data to all masters
    io.master(i).S.Data := slave.Data
  }
  io.master(turnReg).S := slave
}



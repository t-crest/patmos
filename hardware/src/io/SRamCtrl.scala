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
 * SRAM controller.
 * 
 * First step get a hard-coded version running on DE1-115 or BeMicro
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package io

import Chisel._
import Node._

import ocp._

class SRamIO(addrBits: Int) extends Bundle() {
  val ocpPort = new OcpBurstSlavePort(addrBits, 32, 4)
  val ramIO = Bits(OUTPUT, width=32)
}


class SRamCtrl() extends Module {

  val io = new Bundle {
//    val port = new SRamIO(32)
    val ocpPort = new OcpBurstSlavePort(32, 32, 4)
    val ramIO = Bits(INPUT, width=32)
  }

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = sIdle)

  val master = io.ocpPort.M

  when(stateReg === sIdle) {
    when(master.Cmd != OcpCmd.IDLE) {
      when(master.Cmd === OcpCmd.RD) {
        stateReg := sRead
      }
      when(master.Cmd === OcpCmd.WR) {
        stateReg := sWrite
      }
    }
  }
  when(stateReg === sWrite) {
    // Just go back now
//    when(XXX) {
      stateReg := sIdle
//    }
  }
  when(stateReg === sRead) {
    // 
//    when(YYY) {
        stateReg := sIdle
//    }
  }
  io.ramIO := io.ocpPort.M.Addr

}
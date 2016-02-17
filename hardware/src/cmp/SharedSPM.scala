/*
   Copyright 2016 Technical University of Denmark, DTU Compute.
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
 * Shared SPM. Later with ownership.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package cmp

import Chisel._
import Node._

import patmos.Constants._
import ocp._

class SharedSPM(cnt: Int) extends Module {
  // The names Conf and Spm have no real meaning here.
  // It is just two different types of OCP connections.
  // Legacy from the usage in Argo with two OCP ports.
  val io = new Bundle {
    val comConf = Vec.fill(cnt) { new OcpIOSlavePort(ADDR_WIDTH, DATA_WIDTH) }
    val comSpm = Vec.fill(cnt) { new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH) }
  }

  for (i <- 0 to cnt - 1) {
    println("SPM " + i)
    io.comConf(i).S.Data := UInt(i + 'A')
    // Is it legal OCP to have the response flags hard wired?
    // Probably not.
    io.comConf(i).S.CmdAccept := Bits(1)
    io.comConf(i).S.Resp := OcpResp.DVA
  }
}




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
 * Test for usability of OCP definitions
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package ocp

import Chisel._
import Node._

import scala.collection.mutable.HashMap

class OcpMaster() extends Component {
  val io = new OcpMasterPort(8, 32)

  io.M.Cmd := OcpCmd.IDLE
  io.M.Addr := Bits(0)
  io.M.Data := Bits(0)

  val cnt = Reg(UFix(), resetVal = UFix(0))
  cnt := cnt + UFix(1, 32)

  when(cnt(1, 0) === Bits("b11")) {
	io.M.Cmd := OcpCmd.WR
	io.M.Data := cnt
  }
}

class OcpSlave() extends Component {
  val io = new OcpSlavePort(8, 32)

  val M = Reg(io.M, resetVal = OcpMasterSignals.resetVal(io.M))

  io.S.Resp := OcpResp.NULL
  io.S.Data := M.Data
  when(M.Cmd != OcpCmd.IDLE) {
	io.S.Resp := OcpResp.DVA
  }
}

class Ocp() extends Component {
  val io = Bits(width = 32)

  val master = new OcpMaster()
  val slave = new OcpSlave()
  master.io <> slave.io

  io <> master.io.M.Data
}

object OcpTestMain {
  def main(args: Array[String]): Unit = {
    chiselMain(args, () => new Ocp())
  }
}

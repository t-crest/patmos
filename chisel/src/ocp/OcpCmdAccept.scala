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
 * Deffinitions for OCP ports that support CmdAccept
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package ocp

import Chisel._
import Node._

// Trait for ports that support CmdAccept
trait CommandAccept {
  val CmdAccept = Bits(width = 1)
}

// Master port
class OcpCmdAcceptMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpMasterSignals(addrWidth, dataWidth).asOutput
  val S = (new OcpSlaveSignals(dataWidth) with CommandAccept).asInput 
}

// Slave port is reverse of master port
class OcpCmdAcceptSlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpMasterSignals(addrWidth, dataWidth).asInput
  val S = (new OcpSlaveSignals(dataWidth) with CommandAccept).asOutput
}

// Bridge between ports that do/do not support CmdAccept
class OcpCmdAcceptBridge(addrWidth : Int, dataWidth : Int) extends Component() {
  val io = new Bundle() {
	val master = new OcpSlavePort(addrWidth, dataWidth)
	val slave = new OcpCmdAcceptMasterPort(addrWidth, dataWidth)
  }

  // Register signals that come from master
  val masterReg = Reg(resetVal = OcpMasterSignals.resetVal(io.master.M))
  when(masterReg.Cmd === OcpCmd.IDLE || io.slave.S.CmdAccept) {
	masterReg := io.master.M
  }
  // Forward master signals to slave
  io.slave.M := masterReg

  // Forward slave signals to master
  io.master.S <> io.slave.S
}

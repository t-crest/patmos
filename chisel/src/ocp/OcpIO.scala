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
 * Definitions for Patmos' OCP ports for general I/O
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package ocp

import Chisel._
import Node._

// Masters include a RespAccept signal
class OcpIOMasterSignals(addrWidth : Int, dataWidth : Int)
  extends OcpCoreMasterSignals(addrWidth, dataWidth) {
  val RespAccept = Bits(width = 1)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpIOMasterSignals(addrWidth, dataWidth)
  	res.asInstanceOf[this.type]
  }
}

// Slaves include a CmdAccept signal
class OcpIOSlaveSignals(dataWidth : Int)
  extends OcpSlaveSignals(dataWidth) {
  val CmdAccept = Bits(width = 1)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpIOSlaveSignals(dataWidth)
  	res.asInstanceOf[this.type]
  }
}

// Master port
class OcpIOMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpIOMasterSignals(addrWidth, dataWidth).asOutput
  val S = new OcpIOSlaveSignals(dataWidth).asInput 
}

// Slave port is reverse of master port
class OcpIOSlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpIOMasterSignals(addrWidth, dataWidth).asInput
  val S = new OcpIOSlaveSignals(dataWidth).asOutput
}

// Bridge between ports that do/do not support CmdAccept
class OcpIOBridge(master : OcpCoreMasterPort, slave : OcpIOSlavePort) {
  // Register signals that come from master
  val masterReg = Reg(resetVal = OcpMasterSignals.resetVal(master.M))
  when(masterReg.Cmd === OcpCmd.IDLE || slave.S.CmdAccept) {
	masterReg := master.M
  }
  // Forward master signals to slave, always accept responses
  slave.M := masterReg
  slave.M.RespAccept := Bits("b1")

  // Forward slave signals to master
  master.S <> slave.S
}

// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpIOBus(addrWidth : Int, dataWidth : Int) extends Component {
  val io = new Bundle {
    val slave = new OcpIOSlavePort(addrWidth, dataWidth)
    val master = new OcpIOMasterPort(addrWidth, dataWidth)
  }
  io.master <> io.slave
}

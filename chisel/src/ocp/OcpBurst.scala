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
 * Definitions for OCP ports that support Bursts
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package ocp

import Chisel._
import Node._

// Trait for ports that support DataValid
trait DataValid {
  val DataValid = Bits(width = 1)
}

// Master port
class OcpBurstMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = (new OcpMasterSignals(addrWidth, dataWidth) with DataValid).asOutput
  val S = new OcpSlaveSignals(dataWidth).asInput 
}

// Slave port is reverse of master port
class OcpBurstSlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = (new OcpMasterSignals(addrWidth, dataWidth) with DataValid).asInput
  val S = new OcpSlaveSignals(dataWidth).asOutput
}

// Bridge between word-oriented port and burst port
class OcpBurstBridge(addrWidth : Int, dataWidth : Int, burstLength : Int) extends Component() {
  val io = new Bundle() {
	val master = new OcpSlavePort(addrWidth, dataWidth)
	val slave = new OcpBurstMasterPort(addrWidth, dataWidth)
  }

  val burstAddrBits = log2Up(burstLength)

  // State of transmission
  val idle :: read :: readResp :: write :: writeResp :: Nil = Enum(5){ Bits() }
  val state = Reg(resetVal = idle)
  val burstCnt = Reg(resetVal = UFix(0, burstAddrBits))
  val cmdPos = Reg(resetVal = Bits(0, burstAddrBits))

  // Register signals that come from master
  val masterReg = Reg(resetVal = OcpMasterSignals.resetVal(io.master.M))

  // Register to delay response
  val slaveReg = Reg(resetVal = OcpSlaveSignals.resetVal(io.slave.S))

  masterReg.Cmd := io.master.M.Cmd
  masterReg.Addr := io.master.M.Addr
  when(io.master.M.Cmd === OcpCmd.RD) {
	state := read
	cmdPos := io.master.M.Addr(burstAddrBits+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
  }
  when(io.master.M.Cmd === OcpCmd.WRNP) {
	state := write
	cmdPos := io.master.M.Addr(burstAddrBits+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
	masterReg.Data := io.master.M.Data
	masterReg.ByteEn := io.master.M.ByteEn
  }

  // Default values
  io.slave.M.Cmd := masterReg.Cmd
  io.slave.M.Addr := Cat(masterReg.Addr(addrWidth-1, burstAddrBits+log2Up(dataWidth/8)),
						 Fill(Bits(0), burstAddrBits+log2Up(dataWidth/8)))
  io.slave.M.Data := Bits(0)
  io.slave.M.ByteEn := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.master.S := io.slave.S
  
  // Read burst
  when(state === read) {
	when(io.slave.S.Resp === OcpResp.DVA) {
	  when(burstCnt === cmdPos) {
		slaveReg := io.slave.S
	  }
	  when(burstCnt === UFix(burstLength - 1)) {
		state := readResp
	  }
	  burstCnt := burstCnt + UFix(1)
	}
	io.master.S.Resp := OcpResp.NULL
	io.master.S.Data := Bits(0)
  }
  when(state === readResp) {
	state := idle
	io.master.S := slaveReg
  }
  
  // Write burst
  when(state === write) {
	io.slave.M.DataValid := Bits(1)
	when(burstCnt === cmdPos) {
	  io.slave.M.Data := masterReg.Data
	  io.slave.M.ByteEn := masterReg.ByteEn
	}
	when(burstCnt === UFix(burstLength - 1)) {
	  state := idle
	}
	burstCnt := burstCnt + UFix(1)
  }

}

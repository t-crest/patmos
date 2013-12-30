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

// MS: I would like to follow the Scala/Java naming convention (instead of the OCP naming)

// Burst masters provide handshake signals
class OcpBurstMasterSignals(addrWidth : Int, dataWidth : Int)
  extends OcpMasterSignals(addrWidth, dataWidth) {
  val DataValid = Bits(width = 1)
  val DataByteEn = Bits(width = dataWidth/8)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpBurstMasterSignals(addrWidth, dataWidth)
  	res.asInstanceOf[this.type]
  }

  override def reset() = {
	super.reset()
	DataValid := Bits(0)
	DataByteEn := Bits(0)
  }
}

// Reset values for master signals
object OcpBurstMasterSignals {
  def resetVal[T <: OcpBurstMasterSignals](sig : T) : T = {
	val res = sig.clone
	res.reset()
	res
  }
  def resetVal(addrWidth : Int, dataWidth : Int) : OcpBurstMasterSignals = {
	resetVal(new OcpBurstMasterSignals(addrWidth, dataWidth))
  }
}

// Burst slaves provide handshake signal
class OcpBurstSlaveSignals(dataWidth : Int)
  extends OcpSlaveSignals(dataWidth) {
  val CmdAccept = Bits(width = 1)
  val DataAccept = Bits(width = 1)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpBurstSlaveSignals(dataWidth)
  	res.asInstanceOf[this.type]
  }

  override def reset() = {
	super.reset()
	CmdAccept := Bits(0)
	DataAccept := Bits(0)
  }
}

// Reset values for slave signals
object OcpBurstSlaveSignals {
  def resetVal[T <: OcpBurstSlaveSignals](sig : T) : T = {
	val res = sig.clone
	res.reset()
	res
  }
  def resetVal(dataWidth : Int) : OcpBurstSlaveSignals = {
	resetVal(new OcpBurstSlaveSignals(dataWidth))
  }
}

// Master port
class OcpBurstMasterPort(addrWidth : Int, dataWidth : Int, burstLen : Int) extends Bundle() {
  val burstLength = burstLen
  // Clk is implicit in Chisel
  val M = new OcpBurstMasterSignals(addrWidth, dataWidth).asOutput
  val S = new OcpBurstSlaveSignals(dataWidth).asInput 
}

// Slave port is reverse of master port
class OcpBurstSlavePort(addrWidth : Int, dataWidth : Int, burstLen : Int) extends Bundle() {
  val burstLength = burstLen
  // Clk is implicit in Chisel
  val M = new OcpBurstMasterSignals(addrWidth, dataWidth).asInput
  val S = new OcpBurstSlaveSignals(dataWidth).asOutput

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)
  	res.asInstanceOf[this.type]
  }

}

// Bridge between word-oriented port and burst port
class OcpBurstBridge(master : OcpCacheMasterPort, slave : OcpBurstSlavePort) {
  val addrWidth = master.M.Addr.width
  val dataWidth = master.M.Data.width
  val burstLength = slave.burstLength
  val burstAddrBits = log2Up(burstLength)

  // State of transmission
  val idle :: read :: readResp :: write :: Nil = Enum(Bits(), 4)
  val state = Reg(init = idle)
  val burstCnt = Reg(init = UInt(0, burstAddrBits))
  val cmdPos = Reg(init = Bits(0, burstAddrBits))

  // Register signals that come from master
  val masterReg = Reg(init = OcpCacheMasterSignals.resetVal(master.M))

  // Register to delay response
  val slaveReg = Reg(init = OcpSlaveSignals.resetVal(master.S))

  when(state != write && (masterReg.Cmd === OcpCmd.IDLE || slave.S.CmdAccept === Bits(1))) {
	masterReg := master.M
  }
	
  // Default values
  slave.M.Cmd := masterReg.Cmd
  slave.M.Addr := Cat(masterReg.Addr(addrWidth-1, burstAddrBits+log2Up(dataWidth/8)),
					  Fill(Bits(0), burstAddrBits+log2Up(dataWidth/8)))
  slave.M.Data := Bits(0)
  slave.M.DataByteEn := Bits(0)
  slave.M.DataValid := Bits(0)
  master.S := slave.S
  
  // Read burst
  when(state === read) {
	when(slave.S.Resp === OcpResp.DVA) {
	  when(burstCnt === cmdPos) {
		slaveReg := slave.S
	  }
	  when(burstCnt === UInt(burstLength - 1)) {
		state := readResp
	  }
	  burstCnt := burstCnt + UInt(1)
	}
	master.S.Resp := OcpResp.NULL
	master.S.Data := Bits(0)
  }
  when(state === readResp) {
	state := idle
	master.S := slaveReg
  }
  
  // Write burst
  when(state === write) {
	masterReg.Cmd := OcpCmd.IDLE
	slave.M.DataValid := Bits(1)
	when(burstCnt === cmdPos) {
	  slave.M.Data := masterReg.Data
	  slave.M.DataByteEn := masterReg.ByteEn
	}
	when(burstCnt === UInt(burstLength - 1)) {
	  state := idle
	}
	when(slave.S.DataAccept === Bits(1)) {
	  burstCnt := burstCnt + UInt(1)
	}
  }

  // Start new transaction
  when(master.M.Cmd === OcpCmd.RD) {
	state := read
	cmdPos := master.M.Addr(burstAddrBits+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
  }
  when(master.M.Cmd === OcpCmd.WR) {
	state := write
	cmdPos := master.M.Addr(burstAddrBits+log2Up(dataWidth/8)-1, log2Up(dataWidth/8))
  }
}

// Join two OcpBurst ports
class OcpBurstJoin(left : OcpBurstMasterPort, right : OcpBurstMasterPort,
                   joined : OcpBurstSlavePort) {

  val selRightReg = Reg(init = Bool(false))
  val selRight = Mux(left.M.Cmd != OcpCmd.IDLE, Bool(false),
                     Mux(right.M.Cmd != OcpCmd.IDLE, Bool(true),
                         selRightReg))

  joined.M := left.M
  when (selRight) {
    joined.M := right.M
  }
  joined.M.Cmd := right.M.Cmd | left.M.Cmd

  right.S := joined.S
  left.S := joined.S

  when(selRight) {
    left.S.Resp := OcpResp.NULL
  }
  .otherwise {
    right.S.Resp := OcpResp.NULL
  }

  selRightReg := selRight
}

// Join two OcpBurst ports, left port has priority in case of double request
// the right request is buffered till enable is set to true
class OcpBurstPriorityJoin(left : OcpBurstMasterPort, right : OcpBurstMasterPort,
                   joined : OcpBurstSlavePort, enable : Bool) {

  val selLeft = Mux(left.M.Cmd != OcpCmd.IDLE, Bool(true), Bool(false))
  val selRight = Mux(right.M.Cmd != OcpCmd.IDLE, Bool(true), Bool(false)) 
  val selBothReg = Reg(init = Bool(false))
  val selCurrentReg = Reg(init = Bits(0))
  val masterReg = Reg(init = OcpBurstMasterSignals.resetVal(right.M))

  joined.M := left.M
  //left port requests
  when (selLeft) {
    when (selRight) {
      selBothReg := Bool(true)
      masterReg := right.M
    }
    selCurrentReg := Bits(0)
    joined.M := left.M
  }
  //right port requests
  .elsewhen (selRight || selCurrentReg === Bits(1)) {
    selCurrentReg := Bits(1)
    joined.M := right.M
  }
  //switch to right
  when (selBothReg && enable) {
    selBothReg := Bool(false)
    selCurrentReg := Bits(1)
    joined.M := masterReg
  }
  right.S := joined.S
  left.S := joined.S
  when (selLeft || selCurrentReg === Bits(1)) {
    left.S.Resp := OcpResp.NULL
  }
  .elsewhen (selRight || selCurrentReg === Bits(0)){
    right.S.Resp := OcpResp.NULL
  }
}

// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpBurstBus(addrWidth : Int, dataWidth : Int, burstLen : Int) extends Module {
  val io = new Bundle {
    val slave = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)
    val master = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  }
  io.master <> io.slave
}

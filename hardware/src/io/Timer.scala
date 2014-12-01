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
 * Simple I/O module for timer
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */


package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

object Timer extends DeviceObject {

  def init(params: Map[String, String]) = { }

  def create(params: Map[String, String]) : Timer = {
    Module(new Timer(CLOCK_FREQ))
  }

  trait Pins {
  }

  trait Intrs {
	val timerIntrs = Vec.fill(2) { Bool(OUTPUT) }
  }
}

class Timer(clk_freq: Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with Timer.Pins with Timer.Intrs

  val masterReg = Reg(next = io.ocp.M)

  // Register for cycle counter
  val cycleReg     = Reg(init = UInt(0, 2*DATA_WIDTH))
  val cycleIntrReg = Reg(init = UInt(0, 2*DATA_WIDTH))

  // Registers for usec counter
  val cycPerUSec  = UInt(clk_freq/1000000)
  val usecSubReg  = Reg(init = UInt(0, log2Up(clk_freq/1000000)))
  val usecReg     = Reg(init = UInt(0, 2*DATA_WIDTH))
  val usecIntrReg = Reg(init = UInt(0, 2*DATA_WIDTH))

  // Registers for data to read
  val cycleHiReg  = Reg(Bits(width = DATA_WIDTH))
  val usecHiReg   = Reg(Bits(width = DATA_WIDTH))

  // Registers for writing data
  val cycleLoReg  = Reg(Bits(width = DATA_WIDTH))
  val usecLoReg   = Reg(Bits(width = DATA_WIDTH))

  // Default response
  val resp = Bits()
  val data = Bits(width = DATA_WIDTH)
  resp := OcpResp.NULL
  data := Bits(0)

  // Read current state of timer
  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA

    // Read cycle counter
    // Must read word at higher address first!
    when(masterReg.Addr(3, 2) === Bits("b01")) {
      data := cycleReg(DATA_WIDTH-1, 0)
      cycleHiReg := cycleReg(2*DATA_WIDTH-1, DATA_WIDTH)
    }
    when(masterReg.Addr(3, 2) === Bits("b00")) {
      data := cycleHiReg
    }

    // Read usec counter
    // Must read word at higher address first!
    when(masterReg.Addr(3, 2) === Bits("b11")) {
      data := usecReg(DATA_WIDTH-1, 0)
      usecHiReg := usecReg(2*DATA_WIDTH-1, DATA_WIDTH)
    }
    when(masterReg.Addr(3, 2) === Bits("b10")) {
      data := usecHiReg
    }
  }

  // Write interrupt times
  when(masterReg.Cmd === OcpCmd.WR) {
    resp := OcpResp.DVA

    // Write cycle counter interrupt time
    // Must write word at higher address first!
    when(masterReg.Addr(3, 2) === Bits("b01")) {
      cycleLoReg := masterReg.Data
    }
    when(masterReg.Addr(3, 2) === Bits("b00")) {
      cycleIntrReg := masterReg.Data ## cycleLoReg
    }

    // Write usec counter interrupt time
    // Must write word at higher address first!
    when(masterReg.Addr(3, 2) === Bits("b11")) {
      usecLoReg := masterReg.Data
    }
    when(masterReg.Addr(3, 2) === Bits("b10")) {
      usecIntrReg := masterReg.Data ## usecLoReg
    }
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data

  // No interrupts by default
  io.timerIntrs(0) := Bool(false)
  io.timerIntrs(1) := Bool(false)

  // Increment cycle counter
  cycleReg := cycleReg + UInt(1)
  // Trigger cycles interrupt
  when (cycleReg + UInt(1) === cycleIntrReg) {
    io.timerIntrs(0) := Bool(true)
  }
  // Increment usec counter
  usecSubReg := usecSubReg + UInt(1)
  when(usecSubReg === cycPerUSec - UInt(1)) {
    usecSubReg := UInt(0)
    usecReg := usecReg + UInt(1)
    // Trigger usec interrupt
    when (usecReg + UInt(1) === usecIntrReg) {
      io.timerIntrs(1) := Bool(true)
    }
  }
}

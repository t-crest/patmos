/*
 * Simple I/O module for timer
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */


package io

import Chisel._

import patmos.Constants._

import ocp._

object Timer extends DeviceObject {

  def init(params: Map[String, String]) = { }

  def create(params: Map[String, String]) : Timer = {
    Module(new Timer(CLOCK_FREQ))
  }

  trait Intrs {
	val timerIntrs = Vec.fill(2) { Bool(OUTPUT) }
  }
}

class Timer(clk_freq: Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with Timer.Intrs

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

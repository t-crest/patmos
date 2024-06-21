/*
 * Simple I/O module for timer
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */


package io

import chisel3._
import chisel3.util._

import patmos.Constants._

import ocp._

object Timer extends DeviceObject {

  def init(params: Map[String, String]) = { }

  def create(params: Map[String, String]) : Timer = {
    Module(new Timer(CLOCK_FREQ))
  }
}

class Timer(clk_freq: Int) extends CoreDevice() {

  override val io = IO(new CoreDeviceIO() with patmos.HasInterrupts {
    override val interrupts = Vec(2, Output(Bool()) )
  })

  val masterReg = RegNext(next = io.ocp.M)

  // Register for cycle counter
  val cycleReg     = RegInit(init = 0.U((2*DATA_WIDTH).W))
  val cycleIntrReg = RegInit(init = 0.U((2*DATA_WIDTH).W))

  // Registers for usec counter
  val cycPerUSec  = (clk_freq/1000000).U
  val usecSubReg  = RegInit(init = 0.U(log2Up(clk_freq/1000000).W))
  val usecReg     = RegInit(init = 0.U((2*DATA_WIDTH).W))
  val usecIntrReg = RegInit(init = 0.U((2*DATA_WIDTH).W))

  // Registers for data to read
  val cycleHiReg  = Reg(UInt(DATA_WIDTH.W))
  val usecHiReg   = Reg(UInt(DATA_WIDTH.W))

  // Registers for writing data
  val cycleLoReg  = Reg(UInt(DATA_WIDTH.W))
  val usecLoReg   = Reg(UInt(DATA_WIDTH.W))

  // Default response
  val resp = Wire(Bits())
  val data = Wire(UInt(DATA_WIDTH.W))
  resp := OcpResp.NULL
  data := 0.U

  // Read current state of timer
  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA

    // Read cycle counter
    // Must read word at higher address first!
    when(masterReg.Addr(3, 2) === "b01".U) {
      data := cycleReg(DATA_WIDTH-1, 0)
      cycleHiReg := cycleReg(2*DATA_WIDTH-1, DATA_WIDTH)
    }
    when(masterReg.Addr(3, 2) === "b00".U) {
      data := cycleHiReg
    }

    // Read usec counter
    // Must read word at higher address first!
    when(masterReg.Addr(3, 2) === "b11".U) {
      data := usecReg(DATA_WIDTH-1, 0)
      usecHiReg := usecReg(2*DATA_WIDTH-1, DATA_WIDTH)
    }
    when(masterReg.Addr(3, 2) === "b10".U) {
      data := usecHiReg
    }
  }

  // Write interrupt times
  when(masterReg.Cmd === OcpCmd.WR) {
    resp := OcpResp.DVA

    // Write cycle counter interrupt time
    // Must write word at higher address first!
    when(masterReg.Addr(3, 2) === "b01".U) {
      cycleLoReg := masterReg.Data
    }
    when(masterReg.Addr(3, 2) === "b00".U) {
      cycleIntrReg := masterReg.Data ## cycleLoReg
    }

    // Write usec counter interrupt time
    // Must write word at higher address first!
    when(masterReg.Addr(3, 2) === "b11".U) {
      usecLoReg := masterReg.Data
    }
    when(masterReg.Addr(3, 2) === "b10".U) {
      usecIntrReg := masterReg.Data ## usecLoReg
    }
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data

  // No interrupts by default
  io.interrupts(0) := false.B
  io.interrupts(1) := false.B

  // Increment cycle counter
  cycleReg := cycleReg + 1.U
  // Trigger cycles interrupt
  when (cycleReg + 1.U === cycleIntrReg) {
    io.interrupts(0) := true.B
  }
  // Increment usec counter
  usecSubReg := usecSubReg + 1.U
  when(usecSubReg === cycPerUSec - 1.U) {
    usecSubReg := 0.U
    usecReg := usecReg + 1.U
    // Trigger usec interrupt
    when (usecReg + 1.U === usecIntrReg) {
      io.interrupts(1) := true.B
    }
  }
}

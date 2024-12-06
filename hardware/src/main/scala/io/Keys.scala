/*
 * Simple I/O module for Keys
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import chisel3._

import ocp._

object Keys extends DeviceObject {
  var keyCount = -1

  def init(params: Map[String, String]) = {
    keyCount = getPosIntParam(params, "keyCount")
  }

  def create(params: Map[String, String]) : Keys = {
    Module(new Keys(keyCount))
  }
}

class Keys(keyCount : Int) extends CoreDevice() {

  override val io = IO(new CoreDeviceIO() with patmos.HasPins with patmos.HasInterrupts {
    val pins: Bundle {
      val key: UInt
    } = new Bundle() {
      val key = Input(UInt(keyCount.W))
    }
    override val interrupts = Vec(keyCount, Output(Bool()) )
  })

  val keySyncReg = Reg(UInt(keyCount.W))
  val keyReg = Reg(UInt(keyCount.W))

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Read current state of keys
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := keyReg

  // Connection to pins
  keySyncReg := io.pins.key
  keyReg := keySyncReg

  // Generate interrupts on falling edges
  for (i <- 0 until keyCount) {
    io.interrupts(i) := keyReg(i) === "b1".U && keySyncReg(i) === "b0".U
  }
}

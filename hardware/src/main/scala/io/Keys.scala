/*
 * Simple I/O module for Keys
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import Chisel._

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

  override val io = new CoreDeviceIO() with patmos.HasPins with patmos.HasInterrupts {
    override val pins = new Bundle() {
      val key = Bits(INPUT, keyCount)
    }
    override val interrupts = Vec.fill(keyCount) { Bool(OUTPUT) }
  }

  val keySyncReg = Reg(Bits(width = keyCount))
  val keyReg = Reg(Bits(width = keyCount))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
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
    io.interrupts(i) := keyReg(i) === Bits("b1") && keySyncReg(i) === Bits("b0")
  }
}

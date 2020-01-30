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

  trait Pins {
    val keysPins = Wire(new Bundle() {
      val key = Bits(INPUT, keyCount)
    })
  }

  trait Intrs {
    val keysIntrs = Wire(Vec(keyCount, Bool().asOutput ))
  }
}

class Keys(keyCount : Int) extends CoreDevice() {

  override val io = IO(new CoreDeviceIO() with Keys.Pins with Keys.Intrs )

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
  keySyncReg := io.keysPins.key
  keyReg := keySyncReg

  // Generate interrupts on falling edges
  for (i <- 0 until keyCount) {
    io.keysIntrs(i) := keyReg(i) === Bits("b1") && keySyncReg(i) === Bits("b0")
  }
}

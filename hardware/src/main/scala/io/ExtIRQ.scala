/*
 * Simple I/O module for external interrupts
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import Chisel._

import ocp._

object ExtIRQ extends DeviceObject {
  var IRQCount = -1

  def init(params: Map[String, String]) = {
    IRQCount = getPosIntParam(params, "IRQCount")
  }

  def create(params: Map[String, String]) : ExtIRQ = {
    Module(new ExtIRQ(IRQCount))
  }

  trait Pins {
    val extIRQPins = new Bundle() {
      val irq = Bits(INPUT, IRQCount)
    }
  }

  trait Intrs {
    val extIRQIntrs = Vec.fill(IRQCount) { Bool(OUTPUT) }
  }
}

class ExtIRQ(IRQCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with ExtIRQ.Pins with ExtIRQ.Intrs

  val IRQSyncReg = Reg(Bits(width = IRQCount))
  val IRQSyncedReg = Reg(Bits(width = IRQCount))

  val IRQReg = Reg(Bits(width = IRQCount))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Connections to master
  io.ocp.S.Resp := respReg

  // Connection to pins
  IRQSyncReg := io.extIRQPins.irq
  IRQSyncedReg := IRQSyncReg
  IRQReg := IRQSyncedReg

  // Generate interrupts on rising edges
  for (i <- 0 until IRQCount) {
    io.extIRQIntrs(i) := IRQReg(i)
  }
}

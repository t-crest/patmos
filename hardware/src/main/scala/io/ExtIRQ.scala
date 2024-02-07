/*
 * Simple I/O module for external interrupts
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import Chisel._
import chisel3.VecInit

import ocp._

object ExtIRQ extends DeviceObject {
  var IRQCount = -1

  def init(params: Map[String, String]) = {
    IRQCount = getPosIntParam(params, "IRQCount")
  }

  def create(params: Map[String, String]) : ExtIRQ = {
    Module(new ExtIRQ(IRQCount))
  }
}

class ExtIRQ(IRQCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with patmos.HasPins with patmos.HasInterrupts {
    override val pins = new Bundle() {
      val irq = Bits(INPUT, IRQCount)
    }
    override val interrupts = Output(VecInit(Seq.fill(IRQCount)(Bool())))
  }

  val IRQSyncReg = Reg(Bits(width = IRQCount))
  val IRQSyncedReg = Reg(Bits(width = IRQCount))

  val IRQReg = Reg(Bits(width = IRQCount))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Connections to master
  io.ocp.S.Resp := respReg

  // Connection to pins
  IRQSyncReg := io.pins.irq
  IRQSyncedReg := IRQSyncReg
  IRQReg := IRQSyncedReg

  // Generate interrupts on rising edges
  for (i <- 0 until IRQCount) {
    io.interrupts(i) := IRQReg(i)
  }
}

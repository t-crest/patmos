/*
 * Simple I/O module for external interrupts
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import chisel3._
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
    val pins: Bundle {
      val irq: UInt
    } = new Bundle() {
      val irq = Input(UInt(IRQCount.W))
    }
    override val interrupts = Output(VecInit(Seq.fill(IRQCount)(Bool())))
  }

  val IRQSyncReg = Reg(UInt(IRQCount.W))
  val IRQSyncedReg = Reg(UInt(IRQCount.W))

  val IRQReg = Reg(UInt(IRQCount.W))

  // Default response
  val respReg = RegInit(init = OcpResp.NULL)
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

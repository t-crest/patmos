/*
 * Interface to Shibarchi's MPU sensor interface (imu_mpu)
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package io

import Chisel._

import patmos.Constants._

import ocp._

object AauMpu extends DeviceObject {

  def init(params: Map[String, String]) = {}

  def create(params: Map[String, String]): AauMpu = Module(new AauMpu())

  trait Pins {
    val aauMpuPins = new Bundle() {
      val data = Vec(10, Bits(INPUT, 32))
    }
  }
}

class AauMpu() extends CoreDevice() {
  
  override val io = new CoreDeviceIO() with AauMpu.Pins

  // read data shall be in a register as used in the
  // following clock cycle
  val dataReg = RegNext(io.aauMpuPins.data(io.ocp.M.Addr(5,2)))
  
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Set DVA on a read or write
  when(io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
  }

  io.ocp.S.Data := dataReg
  io.ocp.S.Resp := respReg
}

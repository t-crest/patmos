/*
 * I2C interface for Patmos (just the ocp interface for now)
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import chisel3._

object I2CInterface extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : I2CInterface = {
    Module(new I2CInterface(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }
}

class I2CInterface(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val MCmd: UInt
      val MAddr: UInt
      val MData: UInt
      val MByteEn: UInt
      val SResp: UInt
      val SData: UInt
    } = new Bundle() {
      val MCmd = Output(UInt(3.W))
      val MAddr = Output(UInt(extAddrWidth.W))
      val MData = Output(UInt(dataWidth.W))
      val MByteEn = Output(UInt(4.W))
      val SResp = Input(UInt(2.W))
      val SData = Input(UInt(dataWidth.W))
    }
  }
  //Assigments of inputs and outputs
  io.pins.MCmd := io.ocp.M.Cmd
  io.pins.MAddr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.pins.MData := io.ocp.M.Data
  io.pins.MByteEn := io.ocp.M.ByteEn
  io.ocp.S.Resp := io.pins.SResp
  io.ocp.S.Data := io.pins.SData
}

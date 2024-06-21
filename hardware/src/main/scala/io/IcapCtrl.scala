/*
 * ICAP Controller interface for Patmos
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import chisel3._

object IcapCtrl extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : IcapCtrl = {
    Module(new IcapCtrl(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }
}

class IcapCtrl(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
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

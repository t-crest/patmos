/*
 * I2C with subaddress interface for Patmos (just the ocp interface for now)
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object I2CSubAddr extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : I2CSubAddr = {
    Module(new I2CSubAddr(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val i2CSubAddrPins = new Bundle() {
      val MCmd = UInt(OUTPUT,3)
      val MAddr = UInt(OUTPUT,extAddrWidth)
      val MData = UInt(OUTPUT,dataWidth)
      val MByteEn = UInt(OUTPUT,4)
      val SResp = UInt(INPUT,2)
      val SData = UInt(INPUT,dataWidth)
    }
  }
}

class I2CSubAddr(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with I2CSubAddr.Pins
  //Assigments of inputs and outputs
  io.i2CSubAddrPins.MCmd := io.ocp.M.Cmd
  io.i2CSubAddrPins.MAddr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.i2CSubAddrPins.MData := io.ocp.M.Data
  io.i2CSubAddrPins.MByteEn := io.ocp.M.ByteEn
  io.ocp.S.Resp := io.i2CSubAddrPins.SResp
  io.ocp.S.Data := io.i2CSubAddrPins.SData
}

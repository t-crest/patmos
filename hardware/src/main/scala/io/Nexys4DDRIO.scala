/*
 * Multi IO interface for Patmos in the Digilent Nexys4DDR board
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object Nexys4DDRIO extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : Nexys4DDRIO = {
    Module(new Nexys4DDRIO(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val nexys4DDRIOPins = new Bundle() {
      val MCmd = UInt(OUTPUT,3)
      val MAddr = UInt(OUTPUT,extAddrWidth)
      val MData = UInt(OUTPUT,dataWidth)
      val MByteEn = UInt(OUTPUT,4)
      val SResp = UInt(INPUT,2)
      val SData = UInt(INPUT,dataWidth)
    }
  }
}

class Nexys4DDRIO(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with Nexys4DDRIO.Pins
  //Assigments of inputs and outputs
  io.nexys4DDRIOPins.MCmd := io.ocp.M.Cmd
  io.nexys4DDRIOPins.MAddr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.nexys4DDRIOPins.MData := io.ocp.M.Data
  io.nexys4DDRIOPins.MByteEn := io.ocp.M.ByteEn
  io.ocp.S.Resp := io.nexys4DDRIOPins.SResp
  io.ocp.S.Data := io.nexys4DDRIOPins.SData
}

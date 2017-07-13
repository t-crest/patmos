/*
 * AudioBuffer interface for Patmos (just the ocp interface for now)
 *
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object AudioBuffer extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : AudioBuffer = {
    Module(new AudioBuffer(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val audioBufferPins = new Bundle() {
      val MCmd = UInt(OUTPUT,3)
      val MAddr = UInt(OUTPUT,extAddrWidth)
      val MData = UInt(OUTPUT,dataWidth)
      val MByteEn = UInt(OUTPUT,4)
      val SResp = UInt(INPUT,2)
      val SData = UInt(INPUT,dataWidth)
    }
  }
}

class AudioBuffer(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with AudioBuffer.Pins
  //Assigments of inputs and outputs
  io.audioBufferPins.MCmd := io.ocp.M.Cmd
  io.audioBufferPins.MAddr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.audioBufferPins.MData := io.ocp.M.Data
  io.audioBufferPins.MByteEn := io.ocp.M.ByteEn
  io.ocp.S.Resp := io.audioBufferPins.SResp
  io.ocp.S.Data := io.audioBufferPins.SData
}

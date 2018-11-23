/*
 * SSPMio, used to give Patmos the io ports/pins for connecting SSPMAegean when
 * it is implemented is a multicore processor.
 *
 * Authors: Andreas T. Kristensen (s144026@student.dtu.dk)
 */

package io

import Chisel._

object SSPMio extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : SSPMio = {
    Module(new SSPMio(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  // Pins are the external interfaces for Patmos
  trait Pins {
    val sSPMioPins = new Bundle() {
      val M = new Bundle() {
        val Cmd    = UInt(OUTPUT,3)
        val Addr   = UInt(OUTPUT,extAddrWidth)
        val Data   = UInt(OUTPUT,dataWidth)
        val ByteEn = UInt(OUTPUT,4)
      }

      val S = new Bundle() {
        val Data   = UInt(INPUT,dataWidth)
        val Resp = UInt(INPUT, 2)
      }
    }
  }
}

class SSPMio(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with SSPMio.Pins

  // Assigments of inputs and outputs
  // These are simply passed through as SSPMAegean contains the logic
  io.sSPMioPins.M.Cmd  := io.ocp.M.Cmd
  io.sSPMioPins.M.Addr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.sSPMioPins.M.Data := io.ocp.M.Data
  io.sSPMioPins.M.ByteEn := io.ocp.M.ByteEn
  io.ocp.S.Data := io.sSPMioPins.S.Data
  io.ocp.S.Resp := io.sSPMioPins.S.Resp
}


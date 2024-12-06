/*
 * SSPMio, used to give Patmos the io ports/pins for connecting SSPMAegean when
 * it is implemented is a multicore processor.
 *
 * Authors: Andreas T. Kristensen (s144026@student.dtu.dk)
 */

package io

import chisel3._

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
  trait Pins extends patmos.HasPins {
    val pins: Bundle {
      val M: Bundle {
        val Cmd: UInt
        val Addr: UInt
        val Data: UInt
        val ByteEn: UInt
      }
      val S: Bundle {
        val Data: UInt
        val Resp: UInt
      }
    } = new Bundle() {
      val M = new Bundle() {
        val Cmd    = Output(UInt(3.W))
        val Addr   = Output(UInt(extAddrWidth.W))
        val Data   = Output(UInt(dataWidth.W))
        val ByteEn = Output(UInt(4.W))
      }

      val S = new Bundle() {
        val Data   = Input(UInt(dataWidth.W))
        val Resp = Input(UInt(2.W))
      }
    }
  }
}

class SSPMio(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with SSPMio.Pins

  // Assigments of inputs and outputs
  // These are simply passed through as SSPMAegean contains the logic
  io.pins.M.Cmd  := io.ocp.M.Cmd
  io.pins.M.Addr := io.ocp.M.Addr(extAddrWidth-1, 0)
  io.pins.M.Data := io.ocp.M.Data
  io.pins.M.ByteEn := io.ocp.M.ByteEn
  io.ocp.S.Data := io.pins.S.Data
  io.ocp.S.Resp := io.pins.S.Resp
}


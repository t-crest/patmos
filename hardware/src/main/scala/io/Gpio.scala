/*
 * Simple GPIO module with addressable banks of N-bit width and configurable port direction.
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import Chisel._
import patmos.Constants._
import ocp._

object Gpio extends DeviceObject {
  var bankCount = 1
  var bankWidth = 1
  var ioDirection : IODirection = OUTPUT //type declaration necessary otherwise scala complains

  def init(params: Map[String, String]) = {
    bankCount = getPosIntParam(params, "bankCount")
    bankWidth = getPosIntParam(params, "bankWidth")
    if("output" == getParam(params, "ioDirection")){
      ioDirection = OUTPUT
    } else if ("input" == getParam(params, "ioDirection")){
      ioDirection = INPUT
    }
  }

  def create(params: Map[String, String]) : Gpio = {
          Module(new Gpio(bankCount, bankWidth, ioDirection))
  }
}

class Gpio(bankCount: Int, bankWidth: Int, ioDirection: IODirection) extends CoreDevice() {
  // Override
  override val io = new CoreDeviceIO() with patmos.HasPins {
    override val pins = new Bundle() {
      val gpios = Vec.fill(bankCount) {Bits(ioDirection, bankWidth)}
    }
  }

  //Constants
  val constAddressWidth : Int = log2Up(bankCount) + 2

  // Master register
  val masterReg = Reg(next = io.ocp.M)

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))

  // Display register
  val gpioRegVec = RegInit(Vec.fill(bankCount){Bits(0, width = bankWidth)})

  if(ioDirection == OUTPUT){
    // Read/Write gpios
    for(i <- 0 until bankCount by 1){
      when(masterReg.Addr(constAddressWidth-1, 2) === i.U){
        when(masterReg.Cmd === OcpCmd.WR){
          gpioRegVec(i) := masterReg.Data(bankWidth-1, 0) //Drive gpios from OCP
        }
        dataReg := gpioRegVec(i)
      }
    }

    when(masterReg.Cmd === OcpCmd.WR || masterReg.Cmd === OcpCmd.RD){
      respReg := OcpResp.DVA
    }

  } else if(ioDirection == INPUT) {
    // Read gpios
    for(i <- 0 until bankCount by 1){
      when(masterReg.Addr(constAddressWidth-1, 2) === i.U){
        dataReg := gpioRegVec(i)
      }
    }

    when(masterReg.Cmd === OcpCmd.RD){
      respReg := OcpResp.DVA
    } .elsewhen(masterReg.Cmd === OcpCmd.WR) {
      respReg := OcpResp.ERR
    } .otherwise {
      respReg := OcpResp.NULL
    }
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataReg

  // Connections to IO
  io.pins.gpios := gpioRegVec

}


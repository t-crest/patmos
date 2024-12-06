/*
 * Simple GPIO module with addressable banks of N-bit width and configurable port direction.
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import chisel3._
import chisel3.experimental.Analog
import chisel3.util._
import patmos.Constants._
import ocp._

object Gpio extends DeviceObject {
  var bankCount = 1
  var bankWidth = 1
  var ioDirection : Boolean = false //type declaration necessary otherwise scala complains

  def init(params: Map[String, String]) = {
    bankCount = getPosIntParam(params, "bankCount")
    bankWidth = getPosIntParam(params, "bankWidth")
    if("Output".equalsIgnoreCase(getParam(params, "ioDirection"))){
      ioDirection = false
    } else if ("Input".equalsIgnoreCase(getParam(params, "ioDirection"))){
      ioDirection = true
    }
  }

  def create(params: Map[String, String]) : Gpio = {
          Module(new Gpio(bankCount, bankWidth, ioDirection))
  }
}

class Gpio(bankCount: Int, bankWidth: Int, ioDirection: Boolean = false) extends CoreDevice() {
  // Override
  override val io = IO(new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val gpios: Vec[UInt]
    } = new Bundle() {
      val gpios = if (!ioDirection) Output(Vec(bankCount,  UInt(bankWidth.W)))
                  else Input(Vec(bankCount,  UInt(bankWidth.W)))
    }
  })

  //Constants
  val constAddressWidth : Int = log2Up(bankCount) + 2

  // Master register
  val masterReg = RegNext(io.ocp.M)

  // Default response
  val respReg = RegInit(OcpResp.NULL)
  respReg := OcpResp.NULL

  val dataReg = RegInit(0.U(DATA_WIDTH.W))

  // Display register
  val gpioRegVec = RegInit(VecInit(Seq.fill(bankCount)(0.U(bankWidth.W))))

  if(!ioDirection){
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

  } else if(ioDirection) {
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
  io.pins.gpios <> gpioRegVec

}


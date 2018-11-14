/*
   Copyright 2018 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Simple GPIO module with addressable banks of N-bit width and configurable port direction.
 *
 * Authors: Eleftherios Kyriakakis (elky@dtu.dk)
 *
 */

package io

import Chisel._
import Chisel.Node._
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

  trait Pins {
      val gpioPins = new Bundle() {
          val gpios = Vec.fill(bankCount) {Bits(ioDirection, bankWidth)}
      }
  }
}

class Gpio(bankCount: Int, bankWidth: Int, ioDirection: IODirection) extends CoreDevice() {
  // Override
  override val io = new CoreDeviceIO() with Gpio.Pins

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
  io.gpioPins.gpios := gpioRegVec

}


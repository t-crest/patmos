/*
   Copyright 2013 Technical University of Denmark, DTU Compute. 
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
 * Simple I/O module for LEDs
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 * 
 */

package io

import Chisel._
import Node._

import ocp._

import patmos.Constants._

object Leds extends DeviceObject {
  var ledCount = -1

  def create(params: Map[String, String]) : Leds = {
    ledCount = getPosIntParam(params, "ledCount")
    new Leds(ledCount)
  }

  trait Pins {
    val ledsPins = new Bundle() {
      val led = Bits(OUTPUT, ledCount)
    }
  }
}

class Leds(ledCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with Leds.Pins

  val ledReg = Reg(resetVal = Bits(0, ledCount))

  // Default response
  val respReg = Reg(resetVal = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Write to LEDs
  when(io.ocp.M.Cmd === OcpCmd.WR) {
	respReg := OcpResp.DVA
    ledReg := io.ocp.M.Data(ledCount-1, 0)
  }

  // Read current state of LEDs
  when(io.ocp.M.Cmd === OcpCmd.RD) {
	respReg := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := ledReg

  // Connection to pins
  io.ledsPins.led := ledReg
}

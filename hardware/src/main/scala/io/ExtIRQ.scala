/*
   Copyright 2015 Technical University of Denmark, DTU Compute.
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
 * Simple I/O module for external interrupts
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

object ExtIRQ extends DeviceObject {
  var IRQCount = -1

  def init(params: Map[String, String]) = {
    IRQCount = getPosIntParam(params, "IRQCount")
  }

  def create(params: Map[String, String]) : ExtIRQ = {
    Module(new ExtIRQ(IRQCount))
  }

  trait Pins {
    val extIRQPins = new Bundle() {
      val irq = Bits(INPUT, IRQCount)
    }
  }

  trait Intrs {
    val extIRQIntrs = Vec.fill(IRQCount) { Bool(OUTPUT) }
  }
}

class ExtIRQ(IRQCount : Int) extends CoreDevice() {

  override val io = new CoreDeviceIO() with ExtIRQ.Pins with ExtIRQ.Intrs

  val IRQSyncReg = Reg(Bits(width = IRQCount))
  val IRQSyncedReg = Reg(Bits(width = IRQCount))

  val IRQReg = Reg(Bits(width = IRQCount))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Connections to master
  io.ocp.S.Resp := respReg

  // Connection to pins
  IRQSyncReg := io.extIRQPins.irq
  IRQSyncedReg := IRQSyncReg
  IRQReg := IRQSyncedReg

  // Generate interrupts on rising edges
  for (i <- 0 until IRQCount) {
    io.extIRQIntrs(i) := IRQReg(i)
  }
}

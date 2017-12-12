/*
   Copyright 2017 Technical University of Denmark, DTU Compute.
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
 * SSPMio, used to give Patmos the io ports/pins for connecting SSPMAegean when
 * it is implemented is a multicore processor.
 *
 * Authors: Andreas T. Kristensen (s144026@student.dtu.dk)
 */

package io

import Chisel._
import Node._
import ocp._

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


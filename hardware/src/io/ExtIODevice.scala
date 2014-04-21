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
 * A connection to an external IO device
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import scala.math._

import Chisel._
import Node._
import ocp._
import patmos.Constants._

object ExtIODevice extends DeviceObject {
    var extAddrWidth = 32
    var dataWidth = 32

    def init(params : Map[String, String]) = {
        extAddrWidth = getPosIntParam(params, "extAddrWidth")
        dataWidth = getPosIntParam(params, "dataWidth")
    }

    def create(params: Map[String, String]) : ExtIODevice = {
        Module(new ExtIODevice(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
    }

    trait Pins {
        val extIODevicePins = new Bundle() {
         val ocp = new OcpIOMasterPort(extAddrWidth, dataWidth)
      }
    }
}

class ExtIODevice(extAddrWidth : Int = 32,
                dataWidth : Int = 32) extends CoreDevice() {
    override val io = new CoreDeviceIO() with ExtIODevice.Pins

   val coreBus = Module(new OcpCoreBus(extAddrWidth,dataWidth))
   val ioBus = Module(new OcpIOBus(extAddrWidth,dataWidth))
   io.ocp <> coreBus.io.slave
   ioBus.io.master <> io.extIODevicePins.ocp

    val bridge = new OcpIOBridge(coreBus.io.master,ioBus.io.slave)
}

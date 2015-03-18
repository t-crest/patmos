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
 * A connection to an Avalon-MM device
 *
 * Authors: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object AvalonMMBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : AvalonMMBridge = {
    Module(new AvalonMMBridge(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val avalonMMBridgePins = new Bundle() {
      val avs_write_n = Bool(OUTPUT)
      val avs_read_n = Bool(OUTPUT)
      val avs_address = UInt(OUTPUT,extAddrWidth)
      val avs_writedata = UInt(OUTPUT,dataWidth)
      val avs_readdata = UInt(INPUT,dataWidth)
    }
  }
}

class AvalonMMBridge(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with AvalonMMBridge.Pins

  val respReg = Reg(init = OcpResp.NULL)
  val dataReg = Reg(init = Bits(0, dataWidth))
  // Default values in case of ILDE command
  respReg := OcpResp.NULL
  dataReg := Bits(0)
  io.avalonMMBridgePins.avs_write_n := Bool(true)
  io.avalonMMBridgePins.avs_read_n := Bool(true)

  // TODO: handle error if byte enable is not "1111"
  //io.ocp.M.ByteEn

  // Connecting address and data signal straight through
  io.avalonMMBridgePins.avs_address := io.ocp.M.Addr(extAddrWidth-1+2, 2)
  io.avalonMMBridgePins.avs_writedata := io.ocp.M.Data(dataWidth-1, 0)
  //io.ocp.S.Data(dataWidth-1, 0) := io.avalonMMBridgePins.avs_readdata(dataWidth-1, 0)
  io.ocp.S.Data := dataReg

  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    io.avalonMMBridgePins.avs_write_n := Bool(false)
    io.avalonMMBridgePins.avs_read_n := Bool(true)
  }
  when(io.ocp.M.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    io.avalonMMBridgePins.avs_write_n := Bool(true)
    io.avalonMMBridgePins.avs_read_n := Bool(false)
    dataReg := io.avalonMMBridgePins.avs_readdata
  }

  // Sending the generated response to OCP master
  io.ocp.S.Resp := respReg
}

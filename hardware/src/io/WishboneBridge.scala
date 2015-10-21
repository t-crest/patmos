/*
   Copyright 2014 Technical University of Denmark, DTU Compute.
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
 * OCP (Master) -> Wishbone (Master) bridge for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object WishboneBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : WishboneBridge = {
    Module(new WishboneBridge(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val wishboneBridgePins = new Bundle() {
      val wb_addr_o = UInt(OUTPUT,extAddrWidth)
      val wb_data_i = UInt(INPUT,dataWidth)
      val wb_err_i = Bool(INPUT)
      val wb_data_o = UInt(OUTPUT,dataWidth)
      val wb_we_o = Bool(OUTPUT)
      val wb_sel_o = UInt(OUTPUT,4)
      val wb_stb_o = Bool(OUTPUT)
      val wb_ack_i = Bool(INPUT)
      val wb_cyc_o = Bool(OUTPUT)
    }
  }
}

class WishboneBridge(extAddrWidth : Int = 32,
                     dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with WishboneBridge.Pins
  // Registers
  val we_o_Reg = Reg(init = Bits(0,width=1))
  val stb_o_Reg = Reg(init = Bits(0,width=1))
  val cyc_o_Reg = Reg(init = Bits(0,width=1))
  val addr_o_Reg = Reg(init = Bits(0,width=extAddrWidth))
  val data_o_Reg = Reg(init = Bits(0,width=dataWidth))
  
  val ocp_S_data_Reg = Reg(init = Bits(0,width=dataWidth))
  val ocp_S_resp_Reg = Reg(init = OcpResp.NULL)

  // Default values and assigments of outputs
  ocp_S_resp_Reg := OcpResp.NULL
  io.ocp.S.Resp := ocp_S_resp_Reg
  io.ocp.S.Data := ocp_S_data_Reg
  io.wishboneBridgePins.wb_sel_o := Bits("b1111")
  io.wishboneBridgePins.wb_we_o := we_o_Reg 
  io.wishboneBridgePins.wb_stb_o := stb_o_Reg
  io.wishboneBridgePins.wb_cyc_o := cyc_o_Reg
  io.wishboneBridgePins.wb_addr_o := addr_o_Reg
  io.wishboneBridgePins.wb_data_o := data_o_Reg

  // Read command
  when(io.ocp.M.Cmd === OcpCmd.RD) {
  we_o_Reg := Bool(false)
  stb_o_Reg := Bool(true)
  cyc_o_Reg := Bool(true)
  addr_o_Reg := io.ocp.M.Addr(extAddrWidth-1, 0)
  }

  // Write command
  when(io.ocp.M.Cmd === OcpCmd.WR) {
  we_o_Reg := Bool(true)
  stb_o_Reg := Bool(true)
  cyc_o_Reg := Bool(true)
  addr_o_Reg := io.ocp.M.Addr(extAddrWidth-1, 0)
  data_o_Reg := io.ocp.M.Data
  }

  // Transaltion of the WB's ack into OCP's DVA
  when(io.wishboneBridgePins.wb_ack_i === Bits(1)) {
  we_o_Reg := Bool(false)
  stb_o_Reg := Bool(false)
  cyc_o_Reg := Bool(false)
  ocp_S_resp_Reg := OcpResp.DVA
  ocp_S_data_Reg := io.wishboneBridgePins.wb_data_i
  }
}

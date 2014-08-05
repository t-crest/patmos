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
 * OCP interface for LogiCORE IP Virtex-6 FPGA Embedded Tri-Mode Ethernet
 * MAC Wrapper v2.3
 *
 * Authors: Torur Strom (torur.strom@gmail.com)
 *
 */

package io

import Chisel._
import Node._

import ocp._

object EMAC extends DeviceObject {

  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]) : EMAC = {
    Module(new EMAC())
  }

  trait Pins {
    val eMACPins = new Bundle() {
  	val rx_axis_fifo_tdata = Bits(INPUT,8)
  	val rx_axis_fifo_tvalid = Bits(INPUT,1)
  	val rx_axis_fifo_tlast = Bits(INPUT,1)
	val rx_axis_fifo_tready = Bits(OUTPUT,1)

  	val tx_axis_fifo_tdata = Bits(OUTPUT,8)
  	val tx_axis_fifo_tvalid = Bits(OUTPUT,1)
  	val tx_axis_fifo_tlast = Bits(OUTPUT,1)
	val tx_axis_fifo_tready = Bits(INPUT,1)
    }
  }
}

class EMAC() extends CoreDevice() {

  override val io = new CoreDeviceIO() with EMAC.Pins

  val rx_axis_fifo_tready_Reg = Reg(init = Bits(0,1))
  rx_axis_fifo_tready_Reg := Bits(0,1)
  io.eMACPins.rx_axis_fifo_tready := rx_axis_fifo_tready_Reg

  val tx_axis_fifo_tvalid_Reg = Reg(init = Bits(0,1))
  tx_axis_fifo_tvalid_Reg := Bits(0,1)
  io.eMACPins.tx_axis_fifo_tvalid := tx_axis_fifo_tvalid_Reg

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Data from EMAC
  val dataRdReg = Reg(init = Bits(0,32))
 

  // Data to EMAC
  val dataWrReg = Reg(init = Bits(0,32))
  io.eMACPins.tx_axis_fifo_tlast := dataWrReg(30)
  io.eMACPins.tx_axis_fifo_tdata := dataWrReg(7,0)

  val sIdle :: sWait :: sRead :: Nil = Enum(UInt(), 3)
  val state = Reg(init = sIdle)
  
  when(io.ocp.M.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    tx_axis_fifo_tvalid_Reg := Bits(1,1)
    dataWrReg := io.ocp.M.Data
  }
  
  when(state === sIdle) {
    when(io.ocp.M.Cmd === OcpCmd.RD) {
      when(io.ocp.M.Addr(0) === Bool(false)) {
        state := sWait
        rx_axis_fifo_tready_Reg := Bits(1,1)
      }
      .otherwise {
        respReg := OcpResp.DVA
		dataRdReg := Cat(io.eMACPins.tx_axis_fifo_tready,Bits(0,31))
      }
    }
  }
  when(state === sWait) {
    state := sRead
  }
  when(state === sRead) {
    state := sIdle
    respReg := OcpResp.DVA
	dataRdReg := Cat(io.eMACPins.rx_axis_fifo_tvalid,Cat(io.eMACPins.rx_axis_fifo_tlast,Cat(Bits(0,22),io.eMACPins.rx_axis_fifo_tdata)))
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataRdReg

}

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
 * I/O module for LogiCORE IP Virtex-6 FPGA Embedded Tri-Mode Ethernet
 * MAC Wrapper v2.3
 *
 * Authors: Torur Strom (torur.strom@gmail.com)
 *
 */

package io

import Chisel._
import Node._

import ocp._

class v6_emac_v2_3_wrapper extends BlackBox {
  val io = new Bundle{
    // asynchronous reset
	val glbl_rst = Bits(INPUT,1)
	// 200MHz clock input from board
	val clk_in_p = Bits(INPUT,1)
	val clk_in_n = Bits(INPUT,1)
	// Patmos clock
	val gtx_clk_bufg = Bits(INPUT,1)
	// Physical reset
	val phy_resetn = Bits(OUTPUT,1)
	
	// GMII Interface
	val gmii_txd = Bits(OUTPUT,8)
	val gmii_tx_en = Bits(OUTPUT,1)
	val gmii_tx_er = Bits(OUTPUT,1)
	val gmii_tx_clk = Bits(OUTPUT,1)
	val gmii_rxd = Bits(INPUT,8)
	val gmii_rx_dv = Bits(INPUT,1)
	val gmii_rx_er = Bits(INPUT,1)
	val gmii_rx_clk = Bits(INPUT,1)
	val gmii_col = Bits(INPUT,1)
	val gmii_crs = Bits(INPUT,1)
	val mii_tx_clk = Bits(INPUT,1)
	  
	// OCP interface
	val rx_axis_fifo_tdata = Bits(OUTPUT,8)
	val rx_axis_fifo_tvalid = Bits(OUTPUT,1)
	val rx_axis_fifo_tlast = Bits(OUTPUT,1)
	val rx_axis_fifo_tready = Bits(INPUT,1)
	
	val tx_axis_fifo_tdata = Bits(INPUT,8)
	val tx_axis_fifo_tvalid = Bits(INPUT,1)
	val tx_axis_fifo_tlast = Bits(INPUT,1)
	val tx_axis_fifo_tready = Bits(OUTPUT,1)
	  
  } 
}

object EMAC extends DeviceObject {

  def init(params: Map[String, String]) = {
  }

  def create(params: Map[String, String]) : EMAC = {
    Module(new EMAC())
  }

  trait Pins {
    val ledsPins = new Bundle() {
      // asynchronous reset
	val glbl_rst = Bits(INPUT,1)
	// 200MHz clock input from board
	val clk_in_p = Bits(INPUT,1)
	val clk_in_n = Bits(INPUT,1)
	// Patmos clock
	val gtx_clk_bufg = Bits(INPUT,1)
	// Physical reset
	val phy_resetn = Bits(OUTPUT,1)
	
	// GMII Interface
	val gmii_txd = Bits(OUTPUT,8)
	val gmii_tx_en = Bits(OUTPUT,1)
	val gmii_tx_er = Bits(OUTPUT,1)
	val gmii_tx_clk = Bits(OUTPUT,1)
	val gmii_rxd = Bits(INPUT,8)
	val gmii_rx_dv = Bits(INPUT,1)
	val gmii_rx_er = Bits(INPUT,1)
	val gmii_rx_clk = Bits(INPUT,1)
	val gmii_col = Bits(INPUT,1)
	val gmii_crs = Bits(INPUT,1)
	val mii_tx_clk = Bits(INPUT,1)
    }
  }
}

class EMAC() extends CoreDevice() {

  override val io = new CoreDeviceIO() with EMAC.Pins
  val wrapper = Module(new v6_emac_v2_3_wrapper)
  wrapper.io <> io

  val rx_axis_fifo_tready_Reg = Reg(init = Bits(0,1))
  rx_axis_fifo_tready_Reg := Bits(0,1)
  
  val tx_axis_fifo_tdata_Reg = Reg(init = Bits(0,8))
  val tx_axis_fifo_tvalid_Reg = Reg(init = Bits(0,1))
  val tx_axis_fifo_tlast_Reg = Reg(init = Bits(0,1))
  
  wrapper.io.rx_axis_fifo_tready := rx_axis_fifo_tready_Reg
  wrapper.io.tx_axis_fifo_tdata := tx_axis_fifo_tdata_Reg
  wrapper.io.tx_axis_fifo_tvalid := tx_axis_fifo_tvalid_Reg
  wrapper.io.tx_axis_fifo_tlast := tx_axis_fifo_tlast_Reg

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL

  // Default data
  val dataReg = Reg(init = Bits(0,32))
  dataReg := Bits(0,32)
  dataReg(31,31) := wrapper.io.rx_axis_fifo_tvalid
  dataReg(30,30) := wrapper.io.rx_axis_fifo_tlast
  dataReg(29,29) := wrapper.io.tx_axis_fifo_tready
  dataReg(7,0) := wrapper.io.rx_axis_fifo_tdata
  
  // Write to EMAC
  

  val sIdle :: sWait :: sRead :: Nil = Enum(UInt(), 3)
  val state = Reg(init = sIdle)
  
  when(io.ocp.M.Cmd === OcpCmd.WR) {
	respReg := OcpResp.DVA
	tx_axis_fifo_tdata_Reg := io.ocp.M.Data(7, 0)
	tx_axis_fifo_tvalid_Reg := io.ocp.M.Data(31)
	tx_axis_fifo_tlast_Reg := io.ocp.M.Data(30)
  }
  
  when(state === sIdle) {
	when(io.ocp.M.Cmd === OcpCmd.RD) {
	  when(io.ocp.M.Addr === UInt(0)) {
	    state := sWait
		rx_axis_fifo_tready_Reg := Bits(1,1)
      }
	  .otherwise {
	    respReg := OcpResp.DVA
	  }
	}
  }
  when(state === sWait) {
	state := sRead
  }
  when(state === sRead) {
	state := sIdle
	respReg := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataReg

}
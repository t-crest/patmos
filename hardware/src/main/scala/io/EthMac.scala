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
 * EthMac interface for Patmos
 *
 * Author: Luca Pezzarossa (lpez@dtu.dk)
 *
 */

package io

import Chisel._
import Node._
import ocp._

object EthMac extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
  }

  def create(params: Map[String, String]) : EthMac = {
    Module(new EthMac(extAddrWidth=extAddrWidth, dataWidth=dataWidth))
  }

  trait Pins {
    val ethMacPins = new Bundle() {
      // Tx
      val mtx_clk_pad_i = Bits(INPUT, width = 1)  // Transmit clock (from PHY)
      val mtxd_pad_o    = Bits(OUTPUT, width = 4) // Transmit nibble (to PHY)
      val mtxen_pad_o   = Bits(OUTPUT, width = 1) // Transmit enable (to PHY)
      val mtxerr_pad_o  = Bits(OUTPUT, width = 1) // Transmit error (to PHY)

      // Rx
      val mrx_clk_pad_i = Bits(INPUT, width = 1) // Receive clock (from PHY)
      val mrxd_pad_i    = Bits(INPUT, width = 4) // Receive nibble (from PHY)
      val mrxdv_pad_i   = Bits(INPUT, width = 1) // Receive data valid (from PHY)
      val mrxerr_pad_i  = Bits(INPUT, width = 1) // Receive data error (from PHY)

      // Common Tx and Rx
      val mcoll_pad_i   = Bits(INPUT, width = 1) // Collision (from PHY)
      val mcrs_pad_i    = Bits(INPUT, width = 1) // Carrier sense (from PHY)

      // MII Management interface
      val md_pad_i      = Bits(INPUT, width = 1)  // MII data input (from I/O cell)
      val mdc_pad_o     = Bits(OUTPUT, width = 1) // MII Management data clock (to PHY)
      val md_pad_o      = Bits(OUTPUT, width = 1) // MII data output (to I/O cell)
      val md_padoe_o    = Bits(OUTPUT, width = 1) // MII data output enable (to I/O cell)

      val int_o         = Bits(OUTPUT, width = 1) // Interrupt output
    }
  }

  trait Intrs {
    val ethMacIntrs = Vec.fill(1) { Bool(OUTPUT) }
  }
}

class EthMacBB(extAddrWidth : Int = 32, dataWidth : Int = 32) extends BlackBox {
  val io = new OcpCoreSlavePort(extAddrWidth, dataWidth) with EthMac.Pins
  // rename component
  setModuleName("eth_controller_top")

  // rename signals
  renameClock(clock, "clk")
  reset.setName("rst")

  io.M.Cmd.setName("MCmd")
  io.M.Addr.setName("MAddr")
  io.M.Data.setName("MData")
  io.M.ByteEn.setName("MByteEn")
  io.S.Resp.setName("SResp")
  io.S.Data.setName("SData")

  io.ethMacPins.mtx_clk_pad_i.setName("mtx_clk_pad_i")
  io.ethMacPins.mtxd_pad_o.setName("mtxd_pad_o")
  io.ethMacPins.mtxen_pad_o.setName("mtxen_pad_o")
  io.ethMacPins.mtxerr_pad_o.setName("mtxerr_pad_o")
  io.ethMacPins.mrx_clk_pad_i.setName("mrx_clk_pad_i")
  io.ethMacPins.mrxd_pad_i.setName("mrxd_pad_i")
  io.ethMacPins.mrxdv_pad_i.setName("mrxdv_pad_i")
  io.ethMacPins.mrxerr_pad_i.setName("mrxerr_pad_i")
  io.ethMacPins.mcoll_pad_i.setName("mcoll_pad_i")
  io.ethMacPins.mcrs_pad_i.setName("mcrs_pad_i")
  io.ethMacPins.md_pad_i.setName("md_pad_i")
  io.ethMacPins.mdc_pad_o.setName("mdc_pad_o")
  io.ethMacPins.md_pad_o.setName("md_pad_o")
  io.ethMacPins.md_padoe_o.setName("md_padoe_o")
  io.ethMacPins.int_o.setName("int_o")

  // set Verilog parameters
  setVerilogParameters("#(.BUFF_ADDR_WIDTH(16))")

  // keep some sigals for emulation
  debug(io.M.Cmd)
  debug(io.M.Addr)
  debug(io.M.Data)

  // registers to help emulation
  val respReg = Reg(Bits(width = 2))
  val dataReg = Reg(Bits(width = dataWidth))
  io.S.Resp := respReg
  io.S.Data := dataReg
}

class EthMac(extAddrWidth : Int = 32, dataWidth : Int = 32) extends CoreDevice() {
  override val io = new CoreDeviceIO() with EthMac.Pins with EthMac.Intrs
  val SyncReg = Reg(Bits(width = 1))
  val IntReg = Reg(Bits(width = 1))

  val bb = Module(new EthMacBB(extAddrWidth, dataWidth))
  bb.io.M <> io.ocp.M
  bb.io.S <> io.ocp.S
  bb.io.ethMacPins <> io.ethMacPins

  // Connection to pins
  SyncReg := bb.io.ethMacPins.int_o 
  //SyncReg := ~SyncReg
  IntReg := SyncReg

  // Generate interrupts on rising edges?
  io.ethMacIntrs(0) := IntReg(0) === Bits("b0") && SyncReg(0) === Bits("b1")
}

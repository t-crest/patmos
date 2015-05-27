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

import scala.math._
import Chisel._
import Node._
import ocp._
import patmos.Constants._

object AvalonMMBridge extends DeviceObject {
  var extAddrWidth = 32
  var dataWidth = 32
  var numIntrs = 0
  var bitsPerByte = 8
  var bytesPerWord = dataWidth/bitsPerByte

  def init(params : Map[String, String]) = {
    extAddrWidth = getPosIntParam(params, "extAddrWidth")
    dataWidth = getPosIntParam(params, "dataWidth")
    numIntrs = getPosIntParam(params, "numIntrs")
    bytesPerWord = dataWidth/bitsPerByte
  }

  def create(params: Map[String, String]) : AvalonMMBridge = {
    Module(new AvalonMMBridge(extAddrWidth=extAddrWidth, dataWidth=dataWidth, numIntrs=numIntrs))
  }

  trait Pins {
    val avalonMMBridgePins = new Bundle() {
      val avs_waitrequest = Bits(INPUT,1)
      val avs_readdata = UInt(INPUT,dataWidth)
      val avs_readdatavalid = Bits(INPUT,1)
      val avs_burstcount = Bits(OUTPUT,1)
      val avs_writedata = UInt(OUTPUT,dataWidth)
      val avs_address = UInt(OUTPUT,extAddrWidth)
      val avs_write = Bool(OUTPUT)
      val avs_read = Bool(OUTPUT)
      val avs_byteenable = Bits(OUTPUT,bytesPerWord)
      val avs_debugaccess = Bool(OUTPUT)
      val avs_intr = Bits(INPUT,numIntrs)
    }
  }

  trait Intrs {
    val avalonMMBridgeIntrs = Vec.fill(numIntrs) { Bool(OUTPUT) }
  }
}

class AvalonMMBridge(extAddrWidth : Int = 32,
                     dataWidth : Int = 32,
                     numIntrs : Int = 1) extends CoreDevice() {
  override val io = new CoreDeviceIO() with AvalonMMBridge.Pins with AvalonMMBridge.Intrs

  val coreBus = Module(new OcpCoreBus(ADDR_WIDTH,dataWidth))
  val ioBus = Module(new OcpIOBus(ADDR_WIDTH,dataWidth))

  io.ocp <> coreBus.io.slave

  val bridge = new OcpIOBridge(coreBus.io.master,ioBus.io.slave)
  

  val intrVecReg0 = Reg(Bits(width = numIntrs))
  val intrVecReg1 = Reg(Bits(width = numIntrs))

  //for( i <- 0 until numIntrs) {
  //  intrVecReg0(i) := io.avalonMMBridgePins.avs_intr(i)
  //}
  intrVecReg0 := io.avalonMMBridgePins.avs_intr
  intrVecReg1 := intrVecReg0

  // Generate interrupts on rising edges
  for (i <- 0 until numIntrs) {
    io.avalonMMBridgeIntrs(i) := (intrVecReg0(i) === Bits("b1")) && (intrVecReg1(i) === Bits("b0"))
  }

  val cmdType = Reg(init = OcpCmd.IDLE)
  
  //val respReg = Reg(init = OcpResp.NULL)
  //val dataReg = Reg(init = Bits(0, dataWidth))

  val ReadWriteActive = Bool(true)
  val ReadWriteInactive = Bool(false)
  // Default values in case of ILDE command
  //respReg := OcpResp.NULL
  //dataReg := Bits(0)
  io.avalonMMBridgePins.avs_write := ReadWriteInactive
  io.avalonMMBridgePins.avs_read := ReadWriteInactive

  ioBus.io.master.S.Resp := OcpResp.NULL
  ioBus.io.master.S.CmdAccept := Bits(0)
  ioBus.io.master.S.Data := Bits(0)

  // Constant connections
  io.avalonMMBridgePins.avs_burstcount := Bits("b1")
  //for( i <- 3 to 0) {
  //  io.avalonMMBridgePins.avs_byteenable(3-i) := ioBus.io.master.M.ByteEn(i) 
  //}
  io.avalonMMBridgePins.avs_byteenable := ioBus.io.master.M.ByteEn
  io.avalonMMBridgePins.avs_debugaccess := Bits("b0")
  // Connecting address and data signal straight through
  io.avalonMMBridgePins.avs_address := ioBus.io.master.M.Addr(extAddrWidth-1, 0)
  io.avalonMMBridgePins.avs_writedata := ioBus.io.master.M.Data
  ioBus.io.master.S.Data := io.avalonMMBridgePins.avs_readdata

  cmdType := cmdType
  
  
  when(io.avalonMMBridgePins.avs_waitrequest === Bits("b0")) {
    ioBus.io.master.S.CmdAccept := Bits("b1")
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.WR) {
    io.avalonMMBridgePins.avs_write := ReadWriteActive
    io.avalonMMBridgePins.avs_read := ReadWriteInactive
    when(io.avalonMMBridgePins.avs_waitrequest === Bits("b0")) {
      ioBus.io.master.S.Resp := OcpResp.DVA
    }
  }

  when(ioBus.io.master.M.Cmd === OcpCmd.RD) {
    io.avalonMMBridgePins.avs_write := ReadWriteInactive
    io.avalonMMBridgePins.avs_read := ReadWriteActive
    cmdType := OcpCmd.RD
  }

  when(io.avalonMMBridgePins.avs_readdatavalid === Bits("b1") && cmdType === OcpCmd.RD) {
    ioBus.io.master.S.Resp := OcpResp.DVA
    cmdType := OcpCmd.IDLE
  }

}

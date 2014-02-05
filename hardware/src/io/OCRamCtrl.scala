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
 * Onc-chip memory controller with OCP burst interface.
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import scala.math._

import Chisel._
import Node._

import ocp._

import patmos.MemBlock
import patmos.MemBlockIO

object OCRamCtrl extends DeviceObject {
  var addrWidth = 16

  def init(params: Map[String, String]) = {
    addrWidth = getPosIntParam(params, "addrWidth")
  }

  def create(params: Map[String, String]) : OCRamCtrl = {
    Module(new OCRamCtrl(addrWidth))
  }

  trait Pins {
  }
}

class OCRamCtrl(addrWidth : Int) extends BurstDevice(addrWidth) {

  override val io = new BurstDeviceIO(addrWidth) with OCRamCtrl.Pins

  val BYTE_WIDTH = 8
  val BYTES_PER_WORD = io.ocp.M.Data.width / BYTE_WIDTH

  val size = 1 << addrWidth

  val dataReg = Reg(next = io.ocp.M.Data)
  val dataByteEnReg = Reg(next = io.ocp.M.DataByteEn)

  val ramAddrWidth = addrWidth - log2Up(BYTES_PER_WORD)
  val addrReg = Reg(init = Bits(0, width = ramAddrWidth - log2Up(io.ocp.burstLength)))

  val burstCntReg = Reg(init = UInt(0, width = log2Up(io.ocp.burstLength)))

  val idle :: read :: write :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = idle)

  burstCntReg := burstCntReg + UInt(1);
  // end transaction after a burst
  when (burstCntReg === UInt(io.ocp.burstLength-1)) {
    stateReg := idle
  }

  // start a new transaction
  when (io.ocp.M.Cmd === OcpCmd.RD || io.ocp.M.Cmd === OcpCmd.WR) {
    addrReg := io.ocp.M.Addr(addrWidth-1,
                             log2Up(io.ocp.burstLength) + log2Up(BYTES_PER_WORD))
    burstCntReg := Bits(0)
  }
  when (io.ocp.M.Cmd === OcpCmd.RD) {
    stateReg := read
  }
  when (io.ocp.M.Cmd === OcpCmd.WR) {
    stateReg := write
  }

  val addr = addrReg ## burstCntReg
  val wrEn = stateReg === write

  // generate byte memories
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  // store
  val stmsk = Mux(wrEn, dataByteEnReg, Bits("b0000"))
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) <= (stmsk(i), addr, dataReg(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
  }

  // load
  io.ocp.S.Data := mem.map(_(addr)).reduceLeft((x,y) => y ## x)

  // respond
  io.ocp.S.Resp := OcpResp.NULL
  when (stateReg === read ||
        (stateReg === write && burstCntReg === UInt(io.ocp.burstLength-1))) {
    io.ocp.S.Resp := OcpResp.DVA
  }

  // always accept
  io.ocp.S.CmdAccept := Bits(1)
  io.ocp.S.DataAccept := Bits(1)
}

/*
 Used to instantiate a single OCRAM control component
 */
object OCRamMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val addrWidth = args(0).toInt
    chiselMain(chiselArgs, () => Module(new OCRamCtrl(addrWidth)))
  }
}

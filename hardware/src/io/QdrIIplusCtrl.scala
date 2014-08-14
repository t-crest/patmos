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
 * Memory controller for QDR-II+ memory with OCP burst interface.
 * The controller assumes burst-of-four mode for QDR-II+ interface.
 *
 * TODO: Could be _much_ faster with some pipelining
 * 
 * Author: Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *         Martin Schoeberl (martin@jopdesign.com)
 *         Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package io

import scala.math._

import Chisel._
import Node._

import ocp._

import patmos.Constants._

object QdrIIplusCtrl extends DeviceObject {
  var ramAddrWidth = 19
  var ramDataWidth = 16
  var ocpAddrWidth = 22

  def init(params: Map[String, String]) = {
    ramAddrWidth = getPosIntParam(params, "ramAddrWidth")
    ramDataWidth = getPosIntParam(params, "ramDataWidth")
    ocpAddrWidth = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : QdrIIplusCtrl = {
    Module(new QdrIIplusCtrl(ocpAddrWidth, 
                             ocpBurstLen = BURST_LENGTH,
                             ramAddrWidth = ramAddrWidth,
                             ramDataWidth = ramDataWidth))
  }

  trait Pins {
    val qdrIIplusCtrlPins = new Bundle {
      val addr  = Bits(OUTPUT, width = ramAddrWidth)

      val nrps  = Bits(OUTPUT, width = 1)
      val nwps  = Bits(OUTPUT, width = 1)
      val nbws  = Vec.fill(2) { Bits(OUTPUT, width = ramDataWidth / 8) }

      val din   = Vec.fill(2) { Bits(INPUT, width = ramDataWidth) }
      val dout  = Vec.fill(2) { Bits(OUTPUT, width = ramDataWidth) }

      val odt   = Bits(OUTPUT, width = 1)
      val ndoff = Bits(OUTPUT, width = 1)
      val qvld  = Bits(INPUT, width = 1)
    }
  }
}

class QdrIIplusCtrl(ocpAddrWidth   : Int,
                    ocpBurstLen    : Int = BURST_LENGTH,
                    ramAddrWidth   : Int = 19,
                    ramDataWidth   : Int = 16,
                    readWaitCycles : Int = 3) extends BurstDevice(ocpAddrWidth) {

  override val io = new BurstDeviceIO(ocpAddrWidth) with QdrIIplusCtrl.Pins

  // Checks for input parameters
  assert(Bool(DATA_WIDTH % ramDataWidth == 0),"DATA_WIDTH is not a multiple of ramDataWidth ")
  assert(Bool(ramAddrWidth <= ocpAddrWidth),"ocpAddrWidth cannot access the full RAM")
  assert(Bool(isPow2(ramDataWidth/8)),"number of bytes per transaction to RAM is not a power of 2")

  // Helper functions
  // log2up is a standard function but log2up(1)=1 and it should be 0
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  // Constants
  val TRANSPERWORD = DATA_WIDTH/ramDataWidth
  val TRANSPERCMD  = TRANSPERWORD*ocpBurstLen
  val TRANSPERSEL  = 4 // burst-of-four mode
  val BYTESPERTRAN = ramDataWidth / 8
  val BYTESPERSEL  = BYTESPERTRAN * TRANSPERSEL

  // State type and variable
  val sReady :: sReadSel :: sReadWait :: sReadExe :: sReadRet :: sWriteRec  :: sWriteSel :: sWriteExe:: sWriteRet :: Nil = Enum(UInt(), 9)
  val stateReg = Reg(init = sReady)

  // Internal Registers
  val mAddrReg         = Reg(init = Bits(0, width = ramAddrWidth))
  val rdBufferReg      = Vec.fill(TRANSPERCMD){ Reg(Bits(width = ramDataWidth)) }
  val wrBufferReg      = Vec.fill(TRANSPERCMD){ Reg(new Trans(BYTESPERTRAN, ramDataWidth)) }
  val transCountReg    = Reg(init = UInt(0, width = log2upNew(TRANSPERCMD)))
  val subTransCountReg = Reg(init = UInt(0, width = log2upNew(TRANSPERSEL)))
  val wordCountReg     = Reg(init = UInt(0, width = log2upNew(ocpBurstLen)))
  val waitCountReg     = Reg(init = UInt(0, width = log2upNew(readWaitCycles)))

  // Output Registers
  val addrReg = Reg(init = Bits(0, width = ramAddrWidth))
  val nrpsReg = Reg(init = Bits(1, width = 1))
  val nwpsReg = Reg(init = Bits(1, width = 1))
  val nbwsReg = Vec.fill(2) { Reg(init = Bits(-1, width = BYTESPERTRAN)) }
  val doutReg = Vec.fill(2) { Reg(init = Bits(0, width = ramDataWidth)) }

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.CmdAccept := Bits(0)
  io.ocp.S.DataAccept := Bits(0)
  val data = for(i <- 0 until TRANSPERWORD)
             yield rdBufferReg(wordCountReg ## UInt(i))
  io.ocp.S.Data := data.reduceLeft((x,y) => y ## x)

  // Default values for output ports
  addrReg := mAddrReg
  doutReg(0) := wrBufferReg(0).data
  doutReg(1) := wrBufferReg(1).data
  nrpsReg := Bits(1)
  nwpsReg := Bits(1)
  nbwsReg(0) := ~wrBufferReg(0).byteEna
  nbwsReg(1) := ~wrBufferReg(1).byteEna

  when(stateReg === sReady) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(i).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(i).data    := io.ocp.M.Data((i+1)*ramDataWidth-1,i*ramDataWidth)
    }
    when(io.ocp.M.Cmd != OcpCmd.IDLE) {
      mAddrReg := io.ocp.M.Addr(ramAddrWidth + log2upNew(BYTESPERSEL) - 1,
                                log2upNew(BYTESPERSEL))
      io.ocp.S.CmdAccept := Bits(1)
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        stateReg := sReadSel
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        io.ocp.S.DataAccept := Bits(1)
        wordCountReg := UInt(1) // The first ocp data word is already in wrBufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadSel) {
    nrpsReg := Bits(0)
    if (readWaitCycles == 0) {
      stateReg := sReadExe
    } else {
      stateReg := sReadWait
    }
  }
  when(stateReg === sReadWait) {
    waitCountReg := waitCountReg + UInt(1)
    when(waitCountReg === UInt(readWaitCycles-1)) {
      stateReg := sReadExe
      waitCountReg := UInt(0)
    }
  }
  when(stateReg === sReadExe) {
    transCountReg := transCountReg + UInt(1)
    subTransCountReg := subTransCountReg + UInt(1)
    rdBufferReg(transCountReg ## UInt(0)) := io.qdrIIplusCtrlPins.din(0)
    rdBufferReg(transCountReg ## UInt(1)) := io.qdrIIplusCtrlPins.din(1)
    when(subTransCountReg === UInt((TRANSPERSEL+1)/2-1)) {
      stateReg := sReadSel
      mAddrReg := mAddrReg + UInt(1)
      subTransCountReg := UInt(0)
    }
    when(transCountReg === UInt((TRANSPERCMD+1)/2-1)) {
      stateReg := sReadRet
      transCountReg := UInt(0)
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    wordCountReg := wordCountReg + UInt(1)
    when(wordCountReg === UInt(ocpBurstLen-1)) {
      stateReg := sReady
      wordCountReg := UInt(0)
    }
  }
  when(stateReg === sWriteRec) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(wordCountReg ## UInt(i)).byteEna :=
        io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(wordCountReg ## UInt(i)).data :=
        io.ocp.M.Data((i+1)*ramDataWidth-1,i*ramDataWidth)
    }
    when(io.ocp.M.DataValid === Bits(1)){
      io.ocp.S.DataAccept := Bits(1)
      wordCountReg := wordCountReg + UInt(1)
      when(wordCountReg === UInt(ocpBurstLen-1)){
        stateReg := sWriteExe
        wordCountReg := UInt(0)
        nwpsReg := Bits(0)
      }
    }
  }
  when(stateReg === sWriteSel) {
    stateReg := sWriteExe
    nwpsReg := Bits(0)
  }
  when(stateReg === sWriteExe) {
    doutReg(0) := wrBufferReg(transCountReg ## UInt(0)).data
    doutReg(1) := wrBufferReg(transCountReg ## UInt(1)).data
    nbwsReg(0) := ~wrBufferReg(transCountReg ## UInt(0)).byteEna
    nbwsReg(1) := ~wrBufferReg(transCountReg ## UInt(1)).byteEna
    transCountReg := transCountReg + UInt(1)
    subTransCountReg := subTransCountReg + UInt(1)
    when(subTransCountReg === UInt((TRANSPERSEL+1)/2-1)) {
      stateReg := sWriteSel
      mAddrReg := mAddrReg + UInt(1)
      subTransCountReg := UInt(0)
    }
    when(transCountReg === UInt((TRANSPERCMD+1)/2-1)) {
      io.ocp.S.Resp := OcpResp.DVA
      stateReg := sReady
      transCountReg := UInt(0)
    }
  }

  // Assign register values to pins
  io.qdrIIplusCtrlPins.addr := addrReg
  io.qdrIIplusCtrlPins.dout := doutReg
  io.qdrIIplusCtrlPins.nrps := nrpsReg
  io.qdrIIplusCtrlPins.nwps := nwpsReg
  io.qdrIIplusCtrlPins.nbws := nbwsReg

  // Hard-wired pins
  io.qdrIIplusCtrlPins.odt := Bits(1)
  io.qdrIIplusCtrlPins.ndoff := Bits(1)

  class Trans(bytesEnaWidth: Int, dataWidth: Int) extends Bundle {
    val byteEna = Bits(width = bytesEnaWidth)
    val data = Bits(width = dataWidth)

    // This does not really clone, but Data.clone doesn't either
    override def clone() = {
      val res = new Trans(bytesEnaWidth, dataWidth)
      res.asInstanceOf[this.type]
    }
  }
}

/*
 Used to instantiate a single controller component
 */
object QdrIIplusCtrlMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val ocpAddrWidth = args(0).toInt
    chiselMain(chiselArgs, () => Module(new QdrIIplusCtrl(ocpAddrWidth)))
  }
}

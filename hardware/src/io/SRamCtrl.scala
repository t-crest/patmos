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
 * SRAM controller with OCP burst interface.
 *
 * Is running on DE1-115, MS shall test it on the BeMicro
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

object SRamCtrl extends DeviceObject {
  var sramAddrWidth = 20
  var sramDataWidth = 16
  var ocpAddrWidth = 21

  def init(params: Map[String, String]) = {
    sramAddrWidth = getPosIntParam(params, "sramAddrWidth")
    sramDataWidth = getPosIntParam(params, "sramDataWidth")
    ocpAddrWidth = getPosIntParam(params, "ocpAddrWidth")
  }

  def create(params: Map[String, String]) : SRamCtrl = {
    Module(new SRamCtrl(ocpAddrWidth,ocpBurstLen=BURST_LENGTH,sramAddrWidth=sramAddrWidth,sramDataWidth=sramDataWidth))
  }

  trait Pins {
    val sRamCtrlPins = new Bundle {
      val ramOut = new Bundle {
        val addr = Bits(OUTPUT, width = sramAddrWidth)
        val doutEna = Bits(OUTPUT, width = 1)
        val dout = Bits(OUTPUT, width = sramDataWidth)
        val nce = Bits(OUTPUT, width = 1)
        val noe = Bits(OUTPUT, width = 1)
        val nwe = Bits(OUTPUT, width = 1)
        val nlb = Bits(OUTPUT, width = 1)
        val nub = Bits(OUTPUT, width = 1)
      }
      val ramIn = new Bundle {
        val din = Bits(INPUT, width = sramDataWidth)
      }
    }
  }
}

class SRamCtrl( ocpAddrWidth    : Int,
                ocpBurstLen     : Int=4,
                sramAddrWidth   : Int=20,
                sramDataWidth   : Int=16,
                singleCycleRead : Boolean=false,
                writeWaitCycles : Int=1) extends BurstDevice(ocpAddrWidth) {

  override val io = new BurstDeviceIO(ocpAddrWidth) with SRamCtrl.Pins

  // Checks for input parameters
  assert(Bool(DATA_WIDTH % sramDataWidth == 0),"DATA_WIDTH is not a multiple of sramDataWidth")
  assert(Bool(sramAddrWidth <= ocpAddrWidth),"ocpAddrWidth cannot access the full sram")
  assert(Bool(isPow2(sramDataWidth/8)),"number of bytes per transaction to sram is not a power of 2")

  // Helper functions
  // log2up is a standard function but log2up(1)=1 and it should be 0
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  // Constants
  val TRANSPERWORD = DATA_WIDTH/sramDataWidth
  val TRANSPERCMD = TRANSPERWORD*ocpBurstLen
  val BYTESPERTRAN = sramDataWidth/8

  // State type and variable
  val sReady :: sReadExe :: sReadExe2 :: sReadRet :: sWriteRec :: sWriteExe :: sWriteExe2 :: sWriteRet :: Nil = Enum(UInt(), 8)
  val stateReg = Reg(init = sReady)

  // Internal Registers
  val mAddrReg = Reg(Bits(width = sramAddrWidth))
  val rdBufferReg = Vec.fill(TRANSPERCMD){Reg(Bits(width=sramDataWidth))}
  val wrBufferReg = Vec.fill(TRANSPERCMD){Reg(new Trans(BYTESPERTRAN,sramDataWidth))}
  val transCountReg = Reg(init = UInt(0,width=log2upNew(TRANSPERCMD)))
  val wordCountReg = Reg(init = UInt(0,width=log2upNew(ocpBurstLen)))
  val waitCountReg = Reg(init = UInt(0,width=log2upNew(writeWaitCycles+1)))
  // Output Registers
  val addrReg = Reg(Bits(width = sramAddrWidth))
  val doutEnaReg = Reg(Bits())
  val doutReg = Reg(Bits(width = DATA_WIDTH))
  val nceReg = Reg(Bits())
  val noeReg = Reg(Bits())
  val nweReg = Reg(Bits())
  val nlbReg = Reg(Bits())
  val nubReg = Reg(Bits())

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.CmdAccept := Bits(1)
  io.ocp.S.DataAccept := Bits(1)
  val data = for(i <- 0 until TRANSPERWORD)
             yield rdBufferReg(wordCountReg ## UInt(i))
  io.ocp.S.Data := data.reduceLeft((x,y) => y ## x)

  // Default values for sRamCtrlPins.ramOut port
  addrReg := mAddrReg
  doutEnaReg := Bits(0)
  doutReg := wrBufferReg(0).data
  nceReg := Bits(0)
  noeReg := Bits(1)
  nweReg := Bits(1)
  nlbReg := Bits(1)
  nubReg := Bits(1)

  waitCountReg := UInt(0)

  when(stateReg === sReady) {
    for( i <- 0 until TRANSPERWORD) {
      wrBufferReg(i).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(i).data := io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
    }
    when(io.ocp.M.Cmd =/= OcpCmd.IDLE) {
      mAddrReg := io.ocp.M.Addr(sramAddrWidth+log2upNew(BYTESPERTRAN) - 1,
                                log2upNew(BYTESPERTRAN))
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        stateReg := sReadExe
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        wordCountReg := UInt(1) // The first ocp data word is already in wrBufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadExe) {
    noeReg := Bits(0)
    nceReg := Bits(0)
    nubReg := Bits(0)
    nlbReg := Bits(0)
    if (singleCycleRead){
      addrReg := mAddrReg + UInt(1)
      mAddrReg := mAddrReg + UInt(1)
      for (i <- 0 until TRANSPERCMD-1) { rdBufferReg(i) := rdBufferReg(i+1) }
      rdBufferReg(TRANSPERCMD-1) := io.sRamCtrlPins.ramIn.din
      transCountReg := transCountReg + UInt(1)
      stateReg := sReadExe
      when(transCountReg === UInt(TRANSPERCMD-1)){
        stateReg := sReadRet
        transCountReg := UInt(0)
      }
    } else {
      stateReg := sReadExe2
    }
  }
  when(stateReg === sReadExe2) {
    noeReg := Bits(0)
    nceReg := Bits(0)
    nubReg := Bits(0)
    nlbReg := Bits(0)
    addrReg := mAddrReg + UInt(1)
    mAddrReg := mAddrReg + UInt(1)
    for (i <- 0 until TRANSPERCMD-1) { rdBufferReg(i) := rdBufferReg(i+1) }
    rdBufferReg(TRANSPERCMD-1) := io.sRamCtrlPins.ramIn.din
    transCountReg := transCountReg + UInt(1)
    stateReg := sReadExe
    when(transCountReg === UInt(TRANSPERCMD-1)){
      stateReg := sReadRet
      transCountReg := UInt(0)
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    wordCountReg := wordCountReg + UInt(1)
    when(wordCountReg === UInt(ocpBurstLen-1)){
      stateReg := sReady
      wordCountReg := UInt(0)
    }
  }
  when(stateReg === sWriteRec) {
    for(i <- 0 until TRANSPERWORD) {
      wrBufferReg(wordCountReg ## UInt(i)).byteEna :=
        io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
      wrBufferReg(wordCountReg ## UInt(i)).data := 
        io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
    }
    doutReg := wrBufferReg(0).data
    doutEnaReg := Bits(1)
    when(io.ocp.M.DataValid === Bits(1)){
      wordCountReg := wordCountReg + UInt(1)
      when(wordCountReg === UInt(ocpBurstLen-1)){
        stateReg := sWriteExe
        wordCountReg := UInt(0)
        waitCountReg := UInt(1)
      }
    } otherwise {
      stateReg := sWriteRec
    }
    when(wordCountReg === UInt(ocpBurstLen-1)){
      nceReg := Bits(0)
      nweReg := Bits(0)
      nubReg := !wrBufferReg(0).byteEna(1)
      nlbReg := !wrBufferReg(0).byteEna(0)
    }
  }
  when(stateReg === sWriteExe) {
    nceReg := Bits(0)
    nweReg := Bits(0)
    doutReg := wrBufferReg(transCountReg).data
    doutEnaReg := Bits(1)
    when(waitCountReg < UInt(writeWaitCycles)){
      waitCountReg := waitCountReg + UInt(1)
      nubReg := !wrBufferReg(transCountReg).byteEna(1)
      nlbReg := !wrBufferReg(transCountReg).byteEna(0)
      stateReg := sWriteExe
    } otherwise {
      waitCountReg := UInt(0)
      nubReg := Bits(1)
      nlbReg := Bits(1)
      stateReg := sWriteExe2
    }
  }
  when(stateReg === sWriteExe2) {
    when(transCountReg < UInt(TRANSPERCMD-1)){
      nceReg := Bits(0)
      nweReg := Bits(0)
      nubReg := !wrBufferReg(transCountReg+UInt(1)).byteEna(1)
      nlbReg := !wrBufferReg(transCountReg+UInt(1)).byteEna(0)
      doutReg := wrBufferReg(transCountReg+UInt(1)).data
      doutEnaReg := Bits(1)
      addrReg := mAddrReg + UInt(1)
      mAddrReg := mAddrReg + UInt(1)
      transCountReg := transCountReg + UInt(1)
      waitCountReg := UInt(1)
      stateReg := sWriteExe
    }
    when(transCountReg === UInt(TRANSPERCMD-1)){
      stateReg := sWriteRet
      transCountReg := UInt(0)
      waitCountReg := UInt(0)
    }
  }
  when(stateReg === sWriteRet) {
    io.ocp.S.Resp := OcpResp.DVA
    stateReg := sReady
  }

  io.sRamCtrlPins.ramOut.addr := addrReg
  io.sRamCtrlPins.ramOut.doutEna := doutEnaReg
  io.sRamCtrlPins.ramOut.dout := doutReg
  io.sRamCtrlPins.ramOut.nce := nceReg
  io.sRamCtrlPins.ramOut.noe := noeReg
  io.sRamCtrlPins.ramOut.nwe := nweReg
  io.sRamCtrlPins.ramOut.nlb := nlbReg
  io.sRamCtrlPins.ramOut.nub := nubReg


  class Trans(bytesEnaWidth: Int, dataWidth: Int) extends Bundle {
    val byteEna = Bits(width=bytesEnaWidth)
    val data = Bits(width=dataWidth)

    // This does not really clone, but Data.clone doesn't either
    override def clone() = {
      val res = new Trans(bytesEnaWidth, dataWidth)
      res.asInstanceOf[this.type]
    }
  }
}

/*
 Used to instantiate a single SRAM control component
 */
object SRamMain {
  def main(args: Array[String]): Unit = {
    val chiselArgs = args.slice(1,args.length)
    val ocpAddrWidth = args(0).toInt
    chiselMain(chiselArgs, () => Module(new SRamCtrl(ocpAddrWidth)))
  }
}

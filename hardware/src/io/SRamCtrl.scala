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
 *
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
    Module(new SRamCtrl(ocpAddrWidth,sramAddrWidth=sramAddrWidth,sramDataWidth=sramDataWidth))
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
  assert(Bool(DATA_WIDTH % sramDataWidth != 0),"DATA_WIDTH is not a multiple of sramDataWidth")
  assert(Bool(sramAddrWidth > ocpAddrWidth),"ocpAddrWidth cannot access the full sram")
  assert(Bool(!isPow2(sramDataWidth/8)),"number of bytes per transaction to sram is not a power of 2")

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

  //
  // MS: registers shall end with Reg, e.g. addrReg -- see coding.txt
  val mAddrReg = Reg(init = Bits(0,width = sramAddrWidth))
  val bufferReg = Vec.fill(TRANSPERCMD){Reg(init = new Trans(BYTESPERTRAN,sramDataWidth))}
  val transCountReg = Reg(init = UInt(0,width=log2upNew(TRANSPERCMD)))
  val waitCountReg = Reg(init = UInt(0,width=log2Up(writeWaitCycles+1)))
  // Output Registers
  val addrReg = Reg(init = Bits(0, width = sramAddrWidth))
  val doutEnaReg = Reg(init = Bits(0))
  val doutReg = Reg(init = Bits(0, width = DATA_WIDTH))
  val nceReg = Reg(init = Bits(1))
  val noeReg = Reg(init = Bits(1))
  val nweReg = Reg(init = Bits(1))
  val nlbReg = Reg(init = Bits(1))
  val nubReg = Reg(init = Bits(1))

  val addrNext = Bits(width = sramAddrWidth)
  val doutEnaNext = Bits(width = 1)
  val doutNext = Bits(width = DATA_WIDTH)
  val nceNext = Bits(width = 1)
  val noeNext = Bits(width = 1)
  val nweNext = Bits(width = 1)
  val nlbNext = Bits(width = 1)
  val nubNext = Bits(width = 1)

  // Default values for ocp io.ocp.S port
  io.ocp.S.Resp := OcpResp.NULL
  io.ocp.S.Data := Bits(0, width = DATA_WIDTH)
  io.ocp.S.CmdAccept := Bits(0)
  io.ocp.S.DataAccept := Bits(0)

  // Default values for sRamCtrlPins.ramOut port
  //addr := Bits(0, width = sramAddrWidth)
  doutEnaNext := Bits(0)
  doutNext := Bits(0, width = DATA_WIDTH)
  nceNext := Bits(0)
  noeNext := Bits(1)
  nweNext := Bits(1)
  nlbNext := Bits(1)
  nubNext := Bits(1)

  addrNext := mAddrReg
  mAddrReg := mAddrReg
  waitCountReg := UInt(0)

  when(stateReg === sReady) {
    for( i <- 0 to TRANSPERCMD-1) {
      bufferReg(i).byteEna := Bits(0)
      bufferReg(i).data := Bits(0)
    }
    when(io.ocp.M.Cmd != OcpCmd.IDLE) {
      mAddrReg := io.ocp.M.Addr(sramAddrWidth+log2upNew(BYTESPERTRAN)-1,log2upNew(BYTESPERTRAN))
      io.ocp.S.CmdAccept := Bits(1)
      transCountReg := UInt(0)
      when(io.ocp.M.Cmd === OcpCmd.RD) {
        noeNext := Bits(0)
        nceNext := Bits(0)
        nubNext := Bits(0)
        nlbNext := Bits(0)
        stateReg := sReadExe
      }
      when(io.ocp.M.Cmd === OcpCmd.WR) {
        io.ocp.S.DataAccept := Bits(1)
        for(i <- 0 to TRANSPERWORD-1) {
          bufferReg(i).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
          bufferReg(i).data := io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
        }
        transCountReg := UInt(1) // Because the first ocp data word is stored in the bufferReg
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadExe) {
    noeNext := Bits(0)
    nceNext := Bits(0)
    nubNext := Bits(0)
    nlbNext := Bits(0)
    if (singleCycleRead){
      addrNext := mAddrReg + UInt(1)
      mAddrReg := mAddrReg + UInt(1)
      bufferReg(transCountReg).data := io.sRamCtrlPins.ramIn.din
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
    noeNext := Bits(0)
    nceNext := Bits(0)
    nubNext := Bits(0)
    nlbNext := Bits(0)
    addrNext := mAddrReg + UInt(1)
    mAddrReg := mAddrReg + UInt(1)
    bufferReg(transCountReg).data := io.sRamCtrlPins.ramIn.din
    transCountReg := transCountReg + UInt(1)
    stateReg := sReadExe
    when(transCountReg === UInt(TRANSPERCMD-1)){
      stateReg := sReadRet
      transCountReg := UInt(0)
    }
  }
  when(stateReg === sReadRet) {
    io.ocp.S.Resp := OcpResp.DVA
    io.ocp.S.Data := Cat(bufferReg(transCountReg+UInt(1)).data,bufferReg(transCountReg).data)
    transCountReg := transCountReg + UInt(2)
    when(transCountReg === UInt(TRANSPERCMD-2)){
      stateReg := sReady
      transCountReg := UInt(0)
    }
  }
  when(stateReg === sWriteRec) {
    when(io.ocp.M.DataValid === Bits(1)){
      io.ocp.S.DataAccept := Bits(1)
      for(i <- 0 to TRANSPERWORD-1) {
        bufferReg(UInt(i)+transCountReg*UInt(TRANSPERWORD)).byteEna := io.ocp.M.DataByteEn((i+1)*BYTESPERTRAN-1,i*BYTESPERTRAN)
        bufferReg(UInt(i)+transCountReg*UInt(TRANSPERWORD)).data := io.ocp.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
      }
      transCountReg := transCountReg + UInt(1)
      when(transCountReg === UInt(ocpBurstLen-1)){
        stateReg := sWriteExe
        transCountReg := UInt(0)
        nceNext := Bits(0)
        nweNext := Bits(0)
        nubNext := !bufferReg(0).byteEna(1)
        nlbNext := !bufferReg(0).byteEna(0)
        doutNext := bufferReg(0).data
        doutEnaNext := Bits(1)
        waitCountReg := UInt(1)
      }
    } otherwise {
      stateReg := sWriteRec
    }
  }
  when(stateReg === sWriteExe) {
    nceNext := Bits(0)
    nweNext := Bits(0)
    doutNext := bufferReg(transCountReg).data
    doutEnaNext := Bits(1)
    when(waitCountReg < UInt(writeWaitCycles)){
      waitCountReg := waitCountReg + UInt(1)
      nubNext := !bufferReg(transCountReg).byteEna(1)
      nlbNext := !bufferReg(transCountReg).byteEna(0)
      stateReg := sWriteExe
    } otherwise {
      waitCountReg := UInt(0)
      nubNext := Bits(1)
      nlbNext := Bits(1)
      stateReg := sWriteExe2
    }
  }
  when(stateReg === sWriteExe2) {
    when(transCountReg < UInt(TRANSPERCMD-1)){
      nceNext := Bits(0)
      nweNext := Bits(0)
      doutNext := bufferReg(transCountReg+UInt(1)).data
      nubNext := !bufferReg(transCountReg+UInt(1)).byteEna(1)
      nlbNext := !bufferReg(transCountReg+UInt(1)).byteEna(0)
      doutEnaNext := Bits(1)
      addrNext := mAddrReg + UInt(1)
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

  // MS: try to use 'plain' registers for IO pins
  // plain means no condition on them as most (or all?)
  // FPGAs have special IO registers that are not near
  // a LUT. To read your register state within the FPGA
  // you can still have your internal 'additional' register:
  // val addrReg = Reg(init = Bits(0, width = sramAddrWidth))
  // val addrOutReg = Reg(init = Bits(0, width = sramAddrWidth))
  // val addrNext
  // do your combinational logic with addrNext and addrReg
  // e.g.: addrNext := addrReg + UInt(1)
  // keep the regs simple with an always assignment as:
  // addrReg := addrNext
  // addrOutReg := addrNext
  // I see that this is almost what you do with mAddr is what I call addrReg
  //
  // Naming could also be nce and nceReg
  // Probably an error: we must go to one for the write and you should keep
  // address and data stable

  addrReg := addrNext
  doutEnaReg := doutEnaNext
  doutReg := doutNext
  nceReg := nceNext
  noeReg := noeNext
  nweReg := nweNext
  nlbReg := nlbNext
  nubReg := nubNext

  io.sRamCtrlPins.ramOut.addr := addrReg
  io.sRamCtrlPins.ramOut.doutEna := doutEnaReg
  io.sRamCtrlPins.ramOut.dout := doutReg
  io.sRamCtrlPins.ramOut.nce := nceReg
  io.sRamCtrlPins.ramOut.noe := noeReg
  io.sRamCtrlPins.ramOut.nwe := nweReg
  io.sRamCtrlPins.ramOut.nlb := nlbReg
  io.sRamCtrlPins.ramOut.nub := nubReg

}

class Trans(bytesEnaWidth: Int, dataWidth: Int) extends Bundle {
  val byteEna = Bits(width=bytesEnaWidth)
  val data = Bits(width=dataWidth)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new Trans(bytesEnaWidth, dataWidth)
    res.asInstanceOf[this.type]
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

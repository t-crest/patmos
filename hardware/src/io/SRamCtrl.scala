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
 * SRAM controller.
 *
 * First step get a hard-coded version running on DE1-115 or BeMicro
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *         Rasmus Bo Soerensen (rasmus@rbscloud.dk)
 *
 */

package io

import scala.math._

import Chisel._
import Node._

import ocp._

class SRamCtrl(ocpAddrWidth     : Int=32,
                ocpDataWidth    : Int=32,
                ocpBurstLen     : Int=4,
                sramAddrWidth   : Int=19,
                sramDataWidth   : Int=16,
                singleCycleRead : Bool=Bool(false),
                singleCycleWrite: Bool=Bool(false)) extends Module {
  val io = new Bundle {
    val ocpPort = new OcpBurstSlavePort(ocpAddrWidth, ocpDataWidth, ocpBurstLen)
    val ramOut = new Bundle {
      val addr = Bits(OUTPUT, width = sramAddrWidth)
      val dout_ena = Bits(width = 1)
      val dout = Bits(width = sramDataWidth)
      val nce = Bits(width = 1)
      val noe = Bits(width = 1)
      val nwe = Bits(width = 1)
      val nlb = Bits(width = 1)
      val nub = Bits(width = 1)
    }
    val ramIn = new Bundle {
      val din = Bits(INPUT, width = sramDataWidth)
    }
  }

  // Checks for input parameters
  assert(Bool(ocpDataWidth % sramDataWidth != 0),"ocpDataWidth is not a multiple of sramDataWidth")
  assert(Bool(sramAddrWidth > ocpAddrWidth),"ocpAddrWidth cannot access the full sram")
  assert(Bool(!isPow2(sramDataWidth/8)),"number of bytes per transaction to sram is not a power of 2")

  // Helper functions
  // log2up is a standard function but log2up(1)=1 and it should be 0
  def log2upNew(in: Int) = ceil(log(in)/log(2)).toInt
  // Constants
  val TransPerWord = ocpDataWidth/sramDataWidth
  val TransPerCmd = UInt(TransPerWord*ocpBurstLen)
  val BytesPerTran = sramDataWidth/8

  // State type and variable
  val sReady :: sReadExe :: sReadExe2 :: sReadRet :: sWriteRec :: sWriteExe :: sWriteExe2 :: sWriteRet :: Nil = Enum(UInt(), 8)
  val stateReg = Reg(init = sReady)

  //
  val mAddr = Reg(init = Bits(0,width = sramAddrWidth))
  val buffer = Vec.fill(ocpBurstLen*TransPerWord){Reg(init = Bits(0, width = sramDataWidth+BytesPerTran))}
  val transCount = Reg(init = UInt(0))
  // Registers
  val addr = Reg(init = Bits(0, width = sramAddrWidth))
  val dout_ena = Reg(init = Bits(0))
  val dout = Reg(init = Bits(0, width = ocpDataWidth))
  val nce = Reg(init = Bits(1))
  val noe = Reg(init = Bits(1))
  val nwe = Reg(init = Bits(1))
  val nlb = Reg(init = Bits(1))
  val nub = Reg(init = Bits(1))

  // Default values for ocp io.ocpPort.S port
  io.ocpPort.S.Resp := OcpResp.NULL
  io.ocpPort.S.Data := Bits(0, width = ocpDataWidth)
  io.ocpPort.S.CmdAccept := Bits(0)
  io.ocpPort.S.DataAccept := Bits(0)

  // Default values for ramOut port
  addr := Bits(0, width = sramAddrWidth)
  dout_ena := Bits(0)
  dout := Bits(0, width = ocpDataWidth)
  nce := Bits(1)
  noe := Bits(1)
  nwe := Bits(1)
  nlb := Bits(1)
  nub := Bits(1)

  when(stateReg === sReady) {
    when(io.ocpPort.M.Cmd != OcpCmd.IDLE) {
      mAddr := io.ocpPort.M.Addr(sramAddrWidth+log2upNew(BytesPerTran)-1,log2upNew(BytesPerTran))
      io.ocpPort.S.CmdAccept := Bits(1)
      when(io.ocpPort.M.Cmd === OcpCmd.RD) {
        addr := io.ocpPort.M.Addr(sramAddrWidth+log2upNew(BytesPerTran)-1,log2upNew(BytesPerTran))
        noe := Bits(0)
        nce := Bits(0)
        stateReg := sReadExe
      }
      when(io.ocpPort.M.Cmd === OcpCmd.WR) {
        for(i <- 0 to TransPerWord-1) {
          val byteEna = io.ocpPort.M.DataByteEn((i+1)*BytesPerTran-1,i*BytesPerTran)
          val transWord = io.ocpPort.M.Data((i+1)*sramDataWidth-1,i*sramDataWidth)
          buffer(i) := Cat(byteEna,transWord)
        }
        stateReg := sWriteRec
      }
    }
  }
  when(stateReg === sReadExe) {
    noe := Bits(0)
    nce := Bits(0)
    when(singleCycleRead){
      assert(singleCycleRead,"Something is wrong")
      addr := mAddr + UInt(1)
      mAddr := mAddr + UInt(1)
      buffer(transCount) := io.ramIn.din
      transCount := transCount + UInt(1)
      stateReg := sReadExe
      when(transCount === TransPerCmd-UInt(1)){
        stateReg := sReadRet
      }
    } otherwise {
      assert(!singleCycleRead,"Something is wrong")
      stateReg := sReadExe2
    }
  }
  when(stateReg === sReadExe2) {
    assert(!singleCycleRead,"If singleCycleRead is true this state should not be reached.")
    addr := mAddr + UInt(1)
    mAddr := mAddr + UInt(1)
    buffer(transCount) := io.ramIn.din
    transCount := transCount + UInt(1)
    when(transCount === TransPerCmd-UInt(1)){
      stateReg := sReadRet
    } otherwise {
      stateReg := sReadExe
    }
  }
  when(stateReg === sReadRet) {
    stateReg := sReady
  }
  when(stateReg === sWriteRec) {
    stateReg := sReady
  }
  when(stateReg === sWriteExe) {
    stateReg := sReady
  }
  when(stateReg === sWriteExe2) {
    stateReg := sReady
  }
  when(stateReg === sWriteRet) {
    stateReg := sReady
  }

  io.ramOut.addr := addr
  io.ramOut.dout_ena := dout_ena
  io.ramOut.dout := dout
  io.ramOut.nce := nce
  io.ramOut.noe := noe
  io.ramOut.nwe := nwe
  io.ramOut.nlb := nlb
  io.ramOut.nub := nub

}

/*
 Used to instantiate a single SSRAM control component
 */
object SRamMain {
  def main(args: Array[String]): Unit = {
    chiselMain(args, () => Module(new SRamCtrl()))
  }
}

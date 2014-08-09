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
 * Arbiter for OCP burst slaves.
 * TDM arbitration. Each turn for a non-requesting master costs 16+4+2 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com) David Chong (davidchong99@gmail.com)
 *
 */

package ocp

import Chisel._
import Node._
import scala.math._

import scala.collection.mutable.HashMap

class NodeTdmArbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int, ctrlDelay: Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen) 
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
    val node = UInt(INPUT, 6)
  }
  debug(io.master)
  debug(io.slave)
  debug(io.node)

  val cntReg = Reg(init = UInt(0, log2Up(cnt*(burstLen + ctrlDelay + 1))))
  // slot length = burst size + 1 
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))
  val period = cnt * (burstLen + ctrlDelay + 1)
  val slotLen = burstLen + ctrlDelay + 1
  val numPipe = 3 
  
  val wrPipeDelay = burstLen + ctrlDelay + numPipe 
  val wrCntReg = Reg(init = UInt(0, log2Up(wrPipeDelay)))

  val rdPipeDelay = burstLen + ctrlDelay + numPipe 
  val rdCntReg = Reg(init = UInt(0, log2Up(rdPipeDelay)))
 
  val cpuSlot = Vec.fill(cnt){Reg(init = UInt(0, width=1))}

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = sIdle)

  debug(cntReg)
  debug(cpuSlot(0))
  debug(cpuSlot(1))
  debug(cpuSlot(2))
  debug(stateReg)
  debug(wrCntReg)
  debug(rdCntReg)

  cntReg := Mux(cntReg === UInt(period - 1), UInt(0), cntReg + UInt(1))
  
  def slotTable(i: Int): UInt = {
    (cntReg === UInt(i*slotLen)).toUInt
  }
  
  for (i <- 0 until cnt){
    cpuSlot(i) := slotTable(i)
  }
  
  // Initialize master data to zero when cpuSlot is not enabled 
  io.slave.M.Addr       := Bits(0)
  io.slave.M.Cmd        := Bits(0)
  io.slave.M.DataByteEn := Bits(0)
  io.slave.M.DataValid  := Bits(0)
  io.slave.M.Data       := Bits(0)

  // Initialize slave data to zero
  io.master.S.Data       := Bits(0)
  io.master.S.Resp       := OcpResp.NULL
  io.master.S.CmdAccept  := Bits(0)
  io.master.S.DataAccept := Bits(0)
  
  // FSM for TDM Arbiter 
  when (stateReg === sIdle) {
    when (cpuSlot(io.node) === UInt(1)) {
      val master = io.master.M
      //io.slave.M := master
      
      when (master.Cmd != OcpCmd.IDLE){
        when (master.Cmd === OcpCmd.RD) {
          io.slave.M := master
          stateReg := sRead
          io.master.S.CmdAccept := UInt(1)
          rdCntReg := UInt(0)
        }
        when (master.Cmd === OcpCmd.WR) {
          io.slave.M := master
          stateReg := sWrite
          io.master.S.CmdAccept := UInt(1)
          io.master.S.DataAccept := UInt(1)
          wrCntReg := UInt(0)
        }
      }
    }
  }

  when (stateReg === sWrite){
    io.slave.M := io.master.M
    io.master.S.DataAccept := UInt(1)
    wrCntReg := Mux(wrCntReg === UInt(wrPipeDelay), UInt(0), wrCntReg + UInt(1))
   
    // Sends ZEROs after the burst is done 
    when (wrCntReg >= UInt(burstLen-1)) {
      io.slave.M.Cmd  := Bits(0)
      io.slave.M.Addr := Bits(0)
      io.slave.M.Data := Bits(0)
      io.slave.M.DataValid := Bits(0)
      io.slave.M.DataByteEn := Bits(0)
    }

    // Turn off the DataValid after a burst of 4
    when (wrCntReg >= UInt(burstLen-1)){
      io.master.S.DataAccept := UInt(0)
    }
    
    // Forward Rsp/DVA back to node 
    when (wrCntReg === UInt(wrPipeDelay-1)) {
      io.master.S.Resp := io.slave.S.Resp
    }
    // Wait on DVA 
    when(io.master.S.Resp === OcpResp.DVA){
      stateReg := sIdle
    }
  }
     
  when (stateReg === sRead){
    io.slave.M := io.master.M
    rdCntReg := Mux(rdCntReg === UInt(rdPipeDelay + burstLen), UInt(0), rdCntReg + UInt(1))
    
    // Sends ZEROs after the burst is done 
    when (rdCntReg >= UInt(burstLen-1)) {
      io.slave.M.Cmd  := Bits(0)
      io.slave.M.Addr := Bits(0)
      io.slave.M.Data := Bits(0)
      io.slave.M.DataValid := Bits(0)
      io.slave.M.DataByteEn := Bits(0) 
    }
    
    // rdCntReg starts 1 clock cycle after the arrival of the 1st data
    when (rdCntReg >= UInt(ctrlDelay + numPipe)) {
      io.master.S.Data := io.slave.S.Data
      io.master.S.Resp := io.slave.S.Resp
    }
  
    when (io.master.S.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + UInt(1)
        when (burstCntReg === UInt(burstLen) - UInt(1)) {
          stateReg := sIdle
        }
     }
  }      
  
  debug(io.slave.M)
  
  //io.master.S := io.slave.S

}

/* Mux for all arbiters' outputs */
class MemMuxIntf(nr: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends Module {
  val io = new Bundle {
    val master = Vec.fill(nr){new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)}
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  }
    debug(io.master)
    debug(io.slave)
    
    // 1st stage pipeline registers for inputs 
    val mCmd_p1_Reg         = Vec.fill(nr){Reg(init=UInt(0, width=3))}
    val mAddr_p1_Reg        = Vec.fill(nr){Reg(init=UInt(0, width=addrWidth))}
    val mData_p1_Reg        = Vec.fill(nr){Reg(init=UInt(0, width=dataWidth))}
    val mDataByteEn_p1_Reg  = Vec.fill(nr){Reg(init=UInt(0, width=dataWidth/8))}
    val mDataValid_p1_Reg   = Vec.fill(nr){Reg(init=UInt(0, width=1))}

    // 2st stage pipeline registers for inputs 
    val mCmd_p2_Reg         = Reg(init=UInt(0, width=3))
    val mAddr_p2_Reg        = Reg(init=UInt(0, width=addrWidth))
    val mData_p2_Reg        = Reg(init=UInt(0, width=dataWidth))
    val mDataByteEn_p2_Reg  = Reg(init=UInt(0, width=dataWidth/8))
    val mDataValid_p2_Reg   = Reg(init=UInt(0, width=1))
    
    // Pipeline regiaters default to 0
    mCmd_p1_Reg         := Bits(0)
    mAddr_p1_Reg        := Bits(0)
    mData_p1_Reg        := Bits(0)
    mDataByteEn_p1_Reg  := Bits(0)
    mDataValid_p1_Reg   := Bits(0)

    // Wires for cascading OR gate
    //val mCmd_res = Vec.fill(nr-1){UInt(width=3)}
    //val mAddr_res = Vec.fill(nr-1){UInt(width=addrWidth)}
    //val mData_res = Vec.fill(nr-1){UInt(width=dataWidth)}
    //val mDataByteEn_res = Vec.fill(nr-1){UInt(width=dataWidth/8)}
    //val mDataValid_res = Vec.fill(nr-1){UInt(width=1)} 
    
    // 1st stage pipeline of the input
    for (i <- 0 until nr){
      mCmd_p1_Reg(i) := io.master(i).M.Cmd
    }
    
    for (i <- 0 until nr){
      mAddr_p1_Reg(i) := io.master(i).M.Addr
    }
    
    for (i <- 0 until nr){
      mData_p1_Reg(i) := io.master(i).M.Data
    }
    
    for (i <- 0 until nr){
      mDataByteEn_p1_Reg(i) := io.master(i).M.DataByteEn
    }
    
    for (i <- 0 until nr){
      mDataValid_p1_Reg(i) := io.master(i).M.DataValid
    }
     
    // OR gate of all inputs (2nd stage pipeline)
    //mCmd_res(0) := mCmd_p1_Reg(0) | mCmd_p1_Reg(1)
    //for (i <- 1 until nr-1){
    //  mCmd_res(i) := mCmd_res(i-1) | mCmd_p1_Reg(i+1) 
    //}
    //mCmd_p2_Reg := mCmd_res(nr-2)
    mCmd_p2_Reg := mCmd_p1_Reg.reduce(_|_)

    //mAddr_res(0) := mAddr_p1_Reg(0) | mAddr_p1_Reg(1)
    //for (i <- 1 until nr-1){
    //  mAddr_res(i) := mAddr_res(i-1) | mAddr_p1_Reg(i+1)
    //}
    //mAddr_p2_Reg := mAddr_res(nr-2)
    mAddr_p2_Reg := mAddr_p1_Reg.reduce(_|_)
    
    //mData_res(0) := mData_p1_Reg(0) | mData_p1_Reg(1)
    //for (i <- 1 until nr-1){
    //  mData_res(i) := mData_res(i-1) | mData_p1_Reg(i+1)
    //}
    //mData_p2_Reg := mData_res(nr-2)
    mData_p2_Reg := mData_p1_Reg.reduce(_|_)
    
    //mDataByteEn_res(0) := mDataByteEn_p1_Reg(0) | mDataByteEn_p1_Reg(1)
    //for (i <- 1 until nr-1){
    //  mDataByteEn_res(i) := mDataByteEn_res(i-1) | mDataByteEn_p1_Reg(i+1)
    //}
    //mDataByteEn_p2_Reg := mDataByteEn_res(nr-2)
    mDataByteEn_p2_Reg := mDataByteEn_p1_Reg.reduce(_|_)
   
    //mDataValid_res(0) := mDataValid_p1_Reg(0) | mDataValid_p1_Reg(1)
    //for (i <- 1 until nr-1){
    //  mDataValid_res(i) := mDataValid_res(i-1) | mDataValid_p1_Reg(i+1)
    //}
    //mDataValid_p2_Reg := mDataValid_res(nr-2)
    mDataValid_p2_Reg := mDataValid_p1_Reg.reduce(_|_)
    
    // Transfer data from input pipeline registers to output
    io.slave.M.Addr       := mAddr_p2_Reg
    io.slave.M.Cmd        := mCmd_p2_Reg
    io.slave.M.DataByteEn := mDataByteEn_p2_Reg
    io.slave.M.DataValid  := mDataValid_p2_Reg
    io.slave.M.Data       := mData_p2_Reg
   
    // 1st stage pipleline registers for output
    //val sCmdAccept_p1_Reg   = Reg(next=io.slave.S.CmdAccept)
    //val sDataAccept_p1_Reg  = Reg(next=io.slave.S.DataAccept)
    val sResp_p1_Reg        = Reg(next=io.slave.S.Resp)
    val sData_p1_Reg        = Reg(next=io.slave.S.Data)

    
    // Forward response to all arbiters  
    for (i <- 0 until nr) {
      io.master(i).S.Data := sData_p1_Reg
      io.master(i).S.Resp := sResp_p1_Reg 
    }
    
}

object NodeTdmArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(5, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)
    val ctrlDelay = args(4)

    chiselMain(chiselArgs, () => Module(new NodeTdmArbiter(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt, ctrlDelay.toInt)))
  }
}



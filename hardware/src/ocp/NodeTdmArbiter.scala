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
 * Pseudo round robin arbitration. Each turn for a non-requesting master costs 1 clock cycle.
 *
 * Author: Martin Schoeberl (martin@jopdesign.com)
 *
 */

package ocp

import Chisel._
import Node._
import scala.math._

import scala.collection.mutable.HashMap

class NodeTdmArbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int, node : Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = new OcpBurstSlavePort(addrWidth, dataWidth, burstLen) 
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
    val slotEn = UInt(OUTPUT, 1)
  }
  debug(io.master)
  debug(io.slave)

  val cntReg = Reg(init = UInt(0, log2Up(cnt*(burstLen + 1))))
  // slot length = burst size + 1
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))
  val period = cnt * (burstLen + 1)
  val slotLen = burstLen + 1
  val cpuSlot = Vec.fill(cnt){Reg(init = UInt(0, width=1))}
  val slotTable = Vec.fill(cnt){Reg(init = Bits(0, width=period))}

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = sIdle)

  debug(cntReg)
  debug(cpuSlot(0))
  debug(cpuSlot(1))
  debug(cpuSlot(2))
  debug(stateReg)

  cntReg := Mux(cntReg === UInt(period - 1), UInt(0), cntReg + UInt(1))
  
  // Generater the slot Table for the whole period
  def genTable(nodeID: Int) = {
    val x = pow(2,nodeID*slotLen).toInt
    val slot = UInt(x,width=period)
    slot
  }
  
  for (i <- 0 to cnt-1){
    slotTable(i) := genTable(i).toBits
  }
  
  for(i <- 0 to cnt-1) {
    cpuSlot(i) := slotTable(i)(cntReg) 
  }
 
  io.slotEn := cpuSlot(node)
  
  // Initialize data to zero when cpuSlot is not enabled 
  io.slave.M.Addr       := Bits(0)
  io.slave.M.Cmd        := Bits(0)
  io.slave.M.DataByteEn := Bits(0)
  io.slave.M.DataValid  := Bits(0)
  io.slave.M.Data       := Bits(0)
   
  when (cpuSlot(node) === UInt(1)) {
    val master = io.master.M
    io.slave.M := io.master.M
        
    when (stateReg === sIdle) {
      when (master.Cmd != OcpCmd.IDLE){
        when (master.Cmd === OcpCmd.RD) {
          stateReg := sRead
        }
        when (master.Cmd === OcpCmd.WR) {
          stateReg := sWrite
          burstCntReg := UInt(0)
        }
      }
    }
  }

  when (stateReg === sWrite){
    io.slave.M := io.master.M
    // Wait on DVA
    when(io.slave.S.Resp === OcpResp.DVA){
      stateReg := sIdle
    }
  }
     
  when (stateReg === sRead){
    io.slave.M := io.master.M
    when (io.slave.S.Resp === OcpResp.DVA) {
      burstCntReg := burstCntReg + UInt(1)
        when (burstCntReg === UInt(burstLen) - UInt(1)) {
          stateReg := sIdle
        }
     }
  }      
  
  debug(io.slave.M)
  
  io.master.S := io.slave.S

}

/* Mux for all arbiters' outputs */
class memMuxIntf(nr: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends Module {
  val io = new Bundle {
    val master = Vec.fill(nr){new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)}
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
    val en = Vec.fill(nr){UInt(INPUT, 1)}
  }
    debug(io.master)
    debug(io.slave)
     
    val node = Reg(init=UInt(0, log2Up(nr)))
    
    // Initialized to zero when not enabled
    io.slave.M.Addr       := Bits(0)
    io.slave.M.Cmd        := Bits(0)
    io.slave.M.DataByteEn := Bits(0)
    io.slave.M.DataValid  := Bits(0)
    io.slave.M.Data       := Bits(0)
    
    for (i <- 0 until nr){
      when (io.en(i) === UInt(1)) {
        node := UInt(i)
      }
    }
    
    io.slave.M := io.master(node).M
    
    for (i <- 0 to nr - 1) {
      io.master(i).S.CmdAccept := Bits(0)
      io.master(i).S.DataAccept := Bits(0)
      io.master(i).S.Resp := OcpResp.NULL
      // we forward the data to all masters
      io.master(i).S.Data := io.slave.S.Data
    }
    io.master(node).S := io.slave.S  
}

object NodeTdmArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(5, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)
    val node = args(4)

    chiselMain(chiselArgs, () => Module(new NodeTdmArbiter(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt,node.toInt)))
  }
}



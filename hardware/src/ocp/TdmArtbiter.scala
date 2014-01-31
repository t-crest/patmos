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

import scala.collection.mutable.HashMap

class Arbiter(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen : Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = Vec.fill(cnt) { new OcpBurstSlavePort(addrWidth, dataWidth, burstLen) }
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  }
  debug(io.master)
  debug(io.slave)

  val cntReg = Reg(init = UInt(0, log2Up(cnt*(burstLen + 1))))
  // slot length = burst size + 1
  val burstCntReg = Reg(init = UInt(0, log2Up(burstLen)))
  val period = cnt * (burstLen + 1)
  val slotLen = UInt(burstLen + 1)
  val cpuTime = Vec.fill(cnt){Reg(init = UInt(0, log2Up(cnt*burstLen)))}
  val cpuSlot = Vec.fill(cnt){Reg(init = UInt(0, width=1))}

  val sIdle :: sRead :: sWrite :: Nil = Enum(UInt(), 3)
  val stateReg = Vec.fill(cnt){Reg(init = sIdle)}

  debug(cntReg)
  debug(cpuTime(1))
  debug(cpuSlot(0))
  debug(cpuSlot(1))
  debug(cpuSlot(2))
  debug(stateReg(0))
  debug(stateReg(1))
  debug(stateReg(2))

  cntReg := Mux(cntReg === UInt(period - 1), UInt(0), cntReg + UInt(1))
  
  def slotTable(nodeID: Int) = { 
    (cntReg === UInt(nodeID) * slotLen).toUInt
  }
  
  // Calculate slot information
  for (i <- 0 to cnt-1) {
    cpuTime(i) := UInt(i+1) * UInt(burstLen + 1) 
  }

  for (i <- 0 to cnt-1) {
    cpuSlot(i) := UInt(0)
  }
  
  for(i <- 0 to cnt-1) {
    cpuSlot(i) := slotTable(i) 
  }

   // Temporarily assigned to master 0
   val masterID = Reg(init = UInt(0, log2Up(cnt)))
   
    for (i <- 0 to cnt-1) {

      when (cpuSlot(i) === UInt(1)) {
        val master = io.master(i).M
        masterID := UInt(i)
        debug(master)
        
        when (stateReg(i) === sIdle) {
          when (master.Cmd != OcpCmd.IDLE){
            when (master.Cmd === OcpCmd.RD) {
              stateReg(i) := sRead
            }
          when (master.Cmd === OcpCmd.WR) {
            stateReg(i) := sWrite
            burstCntReg := UInt(0)
          }
        }
      }
     }

       when (stateReg(i) === sWrite){
         // Wait on DVA
         when(io.slave.S.Resp === OcpResp.DVA){
           stateReg(i) := sIdle
         }
       }
     
       when (stateReg(i) === sRead){
         when (io.slave.S.Resp === OcpResp.DVA) {
           burstCntReg := burstCntReg + UInt(1)
             when (burstCntReg === UInt(burstLen) - UInt(1)) {
               stateReg := sIdle
             }
           }
        }      
    } 
  
  io.slave.M := io.master(masterID).M 
  debug(io.slave.M)

  for (i <- 0 to cnt - 1) {
    io.master(i).S.CmdAccept := Bits(0)
    io.master(i).S.DataAccept := Bits(0)
    io.master(i).S.Resp := OcpResp.NULL
    // we forward the data to all masters
    io.master(i).S.Data := io.slave.S.Data
  }
  io.master(masterID).S := io.slave.S

}

object ArbiterMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(4, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)

    chiselMain(chiselArgs, () => Module(new Arbiter(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt)))
  }
}



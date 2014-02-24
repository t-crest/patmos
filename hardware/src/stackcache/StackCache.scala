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
 * Stack cache memory 
 * 
 * Author: Sahar Abbaspour (sabb@dtu.dk)
 * 
 */

package stackcache

import Chisel._
import Node._

import scala.math

import ocp._
import patmos._
import patmos.Constants._

class StackCache(SCACHE_SIZE: Int, burstLen: Int) extends Module {
  val io = new Bundle {
    val master = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH) // slave to cpu
    val slave = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, burstLen) // master to memory

  }

  val sc0 = Mem(Bits(width = BYTE_WIDTH), SCACHE_SIZE)
  val sc1 = Mem(Bits(width = BYTE_WIDTH), SCACHE_SIZE)
  val sc2 = Mem(Bits(width = BYTE_WIDTH), SCACHE_SIZE)
  val sc3 = Mem(Bits(width = BYTE_WIDTH), SCACHE_SIZE)

  val addrUFix = EXTMEM_ADDR_WIDTH //log2Up(mem_size)
  val sc_en = Mux(io.master.M.Cmd === OcpCmd.WR, io.master.M.ByteEn, UInt(0))

  val scLdStAddr = UInt(width = ADDR_WIDTH)
  scLdStAddr := io.master.M.Addr
  val cpu_addr_masked = UInt(width = ADDR_WIDTH)
  cpu_addr_masked := (scLdStAddr)(addrUFix + 1, 2)

  val rdData = Reg(Bits())

  val sResp = Reg(init = Bits(0, 2))

  val ldAddress = cpu_addr_masked
  rdData := Cat(sc3(ldAddress),
    sc2(ldAddress),
    sc1(ldAddress),
    sc0(ldAddress))

  io.slave.M.Data := rdData
  io.master.S.Data := rdData

  sResp := Mux(io.master.M.Cmd === OcpCmd.WR || io.master.M.Cmd === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)
  io.master.S.Resp := sResp

  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := UInt(0)
  io.slave.M.Data := UInt(0)
  io.slave.M.DataByteEn := UInt(0)
  io.slave.M.DataValid := UInt(0)

  val stData = io.master.M.Data
  val stAddr = cpu_addr_masked
  val scEn = sc_en

  when(scEn(0)) { sc0(stAddr) := stData(BYTE_WIDTH - 1, 0) }
  when(scEn(1)) { sc1(stAddr) := stData(2 * BYTE_WIDTH - 1, BYTE_WIDTH) }
  when(scEn(2)) { sc2(stAddr) := stData(3 * BYTE_WIDTH - 1, 2 * BYTE_WIDTH) }
  when(scEn(3)) { sc3(stAddr) := stData(DATA_WIDTH - 1, 3 * BYTE_WIDTH) }

}
  







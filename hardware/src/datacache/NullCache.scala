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
 * A no-op cache
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._
import Node._

import patmos.Constants._
import patmos.DataCachePerf

import ocp._

class NullCache() extends Module {
  val io = new Bundle {
    val master = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val invalidate = Bool(INPUT)
    val perf = new DataCachePerf()
  }

  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)

  val burstAddrBits = log2Up(BURST_LENGTH)
  val byteAddrBits = log2Up(DATA_WIDTH/8)

  // State machine for read bursts
  val idle :: read :: readResp :: Nil = Enum(UInt(), 3)
  val stateReg = Reg(init = idle)
  val burstCntReg = Reg(init = UInt(0, burstAddrBits))
  val posReg = Reg(Bits(width = burstAddrBits))

  // Register for master signals
  val masterReg = Reg(io.master.M)

  // Register to delay response
  val slaveReg = Reg(io.master.S)

  when(masterReg.Cmd != OcpCmd.RD || io.slave.S.CmdAccept === Bits(1)) {
    masterReg := io.master.M
  }
  when(reset) {
    masterReg.Cmd := OcpCmd.IDLE;
  }

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(EXTMEM_ADDR_WIDTH-1, burstAddrBits+byteAddrBits),
                         Fill(Bits(0), burstAddrBits+byteAddrBits))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  io.master.S.Resp := OcpResp.NULL
  io.master.S.Data := Bits(0)

  // Wait for response
  when(stateReg === read) {
    when(burstCntReg === posReg) {
      slaveReg := io.slave.S
    }
    when(io.slave.S.Resp === OcpResp.DVA) {
      when(burstCntReg === UInt(BURST_LENGTH-1)) {
        stateReg := readResp
      }
      burstCntReg := burstCntReg + UInt(1)
    }
  }
  // Pass data to master
  when(stateReg === readResp) {
    io.master.S := slaveReg
    stateReg := idle
  }

  // Start a read burst
  when(masterReg.Cmd === OcpCmd.RD) {
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := read
      posReg := masterReg.Addr(burstAddrBits+byteAddrBits-1, byteAddrBits)
      io.perf.miss := Bool(true)
    }
  }
}

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
 * A direct-mapped cache
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._
import Node._

import patmos.Constants._
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class DirectMappedCache(size: Int, lineSize: Int) extends Module {
  val io = new Bundle {
    val master = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, lineSize/4)
  }

  val addrBits = log2Up(size / BYTES_PER_WORD)
  val lineBits = log2Up(lineSize)

  val tagWidth = EXTMEM_ADDR_WIDTH - addrBits - 2
  val tagCount = size / lineSize

  // Register signals from master
  val masterReg = Reg(init = OcpMasterSignals.resetVal(io.master.M))
  masterReg := io.master.M

  // Generate memories
  val tagMem = MemBlock(tagCount, tagWidth)
  val tagVMem = Vec.fill(tagCount) { Reg(init = Bool(false)) }
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val tag = tagMem.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tagV = Reg(next = tagVMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagValid = tagV && tag === Cat(masterReg.Addr(EXTMEM_ADDR_WIDTH-1, addrBits+2))

  val fillReg = Reg(init = Bool(false))
  val fillAddrReg = Reg(init = Bits(0, width = addrBits+2 - lineBits))

  val wrAddrReg = Reg(Bits(width = addrBits))
  val wrDataReg = Reg(Bits(width = DATA_WIDTH))

  wrAddrReg := io.master.M.Addr(addrBits + 1, 2)
  wrDataReg := io.master.M.Data

  // Write to cache; store only updates what's already there
  val stmsk = Mux(masterReg.Cmd === OcpCmd.WR, masterReg.ByteEn,  Bits("b0000"))
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) <= (fillReg || (tagValid && stmsk(i)), wrAddrReg,
               wrDataReg(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
  }

  // Read from cache
  val rdData = mem.map(_(io.master.M.Addr(addrBits + 1, 2))).reduceLeft((x,y) => y ## x)

  // Return data on a hit
  io.master.S.Data := rdData
  io.master.S.Resp := Mux(tagValid && masterReg.Cmd === OcpCmd.RD,
                          OcpResp.DVA, OcpResp.NULL)

  // State machine for misses
  val idle :: hold :: fill :: respond :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = idle)

  val missIndexReg = Reg(init = UInt(0, lineBits-2))
  val burstCntReg = Reg(init = UInt(0, lineBits-2))

  // Register to delay response
  val slaveReg = Reg(init = OcpSlaveSignals.resetVal(io.master.S))

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(EXTMEM_ADDR_WIDTH-1, lineBits),
                         Fill(Bits(0), lineBits))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  fillReg := Bool(false)

  // Start handling a miss
  when(!tagValid && masterReg.Cmd === OcpCmd.RD) {
    fillAddrReg := masterReg.Addr(addrBits + 1, lineBits)
    tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
    missIndexReg := masterReg.Addr(lineBits-1, 2).toUInt
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := fill
    }
    .otherwise {
      stateReg := hold
      masterReg.Addr := masterReg.Addr
    }
  }
  tagMem.io <= (!tagValid && masterReg.Cmd === OcpCmd.RD,
                masterReg.Addr(addrBits + 1, lineBits),
                masterReg.Addr(EXTMEM_ADDR_WIDTH-1, addrBits+2))

  // Hold read command
  when(stateReg === hold) {
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := fill
    }
    .otherwise {
      stateReg := hold
      masterReg.Addr := masterReg.Addr
    }
  }
  // Wait for response
  when(stateReg === fill) {
    wrAddrReg := Cat(fillAddrReg, burstCntReg)

    when(io.slave.S.Resp === OcpResp.DVA) {
      fillReg := Bool(true)
      wrDataReg := io.slave.S.Data
      when(burstCntReg === missIndexReg) {
        slaveReg := io.slave.S
      }
      when(burstCntReg === UInt(lineSize/4-1)) {
        stateReg := respond
      }
      burstCntReg := burstCntReg + UInt(1)
    }
  }
  // Pass data to master
  when(stateReg === respond) {
    io.master.S := slaveReg
    stateReg := idle
  }

}

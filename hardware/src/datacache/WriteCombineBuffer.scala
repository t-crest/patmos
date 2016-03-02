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
 * A write combine buffer
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._
import Node._

import patmos.Constants._
import patmos.WriteCombinePerf

import ocp._

class WriteCombineBuffer() extends Module {
  val io = new Bundle {
    val readMaster = new OcpBurstSlavePort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val writeMaster = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val perf = new WriteCombinePerf()
  }

  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)

  val addrWidth = io.writeMaster.M.Addr.getWidth()
  val dataWidth = io.writeMaster.M.Data.getWidth()
  val byteEnWidth = io.writeMaster.M.ByteEn.getWidth()
  val burstLength = io.readMaster.burstLength
  val burstAddrBits = log2Up(burstLength)
  val byteAddrBits = log2Up(dataWidth/8)
  val tagWidth = addrWidth - burstAddrBits - byteAddrBits

  // State of transmission
  val idle :: read :: write :: writeResp :: writeComb :: writeSnoop :: Nil = Enum(Bits(), 6)
  val state = Reg(init = idle)
  val cntReg = Reg(init = UInt(0, burstAddrBits))

  // Register signals that come from write master
  val writeMasterReg = Reg(io.writeMaster.M)

  // Register signals that come from read master
  val readMasterReg = Reg(next = io.readMaster.M)

  // Registers for write combining
  val tagReg = Reg(init = Bits(0, width = tagWidth))
  val dataBuffer = Vec.fill(burstLength) { Reg(Bits(width = dataWidth)) }
  val byteEnBuffer = Vec.fill(burstLength) { Reg(init = Bits(0, width = byteEnWidth)) }
  val hitReg = Reg(Bool())

  // Temporary vector for combining
  val comb = Vec.fill(byteEnWidth) { Bits(width = 8) }
  for (i <- 0 until byteEnWidth) {
    comb(i) := Bits(0)
  }

  // Default responses
  io.readMaster.S := io.slave.S
  io.writeMaster.S := io.slave.S
  io.writeMaster.S.Resp := OcpResp.NULL

  // Reads are the default towards the slave
  io.slave.M := io.readMaster.M

  // Pass on reads, fill in buffered data if necessary
  when(state === read) {
    io.readMaster.S.Resp := io.slave.S.Resp
    when(hitReg) {
      for (i <- 0 until byteEnWidth) {
        comb(i) := Mux(byteEnBuffer(cntReg)(i) === Bits(1),
                       dataBuffer(cntReg)(8*i+7, 8*i),
                       io.slave.S.Data(8*i+7, 8*i))
      }
      io.readMaster.S.Data := comb.reduceLeft((x,y) => y##x)
    }
    when(io.slave.S.Resp =/= OcpResp.NULL) {
      when(cntReg === UInt(burstLength - 1)) {
        state := idle
      }
      cntReg := cntReg + UInt(1)
    }
  }

  val wrPos = writeMasterReg.Addr(burstAddrBits+byteAddrBits-1, byteAddrBits)

  // Write burst
  when(state === write) {
    io.readMaster.S.Resp := OcpResp.NULL
    when(cntReg === Bits(0)) {
      io.slave.M.Cmd := OcpCmd.WR
      io.slave.M.Addr := Cat(tagReg, Fill(Bits(0), burstAddrBits+byteAddrBits))
    }
    io.slave.M.DataValid := Bits(1)
    io.slave.M.Data := dataBuffer(cntReg)
    io.slave.M.DataByteEn := byteEnBuffer(cntReg)

    when(io.slave.S.DataAccept === Bits(1)) {
      tagReg := writeMasterReg.Addr(addrWidth-1, burstAddrBits+byteAddrBits)
      byteEnBuffer(cntReg) := Bits(0)
      when(cntReg === wrPos) {
        dataBuffer(cntReg) := writeMasterReg.Data
        byteEnBuffer(cntReg) := writeMasterReg.ByteEn
      }
      cntReg := cntReg + UInt(1)
    }
    when(cntReg === UInt(burstLength - 1)) {
      state := writeResp
    }
  }
  when(state === writeResp) {
    io.readMaster.S.Resp := OcpResp.NULL
    io.writeMaster.S.Resp := io.slave.S.Resp
    when(io.slave.S.Resp =/= OcpResp.NULL) {
      state := idle
    }
  }

  // Write combining
  when(state === writeComb) {
    io.writeMaster.S.Resp := OcpResp.DVA
    for (i <- 0 until byteEnWidth) {
      comb(i) := Mux(writeMasterReg.ByteEn(i) === Bits(1),
                     writeMasterReg.Data(8*i+7, 8*i),
                     dataBuffer(wrPos)(8*i+7, 8*i))
    }
    dataBuffer(wrPos) := comb.reduceLeft((x,y) => y##x)
    byteEnBuffer(wrPos) := byteEnBuffer(wrPos) | writeMasterReg.ByteEn
    state := idle
  }

  // Snoop writes from readMaster
  val dataAcceptReg = Reg(next = io.slave.S.DataAccept)
  when(state === writeSnoop) {
    when(dataAcceptReg === Bits(1)) {
      when (hitReg) {
        for (i <- 0 until byteEnWidth) {
          comb(i) := Mux(readMasterReg.DataByteEn(i) === Bits(1),
                         readMasterReg.Data(8*i+7, 8*i),
                         dataBuffer(cntReg)(8*i+7, 8*i))
        }
        dataBuffer(cntReg) := comb.reduceLeft((x,y) => y##x)
        byteEnBuffer(cntReg) := byteEnBuffer(cntReg) | readMasterReg.DataByteEn
      }
      cntReg := cntReg + UInt(1)
    }
    when(cntReg === UInt(burstLength - 1)) {
      state := idle
    }
  }

  // Start new read transaction
  when(io.readMaster.M.Cmd === OcpCmd.RD) {
    state := read
    io.slave.M := io.readMaster.M
    hitReg := tagReg === io.readMaster.M.Addr(addrWidth-1, burstAddrBits+byteAddrBits)
  }
  // Start write transactions
  when(io.writeMaster.M.Cmd === OcpCmd.WR) {
    writeMasterReg := io.writeMaster.M
    when (tagReg === io.writeMaster.M.Addr(addrWidth-1, burstAddrBits+byteAddrBits)) {
      state := writeComb
      io.perf.hit := Bool(true)
    } .otherwise {
      state := write
      io.perf.miss := Bool(true)
    }
  }
  // Snoop writes from readMaster
  when(io.readMaster.M.Cmd === OcpCmd.WR) {
    state := writeSnoop
    hitReg := tagReg === io.readMaster.M.Addr(addrWidth-1, burstAddrBits+byteAddrBits)
  }

}


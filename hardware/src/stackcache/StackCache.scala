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

package patmos

import Chisel._
import Node._

import scala.math

import ocp._
//import patmos._
import patmos.Constants._

class StackCache(SCACHE_SIZE: Int, burstLen: Int) extends Module {
  val io = new Bundle {
    val master = new OcpCoreSlavePort(EXTMEM_ADDR_WIDTH, DATA_WIDTH) // slave to cpu
    val slave = new OcpBurstMasterPort(EXTMEM_ADDR_WIDTH, DATA_WIDTH, burstLen) // master to memory
    val scIO = new StackCacheIO()
  }

  val addrBits = log2Up(SCACHE_SIZE / BYTES_PER_WORD)
  val lineBits = log2Up(burstLen * BYTES_PER_WORD)
  val SC_MASK = Bits(width = log2Up(SCACHE_SIZE / BYTES_PER_WORD))
  SC_MASK := Bits(SCACHE_SIZE / BYTES_PER_WORD) - Bits(1)
  val sc_en = Mux(io.master.M.Cmd === OcpCmd.WR, io.master.M.ByteEn, Bits("b0000"))
  val sc = new Array[MemBlockIO](BYTES_PER_WORD)

  for (i <- 0 until BYTES_PER_WORD) {
    sc(i) = MemBlock(SCACHE_SIZE / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val mTopReg = Reg(init = Bits(0, width = EXTMEM_ADDR_WIDTH))

  val stall = Reg(init = Bits(1, 1))
  val spill = Reg(init = Bits(0, 1))
  val fill = Reg(init = Bits(0, 1))
  val free = Reg(init = Bits(0, 1))
  val fillEn = Reg(init = Bits(0, width = BYTES_PER_WORD))
  val nFillReg = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val nFill = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val nFillCnt = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val nSpill = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val nSpillCnt = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val nSpillReg = Reg(init = Bits(0, width = log2Up(SCACHE_SIZE)))
  val enUp = Reg(init = Bits(0, width = log2Up(burstLen + 1)))

  val burstCnt = Reg(init = SInt(0, width = log2Up(burstLen + 1)))
  val init_st :: wait_st :: spill_st :: fill_st :: Nil = Enum(Bits(), 4)
  val state = Reg(init = init_st)

  val slaveCmdReg = Reg(init = Bits(0, width = 3))
  val slaveDataValidReg = Reg(init = Bits(0, width = 1))
  val slaveAddrReg = Reg(init = Bits(0, width = ADDR_WIDTH))
  val slaveDataByteEnReg = Reg(init = Bits(0, burstLen))
  val slaveRespReg = Reg(init = Bits(0, 2))
  val slaveRespCmdReg = Reg(init = Bits(0, width = 1))

  val cpuAddr = io.master.M.Addr(EXTMEM_ADDR_WIDTH - 1, 2) & SC_MASK

  val rdData = Reg(Bits())

  val sResp = Reg(init = Bits(0, 2))

  val ldAddrreg = Reg(init = Bits(0, 12))
  when(stall != Bits(0)) {
    mTopReg := io.scIO.exsc.mTop
    spill := io.scIO.exsc.spill
    fill := io.scIO.exsc.fill
    free := io.scIO.exsc.free
    nSpillReg := io.scIO.exsc.nSpill
    nFillReg := io.scIO.exsc.nFill
  }

  val ldAddress = Mux(spill === Bits(1) || io.scIO.exsc.spill === Bits(1), (slaveAddrReg(EXTMEM_ADDR_WIDTH - 1, 2) + Bits(1)) & SC_MASK, cpuAddr)
  val ldData = sc.map(_(ldAddress)).reduceLeft((x, y) => y ## x)
  ldAddrreg := ldAddress

  rdData := ldData

  sResp := Mux(io.master.M.Cmd === OcpCmd.WR || io.master.M.Cmd === OcpCmd.RD, OcpResp.DVA, OcpResp.NULL)

  val slaveAddr = Reg(init = Bits(0, width = ADDR_WIDTH))
  val slaveData = Reg(init = Bits(0, width = ADDR_WIDTH))
  val alignSizeDown = mTopReg(log2Up(burstLen) + 1, 2)
  val alignmTopDown = mTopReg - (alignSizeDown << Bits(log2Up(burstLen)))
  slaveAddr := slaveAddrReg
  slaveData := io.slave.S.Data

  val stData = Mux(fill === Bits(1) || io.scIO.exsc.fill === Bits(1), slaveData, io.master.M.Data)
  val stAddr = Mux(fill === Bits(1) || io.scIO.exsc.fill === Bits(1), (slaveAddrReg)(EXTMEM_ADDR_WIDTH - 1, 2) & SC_MASK, cpuAddr)
  val scEn = Mux(fill === Bits(1) || io.scIO.exsc.fill === Bits(1), fillEn, sc_en)

  for (i <- 0 until BYTES_PER_WORD) {
    sc(i) <= (scEn(i), stAddr, stData(BYTE_WIDTH * (i + 1) - 1, BYTE_WIDTH * i))
  }

  io.slave.M.Cmd := slaveCmdReg
  io.slave.M.Addr := slaveAddrReg
  io.slave.M.DataValid := slaveDataValidReg
  io.slave.M.DataByteEn := slaveDataByteEnReg
  slaveRespReg := io.slave.S.Resp
  slaveRespCmdReg := io.slave.S.CmdAccept
  io.scIO.stall := Bits(1)
  io.scIO.scex.mTop := mTopReg
  io.slave.M.Data := ldData
  io.master.S.Data := ldData
  io.master.S.Resp := sResp

  val mTopRegPrev = mTopReg - Bits(4)
  val mTopRegDown = mTopRegPrev - ((nSpillReg - Bits(1)) << Bits(2))
  val alignSize = mTopRegDown(log2Up(burstLen) + 1, 2)
  val alignmTop = mTopRegDown - (alignSize << Bits(log2Up(burstLen)))

  when (io.scIO.exsc.spill === Bits(1) || io.scIO.exsc.fill === Bits(1)) {
    io.scIO.stall := Bits(0)
    stall := Bits(0)
  }
  when(state === init_st) {
    when(spill === Bits(1) && stall === Bits(1)) {
      io.scIO.stall := Bits(0)
      stall := Bits(0)
      nSpill := Bits(0)
      nSpillCnt := nSpillReg + alignSize
      slaveCmdReg := OcpCmd.WR
      slaveAddrReg := alignmTop
      mTopReg := mTopRegDown
      slaveDataValidReg := Bits(1)
      slaveDataByteEnReg := Mux(alignSize === Bits(0), Bits("b1111"), Bits("b0000"))
      spill := Bits(1)
      state := wait_st
      enUp := alignSize
      burstCnt := Bits(burstLen - 1)
    }
      .elsewhen(fill === Bits(1) && stall === Bits(1)) {
        io.scIO.stall := Bits(0)
        stall := Bits(0)
        slaveAddrReg := alignmTopDown
        slaveCmdReg := OcpCmd.RD
        nFill := Bits(0)
        burstCnt := Bits(0)
        nFillCnt := nFillReg + alignSizeDown
        fillEn := Mux(alignSizeDown === Bits(0), Bits("b1111"), Bits("b0000"))
        fill := Bits(1)
        state := wait_st
        enUp := alignSizeDown
        burstCnt := Bits(0)
      }
      .elsewhen(free === Bits(1)) {
        stall := Bits(1)
        io.scIO.stall := Bits(1)
        when(io.scIO.exsc.sp > mTopReg) {
          mTopReg := io.scIO.exsc.sp
        }
        when(fill === UInt(1)) {
          state := fill_st
        }
          .elsewhen(spill === UInt(1)) {
            state := spill_st
          }

      }
      .otherwise {
        io.scIO.stall := Bits(1)
        stall := Bits(1)
        io.slave.M.Cmd := OcpCmd.IDLE
        slaveAddrReg := mTopReg
        slaveDataValidReg := Bits(0)
        slaveDataByteEnReg := Bits(0)
      }
  }

  when(state === wait_st) {
    io.scIO.stall := Bits(0)
    when(io.slave.S.DataAccept === Bits(1) && spill === Bits(1)) {
      slaveCmdReg := OcpCmd.IDLE
      spill := Bits(1)
      burstCnt := burstCnt - Bits(1)
      state := spill_st
      slaveAddrReg := slaveAddrReg + Bits(4)
      nSpill := nSpill + Bits(1)
      slaveDataByteEnReg := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
      enUp := Mux(enUp != Bits(0) && ((enUp.toSInt - SInt(2)) > SInt(0)), enUp - Bits(1), Bits(0))
    }

    when(slaveRespCmdReg === Bits(1) && fill === Bits(1)) {
      slaveCmdReg := OcpCmd.IDLE

      //burstCnt := burstCnt + Bits(1)
      fillEn := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
      enUp := Mux(enUp != Bits(0) && ((enUp.toSInt - SInt(1)) >= SInt(0)), enUp - Bits(1), Bits(0))
      // nFill := nFill + Bits(1)
      state := fill_st
    }
  }

  when(state === spill_st) {
    io.scIO.stall := Bits(0)
    when(nSpill + Bits(1) < nSpillCnt) {
      when(burstCnt - SInt(1) >= SInt(0)) {
        burstCnt := burstCnt - Bits(1)
        nSpill := nSpill + Bits(1)
        spill := Bits(1)
        slaveDataByteEnReg := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
        slaveAddrReg := slaveAddrReg + Bits(4)
        enUp := Mux((enUp != Bits(0) && (enUp.toSInt - SInt(2)) > SInt(0)), enUp - Bits(1), Bits(0))
      }
        .otherwise {
          when ((slaveAddrReg + Bits(4) )> io.scIO.exsc.lp ) {slaveCmdReg := OcpCmd.WR}
          spill := Bits(1)
          burstCnt := Bits(burstLen - 1)
          slaveDataByteEnReg := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
          slaveDataValidReg := Bits(1)
          slaveAddrReg := slaveAddrReg + Bits(4)
          state := wait_st
          nSpill := nSpill + Bits(1)
          enUp := Mux((enUp != Bits(0) && (enUp.toSInt - SInt(2)) > SInt(0)), enUp - Bits(1), Bits(0))
        }
    }
      .otherwise {
        slaveCmdReg := OcpCmd.IDLE
        slaveDataByteEnReg := Bits("b0000")
        slaveDataValidReg := Bits(0)
        state := init_st
        spill := Bits(0)
      }
  }

  when(state === fill_st) {
    io.scIO.stall := Bits(0)
    when(nFill + Bits(1) < nFillCnt) {
      when(slaveRespReg === OcpResp.DVA) {
        when(burstCnt === Bits(burstLen - 1)) {
          slaveCmdReg := OcpCmd.RD
          slaveAddrReg := slaveAddrReg + Bits(4)
          enUp := Mux(enUp != Bits(0) && ((enUp.toSInt - SInt(1)) >= SInt(0)), enUp - Bits(1), Bits(0))
          fillEn := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
          state := wait_st
          nFill := nFill + Bits(1)
          burstCnt := Bits(0)
        }
          .otherwise {
            nFill := nFill + Bits(1)
            slaveAddrReg := slaveAddrReg + Bits(4)
            fill := Bits(1)
            fillEn := Mux(enUp === Bits(0), Bits("b1111"), Bits("b0000"))
            enUp := Mux(enUp != Bits(0) && ((enUp.toSInt - SInt(1)) >= SInt(0)), enUp - Bits(1), Bits(0))
            burstCnt := burstCnt + Bits(1)
          }

      }
    }
      .otherwise {

        slaveCmdReg := OcpCmd.IDLE

        state := init_st
        fill := Bits(0)
        fillEn := Bits("b0000")
        mTopReg := slaveAddrReg + Bits(4)
      }
  }

}

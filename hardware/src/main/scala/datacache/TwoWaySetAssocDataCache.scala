/*
 * A two-way set associative cache with LRU replacement
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *          Sahar Abbaspour (sabb@dtu.dk)
 */

package datacache

import Chisel._
import chisel3.VecInit

import patmos.Constants._
import patmos.DataCachePerf
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class TwoWaySetAssociativeCache(size: Int, lineSize: Int) extends DCacheType(lineSize/4) {

  io.perf.hit := false.B
  io.perf.miss := false.B

  val addrBits = log2Up((size / 2) / BYTES_PER_WORD)
  val lineBits = log2Up(lineSize)

  val tagWidth = ADDR_WIDTH - addrBits - 2
  val tagCount = (size / 2) / lineSize

  // Register signals from master
  val masterReg = Reg(io.master.M)
  masterReg := io.master.M

  // Generate memories
  val tagMem1 = MemBlock(tagCount, tagWidth)
  val tagMem2 = MemBlock(tagCount, tagWidth)
  val tagVMem1 = RegInit(VecInit(Seq.fill(tagCount)(false.B)))
  val tagVMem2 = RegInit(VecInit(Seq.fill(tagCount)(false.B)))
  val lruMem = RegInit(VecInit(Seq.fill(size / 2/ BYTES_PER_WORD)(false.B))) // if 0 use first way to replace if 1 use second way
  val mem1 = new Array[MemBlockIO](BYTES_PER_WORD)
  val mem2 = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem1(i) = MemBlock((size / 2) / BYTES_PER_WORD, BYTE_WIDTH).io
    mem2(i) = MemBlock((size / 2) / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val tag1 = tagMem1.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tag2 = tagMem2.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tagV1 = Reg(next = tagVMem1(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagV2 = Reg(next = tagVMem2(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagValid1 = tagV1 && tag1 === Cat(masterReg.Addr(ADDR_WIDTH - 1, addrBits + 2))
  val tagValid2 = tagV2 && tag2 === Cat(masterReg.Addr(ADDR_WIDTH - 1, addrBits + 2))
  val lru = Reg(next = lruMem(io.master.M.Addr(addrBits + 1, lineBits)))

  val fillReg = Reg(Bool())

  val wrAddrReg = Reg(Bits(width = addrBits))
  val wrDataReg = Reg(Bits(width = DATA_WIDTH))
  val lruReg = Reg(Bool())

  wrAddrReg := io.master.M.Addr(addrBits + 1, 2)
  wrDataReg := io.master.M.Data

  // Write to cache; store only updates what's already there
  val stmsk = Mux(masterReg.Cmd === OcpCmd.WR, masterReg.ByteEn,  "b0000".U(4.W))
   
  for (i <- 0 until BYTES_PER_WORD) {
      mem1(i) <= (((fillReg && !lruReg) || (tagValid1 && stmsk(i))), wrAddrReg,
        wrDataReg(BYTE_WIDTH * (i + 1) - 1, BYTE_WIDTH * i))

      mem2(i) <= (((fillReg && lruReg) || (tagValid2 && stmsk(i))), wrAddrReg,
        wrDataReg(BYTE_WIDTH * (i + 1) - 1, BYTE_WIDTH * i))

  }
  


  // Read from cache
  val rdData1 = mem1.map(_(io.master.M.Addr(addrBits + 1, 2))).reduceLeft((x,y) => y ## x)
  val rdData2 = mem2.map(_(io.master.M.Addr(addrBits + 1, 2))).reduceLeft((x,y) => y ## x)
  // Return data on a hit
  io.master.S.Data := Mux(tagValid1, rdData1, rdData2)
  io.master.S.Resp := Mux((tagValid1 || tagValid2) && (masterReg.Cmd === OcpCmd.RD),
                          OcpResp.DVA, OcpResp.NULL)
                          
  // Update lru on hit
  when ((masterReg.Cmd === OcpCmd.WR || masterReg.Cmd === OcpCmd.RD) && (tagValid1 || tagValid2) && fillReg === false.B) {
	  lruMem(masterReg.Addr(addrBits + 1, 2)) := Mux(tagValid1, true.B, false.B)
  }


  // State machine for misses
  val idle :: hold :: fill :: respond :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = idle)

  val burstCntReg = Reg(init = 0.U((lineBits-2).W))
  val missIndexReg = Reg((lineBits-2).U)

  // Register to delay response
  val slaveReg = Reg(io.master.S)

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, lineBits),
                         Fill(lineBits, Bits(0)))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  fillReg := false.B

  // Record a hit
  when((tagValid1 || tagValid2) && masterReg.Cmd === OcpCmd.RD) {
    io.perf.hit := true.B
  }

  // Start handling a miss
  when((!tagValid1 && !tagValid2) && masterReg.Cmd === OcpCmd.RD) {
    lruReg := lru
    when (lru === false.B) {
      tagVMem1(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    }
    .otherwise {
      tagVMem2(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    }

    missIndexReg := masterReg.Addr(lineBits-1, 2).asUInt
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := fill
      lruMem(masterReg.Addr(addrBits + 1, 2)) := !lru
    }
    .otherwise {
      stateReg := hold
    }
    masterReg.Addr := masterReg.Addr
    io.perf.miss := true.B
  }
  
  tagMem1.io <= (!tagValid1 && !tagValid2 && !lru && masterReg.Cmd === OcpCmd.RD,
                masterReg.Addr(addrBits + 1, lineBits),
                masterReg.Addr(ADDR_WIDTH-1, addrBits+2))
                
  tagMem2.io <= (!tagValid1 && !tagValid2 && lru && masterReg.Cmd === OcpCmd.RD,
                masterReg.Addr(addrBits + 1, lineBits),
                masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  // Hold read command
  when(stateReg === hold) {
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := fill
    }
    .otherwise {
      stateReg := hold
    }
    masterReg.Addr := masterReg.Addr
  }
  // Wait for response
  when(stateReg === fill) {
    wrAddrReg := Cat(masterReg.Addr(addrBits + 1, lineBits), burstCntReg)    
    
    when(io.slave.S.Resp =/= OcpResp.NULL) {
      fillReg := true.B
      wrDataReg := io.slave.S.Data
      when(burstCntReg === missIndexReg) {
        slaveReg := io.slave.S
      }
      when(burstCntReg === (lineSize/4-1).U) {
        stateReg := respond
      }
      burstCntReg := burstCntReg + 1.U
    }
    when(io.slave.S.Resp === OcpResp.ERR) {
      when (lru === false.B) {
        tagVMem1(masterReg.Addr(addrBits + 1, lineBits)) := false.B
      }
      .otherwise {
        tagVMem2(masterReg.Addr(addrBits + 1, lineBits)) := false.B
      }
    }
    masterReg.Addr := masterReg.Addr
  }
  // Pass data to master
  when(stateReg === respond) {
    io.master.S := slaveReg
    stateReg := idle
  }

  // reset valid bits
  when (io.invalidate) {
    tagVMem1.map(_ := false.B)
    tagVMem2.map(_ := false.B)
  }
}

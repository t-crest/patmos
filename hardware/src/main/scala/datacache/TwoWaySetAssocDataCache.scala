/*
 * A two-way set associative cache with LRU replacement
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *          Sahar Abbaspour (sabb@dtu.dk)
 */

package datacache

import Chisel._

import patmos.Constants._
import patmos.DataCachePerf
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class TwoWaySetAssociativeCache(size: Int, lineSize: Int) extends Module {
  val io = IO(new Bundle {
    val master = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, lineSize / 4)
    val invalidate = Bool(INPUT)
    val perf = new DataCachePerf()
  })

  io.perf.hit := Bool(false)
  io.perf.miss := Bool(false)

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
  val tagVMem1 = Vec.fill(tagCount) { Reg(init = Bool(false)) }
  val tagVMem2 = Vec.fill(tagCount) { Reg(init = Bool(false)) }
  val lruMem = Vec.fill(size / 2/ BYTES_PER_WORD) { Reg(init = Bool(false)) } // if 0 use first way to replace if 1 use second way
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
  val stmsk = Mux(masterReg.Cmd === OcpCmd.WR, masterReg.ByteEn,  Bits("b0000"))
   
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
  when ((masterReg.Cmd === OcpCmd.WR || masterReg.Cmd === OcpCmd.RD) && (tagValid1 || tagValid2) && fillReg === Bool(false)) { 
	  lruMem(masterReg.Addr(addrBits + 1, 2)) := Mux(tagValid1, Bool(true), Bool(false))
  }


  // State machine for misses
  val idle :: hold :: fill :: respond :: Nil = Enum(UInt(), 4)
  val stateReg = Reg(init = idle)

  val burstCntReg = Reg(init = UInt(0, lineBits-2))
  val missIndexReg = Reg(UInt(lineBits-2))

  // Register to delay response
  val slaveReg = Reg(io.master.S)

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, lineBits),
                         Fill(lineBits, Bits(0)))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  fillReg := Bool(false)

  // Record a hit
  when((tagValid1 || tagValid2) && masterReg.Cmd === OcpCmd.RD) {
    io.perf.hit := Bool(true)
  }

  // Start handling a miss
  when((!tagValid1 && !tagValid2) && masterReg.Cmd === OcpCmd.RD) {
    lruReg := lru
    when (lru === Bool(false)) {
      tagVMem1(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
    }
    .otherwise {
      tagVMem2(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
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
    io.perf.miss := Bool(true)
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
    when(io.slave.S.Resp === OcpResp.ERR) {
      when (lru === Bool(false)) {
        tagVMem1(masterReg.Addr(addrBits + 1, lineBits)) := Bool(false)
      }
      .otherwise {
        tagVMem2(masterReg.Addr(addrBits + 1, lineBits)) := Bool(false)
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
    tagVMem1.map(_ := Bool(false))
    tagVMem2.map(_ := Bool(false))
  }
}

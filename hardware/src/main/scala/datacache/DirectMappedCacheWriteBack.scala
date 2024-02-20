/*
 * A direct-mapped cache
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import chisel3._
import chisel3.util._

import patmos.Constants._
import patmos.DataCachePerf
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class DirectMappedCacheWriteBack(size: Int, lineSize: Int) extends DCacheType(lineSize/4) {

  val addrBits = log2Up(size / BYTES_PER_WORD)
  val lineBits = log2Up(lineSize)
  val dataWidth = io.master.M.Data.getWidth
  val burstLength = io.slave.burstLength
  val burstAddrBits = log2Up(burstLength)
  val byteAddrBits = log2Up(dataWidth/8)
  val addrWidth = io.master.M.Addr.getWidth

  val doingRead = Reg(Bool()) // are we reading or writing
  val coreWrDataReg = Reg(UInt(DATA_WIDTH.W)) // register for WR cmd data
  val coreByteEnReg = Reg(UInt((DATA_WIDTH/BYTE_WIDTH).W)) // register for WR cmd byte enables

  // Temporary vector for combining written bytes with bytes from memory
  val comb = Wire(Vec(DATA_WIDTH/BYTE_WIDTH, UInt(BYTE_WIDTH.W)))
  for (i <- 0 until DATA_WIDTH/BYTE_WIDTH) { comb(i) := 0.U }

  val tagWidth = ADDR_WIDTH - addrBits - 2
  val tagCount = size / lineSize

  // Register signals from master
  val masterReg = Reg(chiselTypeOf(io.master.M))
  masterReg := io.master.M

  // State machine for misses
  val idle :: write :: writeWaitForResp :: hold :: fill :: respond :: Nil = Enum(6)
  val stateReg = RegInit(init = idle)

  val burstCntReg = RegInit(init = 0.U((lineBits-2).W))
  val missIndexReg = Reg(chiselTypeOf((lineBits-2).U))

  // Generate memories
  val tagMem = MemBlock(tagCount, tagWidth)
  val tagVMem = RegInit(VecInit(Seq.fill(tagCount)(false.B)))
  val dirtyMem = RegInit(VecInit(Seq.fill(tagCount)(false.B)))
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val tag = tagMem.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tagV = RegNext(next = tagVMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val dirty = RegNext(next = dirtyMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagValid = tagV && tag === Cat(masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  val fillReg = Reg(Bool())

  val wrAddrReg = Reg(UInt(addrBits.W))
  val wrDataReg = Reg(UInt(DATA_WIDTH.W))

  wrAddrReg := io.master.M.Addr(addrBits + 1, 2)
  wrDataReg := io.master.M.Data

  // Write to cache; store only updates what's already there
  val stmsk = Mux(masterReg.Cmd === OcpCmd.WR, masterReg.ByteEn,  "b0000".U(4.W))
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) <= (fillReg || (tagValid && stmsk(i)), wrAddrReg,
               wrDataReg(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
  }
  // Update dirty bit when writing
  when(tagValid && (stmsk =/= "b0000".U(4.W))) {
    dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    when(io.master.M.Addr(addrBits + 1, lineBits) === masterReg.Addr(addrBits + 1, lineBits)) {
      dirty := true.B;
    }
  }


  // Count register used to index reads for write-back
  val rdAddrCntReg = RegInit(init = 0.U((lineBits-2).W))
  // Read from cache
  val selWrBack = Wire(Bool())
  val rdAddr = Mux(selWrBack, Cat(masterReg.Addr(addrBits + 1, lineBits), rdAddrCntReg), io.master.M.Addr(addrBits + 1, 2)) // helper signal
  val rdData = mem.map(_(rdAddr)).reduceLeft((x,y) => y ## x)
  val rdDataReg = RegNext(next = rdData)

  // Return data on a hit
  io.master.S.Data := rdData
  io.master.S.Resp := Mux(tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR),
                          OcpResp.DVA, OcpResp.NULL)

  io.perf.hit := tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR)
  io.perf.miss := !tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR)

  // Register to delay response
  val slaveReg = Reg(chiselTypeOf(io.master.S))

  // Register for write-back address
  val memWrAddrReg = Reg(UInt(addrWidth.W))

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, lineBits),
                         Fill(lineBits, 0.B))
  io.slave.M.Data := 0.U
  io.slave.M.DataValid := 0.U
  io.slave.M.DataByteEn := 0.U

  fillReg := false.B
  doingRead := doingRead 
  selWrBack := false.B


  // Start handling a miss
  when(!tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR)) {
    tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    missIndexReg := masterReg.Addr(lineBits-1, 2).asUInt
    memWrAddrReg := Cat(tag, masterReg.Addr(addrBits + 1, lineBits), Fill(lineBits, 0.B))

    // start writing back if block is dirty
    when(dirty) {
      stateReg := write
      selWrBack := true.B
      rdAddrCntReg := rdAddrCntReg + 1.U
    }
    // or skip writeback if block is not dirty
    .otherwise {
      io.slave.M.Cmd := OcpCmd.RD
      when(io.slave.S.CmdAccept === 1.U) {
        stateReg := fill
      }
      .otherwise {
        stateReg := hold
      }
    }
    masterReg.Addr := masterReg.Addr

    when(masterReg.Cmd === OcpCmd.WR) {
      doingRead := false.B
      coreWrDataReg := masterReg.Data
      coreByteEnReg := masterReg.ByteEn
      dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    }
    when(masterReg.Cmd === OcpCmd.RD) {
      doingRead := true.B
      dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := false.B
    }
  }
  tagMem.io <= (!tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR),
                masterReg.Addr(addrBits + 1, lineBits),
                masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  // writeback state
  when(stateReg === write) {
    selWrBack := true.B
    io.slave.M.Addr := Cat(memWrAddrReg(addrWidth-1, burstAddrBits+byteAddrBits),
                           Fill(burstAddrBits+byteAddrBits, 0.B))
    when(burstCntReg === 0.U) {
      io.slave.M.Cmd := OcpCmd.WR
    }
    io.slave.M.DataValid := 1.U
    io.slave.M.Data := rdData
    io.slave.M.DataByteEn := "b1111".U(4.W)
    when(io.slave.S.DataAccept === 1.U) {
      burstCntReg := burstCntReg + 1.U
      rdAddrCntReg := rdAddrCntReg + 1.U
    }
    when(burstCntReg === (burstLength - 1).U) {
      when(io.slave.S.Resp === OcpResp.DVA) {
        stateReg := hold
      }
      when(io.slave.S.Resp === OcpResp.ERR) {
        slaveReg := io.slave.S
        stateReg := respond
      }
      when(io.slave.S.Resp === OcpResp.NULL) {
        stateReg := writeWaitForResp
      }
      rdAddrCntReg := rdAddrCntReg // rdAddrCntReg is one step ahead of burstCnt, so don't update
    }
    masterReg.Addr := masterReg.Addr
  }

  // Slave responds to finish write transaction
  when(stateReg === writeWaitForResp) {
    stateReg := writeWaitForResp
    when(io.slave.S.Resp === OcpResp.DVA) {
      stateReg := hold
    }
    when(io.slave.S.Resp === OcpResp.ERR) {
      slaveReg := io.slave.S
      stateReg := respond
    }
    masterReg.Addr := masterReg.Addr
  }

  // Hold read command
  when(stateReg === hold) {
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === 1.U) {
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
        // insert write data from core on write-allocation
        when(!doingRead) {
          for (i <- 0 until DATA_WIDTH/BYTE_WIDTH) {
            comb(i) := Mux(coreByteEnReg(i) === 1.U,
                           coreWrDataReg(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i),
                           io.slave.S.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
          }
          wrDataReg := comb.reduceLeft((x,y) => y##x)
        }
      }
      when(burstCntReg === (lineSize/4-1).U) {
        stateReg := respond
      }
      burstCntReg := burstCntReg + 1.U
    }
    when(io.slave.S.Resp === OcpResp.ERR) {
      tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := false.B
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
    tagVMem.map(_ := false.B)
  }
}

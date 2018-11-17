/*
 * A direct-mapped cache
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._

import patmos.Constants._
import patmos.DataCachePerf
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class DirectMappedCacheWriteBack(size: Int, lineSize: Int) extends Module {
  val io = IO(new Bundle {
    val master = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, lineSize/4)
    val invalidate = Bool(INPUT)
    val perf = new DataCachePerf()
  })


  val addrBits = log2Up(size / BYTES_PER_WORD)
  val lineBits = log2Up(lineSize)
  val dataWidth = io.master.M.Data.getWidth()
  val burstLength = io.slave.burstLength
  val burstAddrBits = log2Up(burstLength)
  val byteAddrBits = log2Up(dataWidth/8)
  val addrWidth = io.master.M.Addr.getWidth()

  val doingRead = Reg(Bool()) // are we reading or writing
  val coreWrDataReg = Reg(Bits(width = DATA_WIDTH)) // register for WR cmd data
  val coreByteEnReg = Reg(Bits(width = DATA_WIDTH/BYTE_WIDTH)) // register for WR cmd byte enables

  // Temporary vector for combining written bytes with bytes from memory
  val comb = Vec.fill(DATA_WIDTH/BYTE_WIDTH) { Bits(width = BYTE_WIDTH) }
  for (i <- 0 until DATA_WIDTH/BYTE_WIDTH) { comb(i) := Bits(0) }

  val tagWidth = ADDR_WIDTH - addrBits - 2
  val tagCount = size / lineSize

  // Register signals from master
  val masterReg = Reg(io.master.M)
  masterReg := io.master.M

  // State machine for misses
  val idle :: write :: writeWaitForResp :: hold :: fill :: respond :: Nil = Enum(UInt(), 6)
  val stateReg = Reg(init = idle)

  val burstCntReg = Reg(init = UInt(0, lineBits-2))
  val missIndexReg = Reg(UInt(lineBits-2))

  // Generate memories
  val tagMem = MemBlock(tagCount, tagWidth)
  val tagVMem = Vec.fill(tagCount) { Reg(init = Bool(false)) }
  val dirtyMem = Vec.fill(tagCount) { Reg(init = Bool(false)) }
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val tag = tagMem.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tagV = Reg(next = tagVMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val dirty = Reg(next = dirtyMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagValid = tagV && tag === Cat(masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  val fillReg = Reg(Bool())

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
  // Update dirty bit when writing
  when(tagValid && (stmsk =/= Bits("b0000"))) {
    dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
    when(io.master.M.Addr(addrBits + 1, lineBits) === masterReg.Addr(addrBits + 1, lineBits)) {
      dirty := Bool(true);
    }
  }


  // Count register used to index reads for write-back
  val rdAddrCntReg = Reg(init = UInt(0, lineBits-2))
  // Read from cache
  val selWrBack = Bool()
  val rdAddr = Mux(selWrBack, Cat(masterReg.Addr(addrBits + 1, lineBits), rdAddrCntReg), io.master.M.Addr(addrBits + 1, 2)) // helper signal
  val rdData = mem.map(_(rdAddr)).reduceLeft((x,y) => y ## x)
  val rdDataReg = Reg(next = rdData)

  // Return data on a hit
  io.master.S.Data := rdData
  io.master.S.Resp := Mux(tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR),
                          OcpResp.DVA, OcpResp.NULL)

  // Register to delay response
  val slaveReg = Reg(io.master.S)

  // Register for write-back address
  val memWrAddrReg = Reg(Bits(width = addrWidth))

  // Default values
  io.slave.M.Cmd := OcpCmd.IDLE
  io.slave.M.Addr := Cat(masterReg.Addr(ADDR_WIDTH-1, lineBits),
                         Fill(lineBits, Bits(0)))
  io.slave.M.Data := Bits(0)
  io.slave.M.DataValid := Bits(0)
  io.slave.M.DataByteEn := Bits(0)

  fillReg := Bool(false)
  doingRead := doingRead 
  selWrBack := Bool(false)


  // Start handling a miss
  when(!tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR)) {
    tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
    missIndexReg := masterReg.Addr(lineBits-1, 2).toUInt
    memWrAddrReg := Cat(tag, masterReg.Addr(addrBits + 1, lineBits), Fill(lineBits, Bits(0)))

    // start writing back if block is dirty
    when(dirty) {
      stateReg := write
      selWrBack := Bool(true)
      rdAddrCntReg := rdAddrCntReg + UInt(1)
    }
    // or skip writeback if block is not dirty
    .otherwise {
      io.slave.M.Cmd := OcpCmd.RD
      when(io.slave.S.CmdAccept === Bits(1)) {
        stateReg := fill
      }
      .otherwise {
        stateReg := hold
      }
    }
    masterReg.Addr := masterReg.Addr

    when(masterReg.Cmd === OcpCmd.WR) {
      doingRead := Bool(false)
      coreWrDataReg := masterReg.Data
      coreByteEnReg := masterReg.ByteEn
      dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(true)
    }
    when(masterReg.Cmd === OcpCmd.RD) {
      doingRead := Bool(true)
      dirtyMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(false)
    }
  }
  tagMem.io <= (!tagValid && (masterReg.Cmd === OcpCmd.RD || masterReg.Cmd === OcpCmd.WR),
                masterReg.Addr(addrBits + 1, lineBits),
                masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  // writeback state
  when(stateReg === write) {
    selWrBack := Bool(true)
    io.slave.M.Addr := Cat(memWrAddrReg(addrWidth-1, burstAddrBits+byteAddrBits),
                           Fill(burstAddrBits+byteAddrBits, Bits(0)))
    when(burstCntReg === Bits(0)) {
      io.slave.M.Cmd := OcpCmd.WR
    }
    io.slave.M.DataValid := Bits(1)
    io.slave.M.Data := rdData
    io.slave.M.DataByteEn := Bits("b1111")
    when(io.slave.S.DataAccept === Bits(1)) {
      burstCntReg := burstCntReg + UInt(1)
      rdAddrCntReg := rdAddrCntReg + UInt(1)
    }
    when(burstCntReg === UInt(burstLength - 1)) {
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
        // insert write data from core on write-allocation
        when(!doingRead) {
          for (i <- 0 until DATA_WIDTH/BYTE_WIDTH) {
            comb(i) := Mux(coreByteEnReg(i) === Bits(1),
                           coreWrDataReg(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i),
                           io.slave.S.Data(BYTE_WIDTH*(i+1)-1, BYTE_WIDTH*i))
          }
          wrDataReg := comb.reduceLeft((x,y) => y##x)
        }
      }
      when(burstCntReg === UInt(lineSize/4-1)) {
        stateReg := respond
      }
      burstCntReg := burstCntReg + UInt(1)
    }
    when(io.slave.S.Resp === OcpResp.ERR) {
      tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := Bool(false)
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
    tagVMem.map(_ := Bool(false))
  }
}

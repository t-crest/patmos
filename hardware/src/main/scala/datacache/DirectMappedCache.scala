/*
 * A direct-mapped cache
 *
 * Authors: Martin Schoeberl (martin@jopdesign.com)
 *          Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._
import chisel3.VecInit

import patmos.Constants._
import patmos.DataCachePerf
import patmos.MemBlock
import patmos.MemBlockIO

import ocp._

class DirectMappedCache(size: Int, lineSize: Int) extends DCacheType(lineSize/4) {

  io.perf.hit := false.B
  io.perf.miss := false.B

  val addrBits = log2Up(size / BYTES_PER_WORD)
  val lineBits = log2Up(lineSize)

  val tagWidth = ADDR_WIDTH - addrBits - 2
  val tagCount = size / lineSize

  // Register signals from master
  val masterReg = Reg(io.master.M)
  masterReg := io.master.M

  // Generate memories
  val tagMem = MemBlock(tagCount, tagWidth)
  val tagVMem = RegInit(VecInit(Seq.fill(tagCount)(false.B)))
  val mem = new Array[MemBlockIO](BYTES_PER_WORD)
  for (i <- 0 until BYTES_PER_WORD) {
    mem(i) = MemBlock(size / BYTES_PER_WORD, BYTE_WIDTH).io
  }

  val tag = tagMem.io(io.master.M.Addr(addrBits + 1, lineBits))
  val tagV = Reg(next = tagVMem(io.master.M.Addr(addrBits + 1, lineBits)))
  val tagValid = tagV && tag === Cat(masterReg.Addr(ADDR_WIDTH-1, addrBits+2))

  val fillReg = Reg(Bool())

  val wrAddrReg = Reg(Bits(width = addrBits))
  val wrDataReg = Reg(Bits(width = DATA_WIDTH))

  wrAddrReg := io.master.M.Addr(addrBits + 1, 2)
  wrDataReg := io.master.M.Data

  // Write to cache; store only updates what's already there
  val stmsk = Mux(masterReg.Cmd === OcpCmd.WR, masterReg.ByteEn,  "b0000".U(4.W))
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
  when(tagValid && masterReg.Cmd === OcpCmd.RD) {
    io.perf.hit := true.B
  }

  // Start handling a miss
  when(!tagValid && masterReg.Cmd === OcpCmd.RD) {
    tagVMem(masterReg.Addr(addrBits + 1, lineBits)) := true.B
    missIndexReg := masterReg.Addr(lineBits-1, 2).asUInt
    io.slave.M.Cmd := OcpCmd.RD
    when(io.slave.S.CmdAccept === Bits(1)) {
      stateReg := fill
    }
    .otherwise {
      stateReg := hold
    }
    masterReg.Addr := masterReg.Addr
    io.perf.miss := true.B
  }
  tagMem.io <= (!tagValid && masterReg.Cmd === OcpCmd.RD,
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

object DirectMappedCache extends App {
  (new chisel3.stage.ChiselStage).emitVerilog(new DirectMappedCache(4096, 32))
}

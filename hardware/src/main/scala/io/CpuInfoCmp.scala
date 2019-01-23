/*
 * "I/O" module to access information about the CPU
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *          Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */


package io

import Chisel._

import patmos.Constants._
import util.Utility

import ocp._

class CpuInfoCmp(datFile: String, nr: Int, cnt: Int) extends CoreDevice() {

  val masterReg = Reg(next = io.ocp.M)

  // Default response
  val resp = Bits()
  val data = Bits(width = DATA_WIDTH)
  resp := OcpResp.NULL
  data := Bits(0)

  // Ignore writes
  when(masterReg.Cmd === OcpCmd.WR) {
    resp := OcpResp.DVA
  }

  // The ROM for booting
  val rom = Utility.readBin(datFile, DATA_WIDTH)
  val romData = rom(masterReg.Addr(log2Up(rom.length)+1, 2))

  // Read information
  switch(masterReg.Addr(5,2)) {
    is(Bits("b0000")) { data := Bits(nr) }
    is(Bits("b0001")) { data := Bits(CLOCK_FREQ) }
    is(Bits("b0010")) { data := Bits(cnt) }
    is(Bits("b0011")) { data := Bits(PIPE_COUNT) }
    // ExtMEM
    // Size (32 bit)
    is(Bits("b0100")) { data := Bits(EXTMEM_SIZE) } 
    // Burst length (8 bit ) & Write combine (8 bit)
    is(Bits("b0101")) { data := Bits(BURST_LENGTH, width = 8) ## Bits(0, width = 7) ## Bool(WRITE_COMBINE) }
    // ICache
    // Size (32 bit)
    is(Bits("b0110")) { data := Bits(ICACHE_SIZE) }
    // Type (8 bit) & Replacement policy (8 bit) & Associativity (16 bit)
    is(Bits("b0111")) { data := Bits(iCacheType2Int(ICACHE_TYPE), width = 8) ## Bits(cacheRepl2Int(ICACHE_REPL), width = 8) ## Bits(ICACHE_ASSOC, width = 16) }
    // DCache
    // Size (32 bit)
    is(Bits("b1000")) { data := Bits(DCACHE_SIZE) }
    // Type (8 bit) & Replacement policy (8 bit) & Associativity (16 bit)
    is(Bits("b1001")) { data := Bits(0, width = 7) ## Bool(DCACHE_WRITETHROUGH) ## Bits(cacheRepl2Int(DCACHE_REPL), width = 8) ## Bits(DCACHE_ASSOC, width = 16) }
    // SCache
    // Size (32 bit)
    is(Bits("b1010")) { data := Bits(SCACHE_SIZE) }
    // Reserved
    is(Bits("b1011")) { data := Bits("b0") }
    // ISPM
    // Size (32 bit)
    is(Bits("b1100")) { data := Bits(ISPM_SIZE) }
    // DSPM
    // Size (32 bit)
    is(Bits("b1101")) { data := Bits(DSPM_SIZE) }
  }
  when (masterReg.Addr(15) === Bits("b1")) {
    data := romData
  }

  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data
}

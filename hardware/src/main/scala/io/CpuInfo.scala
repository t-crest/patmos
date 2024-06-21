/*
 * "I/O" module to access information about the CPU
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *          Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */


package io

import util.Utility

import chisel3._
import chisel3.util._

import patmos.Constants._

import ocp._

class CpuInfo(datFile: String, cpucnt: Int) extends CoreDevice() {

  override val io = IO(new CoreDeviceIO() {
    val nr = Input(UInt(log2Up(cpucnt).W))
    val cnt = Input(UInt((log2Floor(cpucnt) + 1).W))
  })

  val masterReg = RegNext(next = io.ocp.M)

  // Default response
  val resp = Wire(Bits())
  val data = Wire(UInt(DATA_WIDTH.W))
  resp := OcpResp.NULL
  data := 0.U

  // Ignore writes
  when(masterReg.Cmd === OcpCmd.WR) {
    resp := OcpResp.DVA
  }

  // The ROM for booting
  val rom = Utility.readBin(datFile, DATA_WIDTH)
  val romData = rom(masterReg.Addr(log2Up(rom.length)+1, 2))

  // Read information
  switch(masterReg.Addr(5,2)) {
    is("b0000".U) { data := io.nr }
    is("b0001".U) { data := CLOCK_FREQ.U }
    is("b0010".U) { data := io.cnt }
    is("b0011".U) { data := PIPE_COUNT.U }
    // ExtMEM
    // Size (32 bit)
    is("b0100".U) { data := EXTMEM_SIZE.U } 
    // Burst length (8 bit ) & Write combine (8 bit)
    is("b0101".U) { data := BURST_LENGTH.U(8.W) ## 0.U(7.W) ## WRITE_COMBINE.B }
    // ICache
    // Size (32 bit)
    is("b0110".U) { data := ICACHE_SIZE.U }
    // Type (8 bit) & Replacement policy (8 bit) & Associativity (16 bit)
    is("b0111".U) { data := iCacheType2Int(ICACHE_TYPE).U(8.W) ## cacheRepl2Int(ICACHE_REPL).U(8.W) ## ICACHE_ASSOC.U(16.W) }
    // DCache
    // Size (32 bit)
    is("b1000".U) { data := DCACHE_SIZE.U }
    // Type (8 bit) & Replacement policy (8 bit) & Associativity (16 bit)
    is("b1001".U) { data := 0.U(7.W) ## DCACHE_WRITETHROUGH.B ## cacheRepl2Int(DCACHE_REPL).U(8.W) ## DCACHE_ASSOC.U(16.W) }
    // SCache
    // Size (32 bit)
    is("b1010".U) { data := SCACHE_SIZE.U }
    // Reserved
    is("b1011".U) { data := "b0".U }
    // ISPM
    // Size (32 bit)
    is("b1100".U) { data := ISPM_SIZE.U }
    // DSPM
    // Size (32 bit)
    is("b1101".U) { data := DSPM_SIZE.U }
  }
  when (masterReg.Addr(15) === "b1".U) {
    data := romData
  }

  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data
}

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
 * "I/O" module to access information about the CPU
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */


package io

import Chisel._
import Node._

import patmos.Constants._

import ocp._

object CpuInfo extends DeviceObject {

  def init(params: Map[String, String]) = { }

  def create(params: Map[String, String]) : CpuInfo = {
    Module(new CpuInfo())
  }

  trait Pins {
    val cpuInfoPins = new Bundle() {
      val id = Bits(INPUT, DATA_WIDTH)
      val cnt = Bits(INPUT, DATA_WIDTH)
    }
  }
}

class CpuInfo() extends CoreDevice() {

  override val io = new CoreDeviceIO() with CpuInfo.Pins

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

  // Read information
  switch(masterReg.Addr(5,2)) {
    is(Bits("b0000")) { data := io.cpuInfoPins.id }
    is(Bits("b0001")) { data := Bits(CLOCK_FREQ) }
    is(Bits("b0010")) { data := io.cpuInfoPins.cnt }
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
    is(Bits("b1001")) { data := Bits(ICACHE_TYPE_INT, width = 8) ## Bits(ICACHE_REPL_TYPE, width = 8) ## Bits(ICACHE_ASSOC, width = 16) }
    // DCache
    // Size (32 bit)
    is(Bits("b1000")) { data := Bits(DCACHE_SIZE) }
    // Type (8 bit) & Replacement policy (8 bit) & Associativity (16 bit)
    is(Bits("b1001")) { data := Bits(0, width = 7) ## Bool(DCACHE_WRITETHROUGH) ## Bits(DCACHE_REPL_TYPE, width = 8) ## Bits(DCACHE_ASSOC, width = 16) }
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
    // BootSPM
    // Size (32 bit)
    is(Bits("b1110")) { data := Bits(BOOTSPM_SIZE) }
  }
  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data
}

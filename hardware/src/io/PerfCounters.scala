/*
   Copyright 2015 Technical University of Denmark, DTU Compute.
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
 * "I/O" module to access performance counters
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */


package io

import Chisel._
import Node._

import patmos.Constants._
import patmos.PerfCounterIO

import ocp._

object PerfCounters extends DeviceObject {

  def init(params: Map[String, String]) = { }

  def create(params: Map[String, String]) : PerfCounters = {
    Module(new PerfCounters())
  }

  trait Pins {
  }
}

class PerfCounters() extends CoreDevice() {

  override val io = new CoreDeviceIO() with PerfCounters.Pins {
    override val internalPort = new Bundle() {
      val perf = new PerfCounterIO().asInput
    }
  }

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

  val PERFCOUNTER_COUNT = 12

  val inputVec = Vec.fill(PERFCOUNTER_COUNT) { Reg(Bool()) }
  inputVec(0) := io.internalPort.perf.mc.hit
  inputVec(1) := io.internalPort.perf.mc.miss
  inputVec(2) := io.internalPort.perf.dc.hit
  inputVec(3) := io.internalPort.perf.dc.miss
  inputVec(4) := io.internalPort.perf.sc.spill
  inputVec(5) := io.internalPort.perf.sc.fill
  inputVec(6) := io.internalPort.perf.wc.hit
  inputVec(7) := io.internalPort.perf.wc.miss
  inputVec(8) := io.internalPort.perf.mem.read
  inputVec(9) := io.internalPort.perf.mem.write
  inputVec(10) := io.internalPort.perf.rsc.hit
  inputVec(11) := io.internalPort.perf.rsc.miss

  val counterVec = Vec.fill(PERFCOUNTER_COUNT) { Reg(init = UInt(0, width = DATA_WIDTH)) }
  for (i <- 0 until PERFCOUNTER_COUNT) {
    when (inputVec(i)) {
      counterVec(i) := counterVec(i) + UInt(1)
    }
  }

  // Read information
  data := counterVec(masterReg.Addr(5,2))
  when(masterReg.Cmd === OcpCmd.RD) {
    resp := OcpResp.DVA
  }

  // Connections to master
  io.ocp.S.Resp := resp
  io.ocp.S.Data := data
}

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
 * The data cache unit for Patmos
 *
 * Author: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

package datacache

import Chisel._
import Node._


import stackcache._
import patmos._
import patmos.Constants._

import ocp._

class DataCache extends Module {
  val io = new Bundle {
    val master = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
    val scIO = new StackCacheIO()
    val invalDCache = Bool(INPUT)
    val dcPerf = new DataCachePerf()
    val scPerf = new StackCachePerf()
    val wcPerf = new WriteCombinePerf()
  }

  // Register selects
  val selDC = Bool(DCACHE_SIZE > 0) && io.master.M.AddrSpace === OcpCache.DATA_CACHE
  val selDCReg = Reg(Bool())
  val selSC = io.master.M.AddrSpace === OcpCache.STACK_CACHE
  val selSCReg = Reg(Bool())
  when(io.master.M.Cmd != OcpCmd.IDLE) {
    selDCReg := selDC
    selSCReg := selSC
  }

  // Instantiate direct-mapped cache for regular data cache
  val dm = 
    if (DCACHE_SIZE <= 0)
      Module(new NullCache())
    else if (DCACHE_ASSOC == 1)
      Module(new DirectMappedCache(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD))
    else if (DCACHE_ASSOC == 2 && DCACHE_REPL == "lru")
      Module(new TwoWaySetAssociativeCache(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD))
    else {
      ChiselError.error("Unsupported data cache configuration: "+
                        "associativity "+DCACHE_ASSOC+
                        " with replacement policy \""+DCACHE_REPL+"\"")
      Module(new NullCache()) // return at least a dummy cache
    }

  dm.io.master.M := io.master.M
  dm.io.master.M.Cmd := Mux(selDC ||
                            (Bool(DCACHE_SIZE > 0) && io.master.M.Cmd === OcpCmd.WR),
                            io.master.M.Cmd, OcpCmd.IDLE)
  dm.io.invalidate := io.invalDCache
  val dmS = dm.io.master.S

  // Instantiate stack cache
  val sc = Module(if (SCACHE_SIZE <= 0) new NullStackCache() else new StackCache())
  io.scIO <> sc.io

  // connect the stack cache to the Patmos core
  sc.io.fromCPU.M := io.master.M
  sc.io.fromCPU.M.Cmd := Mux(selSC, io.master.M.Cmd, OcpCmd.IDLE)
  val scS = sc.io.fromCPU.S

  // Instantiate bridge for bypasses and writes
  val bp = Module(new NullCache())
  bp.io.master.M := io.master.M
  bp.io.master.M.Cmd := Mux(!selDC && !selSC, io.master.M.Cmd, OcpCmd.IDLE)
  val bpS = bp.io.master.S

  // Join read requests
  val burstReadBus1 = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val burstReadJoin1 = new OcpBurstJoin(dm.io.slave, bp.io.slave, burstReadBus1.io.slave)

  val burstReadBus2 = Module(new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH))
  val burstReadJoin2 = new OcpBurstJoin(sc.io.toMemory, burstReadBus1.io.master, burstReadBus2.io.slave)

  // Combine writes
  val wc = Module(if (WRITE_COMBINE) new WriteCombineBuffer() else new WriteNoBuffer())
  wc.io.readMaster <> burstReadBus2.io.master
  wc.io.writeMaster.M := io.master.M
  wc.io.writeMaster.M.Cmd := Mux(!selSC, io.master.M.Cmd, OcpCmd.IDLE)
  val wcWriteS = wc.io.writeMaster.S
  io.slave <> wc.io.slave

  // Pass data to pipeline
  io.master.S.Data := bpS.Data
  when(selDCReg) { io.master.S.Data := dmS.Data }
  when(selSCReg) { io.master.S.Data := scS.Data }

  // Merge responses
  io.master.S.Resp := dmS.Resp | scS.Resp | bpS.Resp | wcWriteS.Resp

  // Pass on performance counters
  io.dcPerf <> dm.io.perf
  io.scPerf <> sc.io.perf
  io.wcPerf <> wc.io.perf
}

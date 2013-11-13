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

import patmos.Constants._

import ocp._

class DataCache extends Component {
  val io = new Bundle {
	val master = new OcpCacheSlavePort(ADDR_WIDTH, DATA_WIDTH)
	val slave = new OcpBurstMasterPort(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  }

  // Register selects
  val selDC = io.master.M.AddrSpace === OcpCache.DATA_CACHE
  val selDCReg = Reg(resetVal = Bool(false))
  when(io.master.M.Cmd != OcpCmd.IDLE) {
	selDCReg := selDC
  }

  // Instantiate direct-mapped cache for regular data cache
  val dm = new DirectMappedCache(DCACHE_SIZE, BURST_LENGTH*BYTES_PER_WORD)
  dm.io.master.M := io.master.M
  dm.io.master.M.Cmd := Mux(selDC, io.master.M.Cmd, OcpCmd.IDLE)
  val dmS = dm.io.master.S

  // Instantiate bridge for bypasses and writes
  val bp = new OcpCacheBus(ADDR_WIDTH, DATA_WIDTH)
  val bpBurst = new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val bpBridge = new OcpBurstBridge(bp.io.master, bpBurst.io.slave)
  bp.io.slave.M := io.master.M
  bp.io.slave.M.Cmd := Mux(!selDC || io.master.M.Cmd === OcpCmd.WR,
						   io.master.M.Cmd, OcpCmd.IDLE)
  val bpS = bp.io.slave.S

  // Join requests
  val burstBus = new OcpBurstBus(ADDR_WIDTH, DATA_WIDTH, BURST_LENGTH)
  val burstJoin = new OcpBurstJoin(dm.io.slave, bpBurst.io.master, burstBus.io.slave)
  io.slave <> burstBus.io.master

  // Pass data to pipeline
  io.master.S.Data := bpS.Data
  when(selDCReg) { io.master.S.Data := dmS.Data }

  // Merge responses
  io.master.S.Resp := dmS.Resp | bpS.Resp
}

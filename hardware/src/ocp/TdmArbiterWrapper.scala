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
 * Wrapper for NodeTdmArbiter 
 *
 * Author: Martin Schoeberl (martin@jopdesign.com) David Chong (davidchong99@gmail.com)
 *
 */

/*
 * Switching between different arbiters in Aegean:
 * Change 'Arbiter' in aegeanGen.py and in aegeanCode.py to 'TdmArbiterWrapper'.
 * Three places all together.
 */

package ocp

import Chisel._
import Node._
import scala.math._

import scala.collection.mutable.HashMap

class TdmArbiterWrapper(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = new Bundle {
    val master = Vec.fill(cnt){new OcpBurstSlavePort(addrWidth, dataWidth, burstLen)} 
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  }
  
  val memMux = Module(new MemMuxIntf(cnt, addrWidth, dataWidth, burstLen))
  
  for (i <- 0 until cnt) {
    val nodeID = UInt(i, width=6)
    val arb = Module(new ocp.NodeTdmArbiter(cnt, addrWidth, dataWidth, burstLen, 16))
    arb.io.master <> io.master(i)
    arb.io.node := nodeID
    
    memMux.io.master(i) <> arb.io.slave
  }
  
  io.slave <> memMux.io.slave
}

object TdmArbiterWrapperMain {
  def main(args: Array[String]): Unit = {

    val chiselArgs = args.slice(4, args.length)
    val cnt = args(0)
    val addrWidth = args(1)
    val dataWidth = args(2)
    val burstLen = args(3)

    chiselMain(chiselArgs, () => Module(new TdmArbiterWrapper(cnt.toInt,addrWidth.toInt,dataWidth.toInt,burstLen.toInt)))
  }
}


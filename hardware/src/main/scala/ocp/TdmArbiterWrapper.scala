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

import chisel3._

class TdmArbiterWrapper(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends ArbiterType(cnt, dataWidth, dataWidth, burstLen) {

  val memMux = Module(new MemMuxIntf(cnt, addrWidth, dataWidth, burstLen))
  
  for (i <- 0 until cnt) {
    val nodeID = i.U(6.W)
    val arb = Module(new ocp.NodeTdmArbiter(cnt, addrWidth, dataWidth, burstLen, 16))
    arb.io.master.M <> io.master(i).M
    io.master(i).S <> arb.io.master.S
    arb.io.node := nodeID
    
    memMux.io.master(i).M <> arb.io.slave.M
    arb.io.slave.S <> memMux.io.master(i).S
  }
  
  io.slave <> memMux.io.slave
}


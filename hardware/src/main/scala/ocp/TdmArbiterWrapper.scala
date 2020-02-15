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

class TdmArbiterWrapper(cnt: Int, addrWidth : Int, dataWidth : Int, burstLen: Int) extends Module {
  // MS: I'm always confused from which direction the name shall be
  // probably the other way round...
  val io = IO(new Bundle {
    val master = Vec(cnt, new OcpBurstSlavePort(addrWidth, dataWidth, burstLen))
    val slave = new OcpBurstMasterPort(addrWidth, dataWidth, burstLen)
  })
  
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


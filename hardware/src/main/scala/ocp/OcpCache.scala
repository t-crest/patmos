/*
 * Definitions for OCP ports for split cache
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import Chisel._

object OcpCache {
  val STACK_CACHE = Bits("b00")
  val DATA_CACHE  = Bits("b10")
  val UNCACHED    = Bits("b11")
}

// Cache masters provide address space signal
class OcpCacheMasterSignals(addrWidth : Int, dataWidth : Int)
  extends OcpCoreMasterSignals(addrWidth, dataWidth) {
  val AddrSpace = Bits(width = 2)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpCacheMasterSignals(addrWidth, dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Master port
class OcpCacheMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpCacheMasterSignals(addrWidth, dataWidth).asOutput
  val S = new OcpSlaveSignals(dataWidth).asInput
}

// Slave port is reverse of master port
class OcpCacheSlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpCacheMasterSignals(addrWidth, dataWidth).asInput
  val S = new OcpSlaveSignals(dataWidth).asOutput
}

// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpCacheBus(addrWidth : Int, dataWidth : Int) extends Module {
  val io = new Bundle {
    val slave = new OcpCacheSlavePort(addrWidth, dataWidth)
    val master = new OcpCacheMasterPort(addrWidth, dataWidth)
  }
  io.master <> io.slave
}

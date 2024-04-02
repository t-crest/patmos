/*
 * Definitions for OCP ports for split cache
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import chisel3._
import chisel3.util._

object OcpCache {
  val STACK_CACHE = "b00".U(2.W)
  val DATA_CACHE  = "b10".U(2.W)
  val UNCACHED    = "b11".U(2.W)
}

// Cache masters provide address space signal
class OcpCacheMasterSignals(addrWidth : Int, dataWidth : Int)
  extends OcpCoreMasterSignals(addrWidth, dataWidth) {
  val AddrSpace = UInt(2.W)

  // This does not really clone, but Data.clone doesn't either
  override def cloneType() = {
    val res = new OcpCacheMasterSignals(addrWidth, dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Master port
class OcpCacheMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Output(new OcpCacheMasterSignals(addrWidth, dataWidth))
  val S = Input(new OcpSlaveSignals(dataWidth))
}

// Slave port is reverse of master port
class OcpCacheSlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Input(new OcpCacheMasterSignals(addrWidth, dataWidth))
  val S = Output(new OcpSlaveSignals(dataWidth))
}

// Provide a "bus" with a master port and a slave port to simplify plumbing
class OcpCacheBus(addrWidth : Int, dataWidth : Int) extends Module {
  val io = new Bundle {
    val slave = new OcpCacheSlavePort(addrWidth, dataWidth)
    val master = new OcpCacheMasterPort(addrWidth, dataWidth)
  }
  io.master <> io.slave
}

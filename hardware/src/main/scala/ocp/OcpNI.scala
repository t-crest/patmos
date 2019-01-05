/*
 * Definitions for OCP port for the network interface
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import Chisel._

// Cache masters provide address space signal
class OcpNISlaveSignals(dataWidth : Int)
  extends OcpIOSlaveSignals(dataWidth) {
  val Reset_n = Bits(width = 1)
  val Flag = Bits(width = 2)

  // This does not really clone, but Data.clone doesn't either
  override def clone() = {
    val res = new OcpNISlaveSignals(dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Master port
class OcpNIMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpIOMasterSignals(addrWidth, dataWidth).asOutput
  val S = new OcpNISlaveSignals(dataWidth).asInput
}

// Slave port is reverse of master port
class OcpNISlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = new OcpIOMasterSignals(addrWidth, dataWidth).asInput
  val S = new OcpNISlaveSignals(dataWidth).asOutput
}

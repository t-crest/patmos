/*
 * Definitions for OCP port for the network interface
 *
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 *
 */

package ocp

import chisel3._

// Cache masters provide address space signal
class OcpNISlaveSignals(dataWidth : Int)
  extends OcpIOSlaveSignals(dataWidth) {
  val Reset_n = UInt(1.W)
  val Flag = UInt(2.W)

  // This does not really clone, but Data.clone doesn't either
  override def cloneType() = {
    val res = new OcpNISlaveSignals(dataWidth)
    res.asInstanceOf[this.type]
  }
}

// Master port
class OcpNIMasterPort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Output(new OcpIOMasterSignals(addrWidth, dataWidth))
  val S = Input(new OcpNISlaveSignals(dataWidth))
}

// Slave port is reverse of master port
class OcpNISlavePort(addrWidth : Int, dataWidth : Int) extends Bundle() {
  // Clk is implicit in Chisel
  val M = Input(new OcpIOMasterSignals(addrWidth, dataWidth))
  val S = Output(new OcpNISlaveSignals(dataWidth))
}

/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package twoway

import Chisel._

class Port(size: Int) extends Bundle {
  val addr = UInt(log2Up(size).W).asInput
  val wrData = UInt(32.W).asInput
  val rdData = UInt(32.W).asOutput
  val wrEna = Bool().asInput
}

class DualPort(size: Int) extends Bundle {
  val rdAddr = UInt(log2Up(size).W).asInput
  val wrAddr = UInt(log2Up(size).W).asInput
  val wrData = UInt(32.W).asInput
  val rdData = UInt(32.W).asOutput
  val wrEna = Bool().asInput
}

class DualPortMemory(size: Int) extends Module {
  val io = new Bundle {
    val port = new DualPort(size)
  }

  val mem = Mem(UInt(32.W), size)

  io.port.rdData := mem(Reg(next = io.port.rdAddr, init = 0.U))
  when(io.port.wrEna) {
    mem(io.port.wrAddr) := io.port.wrData
  }

}

/**
 * This true dual port memory generates registers.
 */
class TrueDualPortMemory(size: Int) extends Module {
  val io = new Bundle {
    val portA = new Port(size)
    val portB = new Port(size)
  }

  val mem = Mem(UInt(32.W), size)

  val regAddrA = Reg(io.portA.addr, init = 0.U)
  when(io.portA.wrEna) {
    mem(io.portA.addr) := io.portA.wrData
  }.otherwise {
    regAddrA := io.portA.addr
  }
  io.portA.rdData := mem(regAddrA)

  // This does not generate a true dual-ported memory,
  // but a register based implementation
  val regAddrB = Reg(io.portB.addr, init = 0.U)
  when(io.portB.wrEna) {
    mem(io.portB.addr) := io.portB.wrData
  }.otherwise {
    regAddrB := io.portB.addr
  }
  io.portB.rdData := mem(regAddrB)
}

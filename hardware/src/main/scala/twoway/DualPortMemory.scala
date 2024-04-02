/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package twoway

import chisel3._
import chisel3.util._

class Port(size: Int) extends Bundle {
  val addr = Input(UInt(log2Up(size).W))
  val wrData = Input(UInt(32.W))
  val rdData = Output(UInt(32.W))
  val wrEna = Input(Bool())
}

class DualPort(size: Int) extends Bundle {
  val rdAddr = Input(UInt(log2Up(size).W))
  val wrAddr = Input(UInt(log2Up(size).W))
  val wrData = Input(UInt(32.W))
  val rdData = Output(UInt(32.W))
  val wrEna = Input(Bool())
}

class DualPortMemory(size: Int) extends Module {
  val io = new Bundle {
    val port = new DualPort(size)
  }

  val mem = Mem(size, UInt(32.W))

  io.port.rdData := mem(RegNext(next = io.port.rdAddr, init = 0.U))
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

  val mem = Mem(size, UInt(32.W))

  val regAddrA = RegNext(io.portA.addr, init = 0.U)
  when(io.portA.wrEna) {
    mem(io.portA.addr) := io.portA.wrData
  }.otherwise {
    regAddrA := io.portA.addr
  }
  io.portA.rdData := mem(regAddrA)

  // This does not generate a true dual-ported memory,
  // but a register based implementation
  val regAddrB = RegNext(io.portB.addr, init = 0.U)
  when(io.portB.wrEna) {
    mem(io.portB.addr) := io.portB.wrData
  }.otherwise {
    regAddrB := io.portB.addr
  }
  io.portB.rdData := mem(regAddrB)
}
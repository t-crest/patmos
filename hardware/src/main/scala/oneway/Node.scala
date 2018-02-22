/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import Chisel._

/**
 * One NoC node, connected to the router, containing the memory,
 * providing the NI machinery.
 */
class Node(n: Int, size: Int) extends Module {
  val io = new Bundle {
    val local = new Channel()
    val memPort = new DualPort(size)
  }

  // TODO: shall get TDM schedule length from the actual schedule
  // for 2x2 it is 5
  // for 3x3 it is 10
  // for 4x4 it is 19
  val scheduleLength = 5
  // TODO: get the valid slots from the schedule for counter increments
  // Counters will be different for send and receive
  
  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength-1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))
  // For quicker testing use only 4 words per connection
  // TODO: get this constant from somewhere
  val regAddrCounter = Reg(init = UInt(0, 2))
  when (end) {
    regAddrCounter := regAddrCounter + UInt(1)
  }

  // TODO: fix the addresses and the write enables
  val address = Cat(regTdmCounter, regAddrCounter)

  // Receive data from the NoC
  val memRx = Module(new DualPortMemory(size))
  
  memRx.io.port.wrAddr := address
  memRx.io.port.wrData := io.local.in.data
  memRx.io.port.wrEna := Bool(true)

  memRx.io.port.rdAddr := io.memPort.rdAddr
  io.memPort.rdData := memRx.io.port.rdData

  // Send data to the NoC
  val memTx = Module(new DualPortMemory(size))

  memTx.io.port.wrAddr := io.memPort.wrAddr
  memTx.io.port.wrData := io.memPort.wrData
  memTx.io.port.wrEna := io.memPort.wrEna

  memTx.io.port.rdAddr := address
  io.local.out.data := memTx.io.port.rdData
  
}

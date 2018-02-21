/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import Chisel._

/**
 * One NoC node, connected to the router, containing the memory,
 * providing the NI machinery, and adding a tester.
 * Provide some data in dout to get synthesize results.
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
  
  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength-1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))
  // For quicker testing use only 4 words per connection
  val regAddrCounter = Reg(init = UInt(0, 2))
  when (end) {
    regAddrCounter := regAddrCounter + UInt(1)
  }

  val address = Cat(regTdmCounter, regAddrCounter)

  // There will be probably re-shuffling of data anyway.
  // We would need a more elaborated address generation/translation.
  // Need to go through it on paper.
  // Or find a delay that works?
  // Or better insert and read (and address increment) only when it is a valid slot?
  // Simple have a tiny table that knows in which cycles a word
  // shall be read and the counters incremented, and the same on write.
  val regTdmCounter2 = Reg(init = UInt(1, log2Up(scheduleLength)))
  val end2 = regTdmCounter2 === UInt(scheduleLength-1)
  regTdmCounter2 := Mux(end, UInt(0), regTdmCounter2 + UInt(1))
  // For quicker testing use only 4 words per connection
  val regAddrCounter2 = Reg(init = UInt(1, 2))
  when (end2) {
    regAddrCounter2 := regAddrCounter2 + UInt(1)
  }

  val address2 = Cat(regTdmCounter2, regAddrCounter2)
  
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

  memTx.io.port.rdAddr := address2
  io.local.out.data := memTx.io.port.rdData
  
}

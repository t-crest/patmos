/*
 * Copyright: 2017, Technical University of Denmark, DTU Compute
 * Author: Martin Schoeberl (martin@jopdesign.com)
 * License: Simplified BSD License
 */

package oneway

import Chisel._
import s4noc._

/**
 * One NoC node, connected to the router, containing the memory,
 * providing the NI machinery.
 */
class Node(n: Int, size: Int) extends Module {
  val io = IO(new Bundle {
    val local = new Channel(UInt(width = size))
    val memPort = new DualPort(size)
  })

  val st = Schedule.getSchedule(n)
  val scheduleLength = st._1.length
  val validTab = Vec(st._2.map(Bool(_)))

  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength - 1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))

  val nrChannels = n * n - 1
  val blockAddrWidth = log2Down(size/nrChannels)
  println(blockAddrWidth.toShort)
  println("Memory block size: " + scala.math.pow(2, blockAddrWidth).toInt)

  // Send data to the NoC
  val regTxAddrUpper = RegInit(UInt(0, log2Up(scheduleLength)))
  val regTxAddrLower = RegInit(UInt(0, blockAddrWidth))

  val valid = validTab(regTdmCounter)
  
  //debug(valid) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  
  when(valid) {
    regTxAddrUpper := regTxAddrUpper + UInt(1)
    when(regTxAddrUpper === UInt(nrChannels - 1)) {
      regTxAddrUpper := 0.U
      regTxAddrLower := regTxAddrLower + UInt(1)
    }
  }

  val txAddr = Cat(regTxAddrUpper, regTxAddrLower)

  val memTx = Module(new DualPortMemory(size))
  memTx.io.port.wrAddr := io.memPort.wrAddr
  memTx.io.port.wrData := io.memPort.wrData
  memTx.io.port.wrEna := io.memPort.wrEna
  memTx.io.port.rdAddr := txAddr
  io.local.out.data := memTx.io.port.rdData
  // TDM schedule starts two cycles late for read data delay
  io.local.out.valid := RegNext(valid, init = false.B)

  // Receive data from the NoC  
  val regRxAddrUpper = RegInit(UInt(0, log2Up(scheduleLength)))
  val regRxAddrLower = RegInit(UInt(0, blockAddrWidth))

  val validRx = io.local.in.valid
  
  //debug(validRx) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  when(validRx) {
    regRxAddrUpper := regRxAddrUpper + UInt(1)
    when(regRxAddrUpper === UInt(nrChannels - 1)) {
      regRxAddrUpper := UInt(0)
      regRxAddrLower := regRxAddrLower + UInt(1)
    }
  }

  val rxAddr = Cat(regRxAddrUpper, regRxAddrLower)
  
  //debug(rxAddr) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  val memRx = Module(new DualPortMemory(size))
  memRx.io.port.wrAddr := rxAddr
  memRx.io.port.wrData := io.local.in.data
  memRx.io.port.wrEna := validRx
  memRx.io.port.rdAddr := io.memPort.rdAddr
  io.memPort.rdData := memRx.io.port.rdData

}

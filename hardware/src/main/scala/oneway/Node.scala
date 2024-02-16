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
    val local = new Channel(UInt(size.W))
    val memPort = new DualPort(size)
  })

  val st = Schedule.getSchedule(n)
  val scheduleLength = st._1.length
  val validTab = Vec(st._2.map(Bool(_)))

  val regTdmCounter = Reg(init = 0.U(log2Up(scheduleLength).W))
  val end = regTdmCounter === (scheduleLength - 1).U
  regTdmCounter := Mux(end, 0.U, regTdmCounter + 1.U)

  val nrChannels = n * n - 1
  val blockAddrWidth = log2Down(size/nrChannels)
  println(blockAddrWidth.toShort)
  println("Memory block size: " + scala.math.pow(2, blockAddrWidth).toInt)

  // Send data to the NoC
  val regTxAddrUpper = RegInit(0.U(log2Up(scheduleLength).W))
  val regTxAddrLower = RegInit(0.U(blockAddrWidth.W))

  val valid = validTab(regTdmCounter)
  
  //debug(valid) does nothing in chisel3 (no proning in frontend of chisel3 anyway)
  
  when(valid) {
    regTxAddrUpper := regTxAddrUpper + 1.U
    when(regTxAddrUpper === (nrChannels - 1).U) {
      regTxAddrUpper := 0.U
      regTxAddrLower := regTxAddrLower + 1.U
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
  val regRxAddrUpper = RegInit(0.U(log2Up(scheduleLength).W))
  val regRxAddrLower = RegInit(0.U(blockAddrWidth.W))

  val validRx = io.local.in.valid
  
  //debug(validRx) does nothing in chisel3 (no proning in frontend of chisel3 anyway)

  when(validRx) {
    regRxAddrUpper := regRxAddrUpper + 1.U
    when(regRxAddrUpper === (nrChannels - 1).U) {
      regRxAddrUpper := 0.U
      regRxAddrLower := regRxAddrLower + 1.U
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

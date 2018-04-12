/*
  Network interface for the S4NOC.

  Author: Martin Schoeberl (martin@jopdesign.com)
  license see LICENSE
 */
package s4noc

import Chisel._

// TODO: check if too large address ports are optimized away when not used
class CpuPort() extends Bundle {
  val addr = UInt(width = 32).asInput
  val rdData = UInt(width = 32).asOutput
  val wrData = UInt(width = 32).asInput
  val rd = Bool().asInput
  val wr = Bool().asInput
}

class NetworkInterface(dim: Int) extends Module {
  val io = new Bundle {
    val cpuPort = new CpuPort()
    val local = new Channel()
  }

  // TODO: too much repetition
  // Either provide the schedule as parameter
  // or simply read out the TDM counter from the router.
  // Why duplicating it? Does it matter?
  val st = Schedule.getSchedule(dim)
  val scheduleLength = st._1.length
  val validTab = Vec(st._2.map(Bool(_)))

  val regTdmCounter = Reg(init = UInt(0, log2Up(scheduleLength)))
  val end = regTdmCounter === UInt(scheduleLength - 1)
  regTdmCounter := Mux(end, UInt(0), regTdmCounter + UInt(1))
  // TDM schedule starts one cycles later for read data delay
  val regDelay = RegNext(regTdmCounter, init=UInt(0))


  val dataReg = RegInit(UInt(0, 32))
  val timeReg = RegInit(UInt(0, 6))
  when (io.cpuPort.wr) {
    dataReg := io.cpuPort.wrData
    timeReg := io.cpuPort.addr
  }

  io.local.out.data := dataReg
  io.local.out.valid := regDelay === timeReg

  io.cpuPort.rdData := io.local.in.data

}

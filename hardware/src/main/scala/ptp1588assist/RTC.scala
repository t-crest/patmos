package ptp1588assist

import Chisel._
import io.CoreDeviceIO
import ocp.{OcpCmd, OcpCoreSlavePort, OcpResp}
import patmos.Constants._

class RTC(clockFreq: Int, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt) extends Module {
  override val io = new Bundle() {
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val ptpTimestamp = UInt(OUTPUT, width = secondsWidth + nanoWidth)
    val periodIntr = Bool(OUTPUT)
  }

  // Constants
  println("IEEE 1588 RTC instantiated with clockFrequency @ " + clockFreq + " MHz.")

  // Register command
  val masterReg = Reg(next = io.ocp.M)

  // Registers
  val carryNs = Reg(init = UInt(0, width = nanoWidth))
  val carrySec = Reg(init = UInt(0, width = secondsWidth))
  val tickReg = Reg(init = false.B)
  val prescaleReg = Reg(init = UInt(0, width = 4))
  val secCntReg = Reg(init = UInt((clockFreq).U, width = nanoWidth))
  val stepReg = Reg(init = UInt(1, width = 16))
  val timeReg = Reg(init = UInt(0, width = secondsWidth + nanoWidth))
  val secTickReg = Reg(init = UInt(initialTime, width = secondsWidth))
  val nsTickReg = Reg(init = UInt(0, width = nanoWidth))
  val updateSecReg = Reg(init = false.B)
  val updateNsReg = Reg(init = false.B)
  val periodSelReg = Reg(init = UInt(secondsWidth + nanoWidth - 1, width = log2Up(secondsWidth + nanoWidth)))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL
  val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))

  // Clock engine
  tickReg := false.B
  when(prescaleReg === 3.U){
    prescaleReg := 0.U
    tickReg := true.B
  }.otherwise{
    prescaleReg := prescaleReg + 1.U
  }

  updateSecReg := false.B
  updateNsReg := false.B
  when(tickReg) {
    when(nsTickReg >= 1000000000.U - 50.U) {
      when(~updateSecReg) {
        secTickReg := secTickReg + 1.U
        nsTickReg := 0.U
      }.otherwise{
        secTickReg := timeReg(2*DATA_WIDTH-1, DATA_WIDTH) + 1.U
      }
    }.otherwise {
      when(~updateNsReg) {
        nsTickReg := nsTickReg + 50.U //TODO: Configurable timestep based on prescaler (i.e. 4-cycles 50ns at 80MHz)
      }.otherwise{
        nsTickReg := timeReg(DATA_WIDTH-1, 0) + 50.U
      }
    }
  }.otherwise{
    when(updateSecReg) {
      secTickReg := timeReg(2*DATA_WIDTH-1, DATA_WIDTH)
    }
    when(updateNsReg){
      nsTickReg := timeReg(DATA_WIDTH-1, 0)
    }
  }

  when(~updateNsReg && ~updateSecReg){
    timeReg := secTickReg ## nsTickReg
  }

  // Read response
  when(masterReg.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    when(masterReg.Addr(5,4) === Bits("b00")){
      when(masterReg.Addr(3,2) === Bits("b00")){
        dataReg := timeReg(DATA_WIDTH-1, 0)
      }.elsewhen(masterReg.Addr(3,2) ===Bits("b01")){
        dataReg := timeReg(2*DATA_WIDTH-1, DATA_WIDTH)
      }
    }.elsewhen(masterReg.Addr(5,4) === Bits("b01")){
      dataReg := periodSelReg
    }.elsewhen(masterReg.Addr(5,4) === Bits("b10")){
      dataReg := stepReg
    }
  }

  // Write response
  when(masterReg.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    when(masterReg.Addr(5, 4) === Bits("b00")) {
      when(masterReg.Addr(3, 2) === Bits("b00")) {
        updateNsReg := true.B
        timeReg(DATA_WIDTH-1, 0) := masterReg.Data
      }.elsewhen(masterReg.Addr(3, 2) === Bits("b01")) {
        updateSecReg := true.B
        timeReg(2*DATA_WIDTH-1, DATA_WIDTH) := masterReg.Data
      }
    }.elsewhen(masterReg.Addr(5, 4) === Bits("b01")) {
      periodSelReg := masterReg.Data
    }.elsewhen(masterReg.Addr(5, 4) === Bits("b10")) {
      stepReg := masterReg.Data(15, 0)
    }
  }

  // No interrupts by default
  io.periodIntr := timeReg(periodSelReg)

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataReg

  // Connections to PTP
  io.ptpTimestamp := timeReg

}



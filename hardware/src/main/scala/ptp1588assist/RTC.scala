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

  println("IEEE 1588 RTC instantiated with clockFrequency @ " + clockFreq + " MHz.")

  // Constants
  val timeStep = 50.S

  // Register command
  val masterReg = Reg(next = io.ocp.M)

  // Registers
  val tickReg = Reg(init = false.B)
  val prescaleReg = Reg(init = UInt(0, width = 4))
  val correctionStepReg = Reg(init = SInt(0, width = nanoWidth+1))
  val nsOffsetReg = Reg(init = SInt(0, width = nanoWidth+1))
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

  //Update Seconds
  when(updateSecReg) {
    secTickReg := timeReg(2 * DATA_WIDTH - 1, DATA_WIDTH)
  }

  //Update Nanoseconds
  when(updateNsReg){
   nsTickReg := timeReg(DATA_WIDTH-1, 0)
  }.elsewhen(tickReg){
   when(nsTickReg >= (1000000000.U - (timeStep + correctionStepReg).asUInt())){
     secTickReg := secTickReg + 1.U
     nsTickReg := 0.U
   }.otherwise{
     nsTickReg := nsTickReg + (timeStep + correctionStepReg).asUInt()
   }
  }

  //Smooth Adjustment
  when(tickReg) {
    when(nsOffsetReg =/= 0.S) {
      when(nsOffsetReg < 0.S) {
        correctionStepReg := 1.S //If negative offset then move faster (lacking behind)
      }.otherwise {
        correctionStepReg := -1.S //If positive offset then move slower (running forward)
      }
      nsOffsetReg := nsOffsetReg + correctionStepReg //Correct towards zero
    }.otherwise {
      correctionStepReg := 0.S
    }
  }

  //Register current time when it is not being updated
  when(masterReg.Cmd === OcpCmd.IDLE && ~updateNsReg && ~updateSecReg && ~tickReg){
    timeReg := secTickReg ## nsTickReg
  }

  updateSecReg := false.B
  updateNsReg := false.B

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
      nsOffsetReg := masterReg.Data(nanoWidth-1, 0).asSInt()
    }
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
      dataReg := nsOffsetReg
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



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
  val secInNanoConst = 1000000000
  val milliInNanoConst = 1000000
  val microInNanoConst = 1000
  val hundredNanoConst = 100
  val fiftyNanoConst = 50
  val prescaleConst = 2
  val timeStepConst = prescaleConst*(1000/(clockFreq/1000000))+1

  println("IEEE 1588 RTC instantiated with clockFrequency @ " + (clockFreq/1000000) + " MHz, prescaler:" + prescaleConst + " and timeStep=" + timeStepConst)

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
  when(prescaleReg === (prescaleConst-1).U){
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
  when(updateNsReg) {
    nsTickReg := timeReg(DATA_WIDTH - 1, 0)
  }.elsewhen(tickReg){
   when(nsTickReg >= (1000000000.U - (timeStepConst.S + correctionStepReg).asUInt())){
     secTickReg := secTickReg + 1.U
     nsTickReg := 0.U + (timeStepConst.S + correctionStepReg).asUInt()
   }.otherwise{
     nsTickReg := nsTickReg + (timeStepConst.S + correctionStepReg).asUInt()
     nsOffsetReg := nsOffsetReg + correctionStepReg // Always correct towards zero offset
   }
  }

  //Smooth Adjustment
  when(nsOffsetReg =/= 0.S) {
    when(nsOffsetReg < -milliInNanoConst.S || nsOffsetReg > milliInNanoConst.S) {   //under/over -1ms
      nsTickReg := (nsTickReg.toSInt() + nsOffsetReg).toUInt()
      nsOffsetReg := 0.S
    }.elsewhen(nsOffsetReg < -microInNanoConst.S && nsOffsetReg >= -milliInNanoConst.S) { //-1ms to 1us
      correctionStepReg := 25.S
    }.elsewhen(nsOffsetReg < -hundredNanoConst.S && nsOffsetReg >= -microInNanoConst.S) { //-1us to -100ns
      correctionStepReg := 10.S
    }.elsewhen(nsOffsetReg < -fiftyNanoConst.S && nsOffsetReg >= -hundredNanoConst.S) {   //-100ns to -50ns
      correctionStepReg := 2.S
    }.elsewhen(nsOffsetReg < 0.S && nsOffsetReg >= -fiftyNanoConst.S) {                 //-50ns to -1ns
      correctionStepReg := 1.S
    }.elsewhen(nsOffsetReg > 0.S && nsOffsetReg <= fiftyNanoConst.S){                   //1ns to 50ns
      correctionStepReg := -1.S
    }.elsewhen(nsOffsetReg > fiftyNanoConst.S && nsOffsetReg <= hundredNanoConst.S) {     //50ns to 100ns
      correctionStepReg := -5.S
    }.elsewhen(nsOffsetReg > hundredNanoConst.S && nsOffsetReg <= microInNanoConst.S) {   //100ns to 1us
      correctionStepReg := -10.S
    }.elsewhen(nsOffsetReg > microInNanoConst.S && nsOffsetReg <= milliInNanoConst.S) {   //1us to 1ms
      correctionStepReg := -20.S
    }.otherwise{
      correctionStepReg := 0.S
    }
  }.otherwise {
    correctionStepReg := 0.S
  }

  //Register current time when it is not being updated
  when(masterReg.Cmd === OcpCmd.IDLE && ~updateNsReg && ~updateSecReg && ~tickReg){
    timeReg := secTickReg ## nsTickReg
  }

  // Write response
  updateSecReg := false.B
  updateNsReg := false.B
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
      nsOffsetReg := masterReg.Data.toSInt()
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

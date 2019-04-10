package ptp1588assist

import scala.math._
import Chisel._
import io.CoreDeviceIO
import ocp.{OcpCmd, OcpCoreSlavePort, OcpResp}
import patmos.Constants._

class RTC(clockFreq: Int = CLOCK_FREQ, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt, timeStep: Int) extends Module {
  override val io = new Bundle() {
    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
    val ptpTimestamp = UInt(OUTPUT, width = secondsWidth + nanoWidth)
    val periodIntr = Bool(OUTPUT)
    val pps = Bool(OUTPUT)
  }

  // Constants
  val secInNanoConst = 1000000000
  val milliInNanoConst = 1000000
  val microInNanoConst = 1000
  val hundredNanoConst = 100
  val fiftyNanoConst = 50
  val prescalerWidth = log2Up(((timeStep.toFloat/secInNanoConst.toFloat)/(1f/clockFreq.toFloat)).toInt)
  val ppsNanoDurationConst = 2*10*microInNanoConst //should be between 10us to 500ms (http://digitalsubstation.com/en/2016/11/08/white-paper-on-implementing-ptp-in-substations/)

  println("IEEE 1588 RTC instantiated @ " + clockFreq/1000000 + " MHz with timeStep= "+timeStep+" and calculated prescalerWidth=" + prescalerWidth)

  // Register command
  val masterReg = Reg(next = io.ocp.M)

  // Registers
  val prescaleReg = Reg(init = UInt(0, width = prescalerWidth))
  val correctionStepReg = Reg(init = SInt(0, width = nanoWidth+1))
  val nsOffsetReg = Reg(init = SInt(0, width = nanoWidth+1))
  val timeReg = Reg(init = UInt(0, width = secondsWidth + nanoWidth))
  val secTickReg = Reg(init = UInt(initialTime, width = secondsWidth))
  val nsTickReg = Reg(init = UInt(0, width = nanoWidth))
  val updateSecReg = Reg(init = false.B)
  val updateNsReg = Reg(init = false.B)
  val periodSelReg = Reg(init = UInt(secondsWidth + nanoWidth - 1, width = log2Up(secondsWidth + nanoWidth)))
  val ppsReg = Reg(init = false.B)
  val ppsHoldCountReg = Reg(init = UInt(ppsNanoDurationConst, width = log2Up(ppsNanoDurationConst)))

  // Default response
  val respReg = Reg(init = OcpResp.NULL)
  respReg := OcpResp.NULL
  val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))

  // Clock engine
  prescaleReg := prescaleReg + 1.U
  val tickEnPulse = prescaleReg(prescalerWidth-1) & ~Reg(next = prescaleReg(prescalerWidth-1))

  //Update Seconds
  when(updateSecReg) {
    secTickReg := timeReg(2 * DATA_WIDTH - 1, DATA_WIDTH)
  }

  //PPS Engine
  when(~ppsReg){
    ppsReg := nsTickReg >= (secInNanoConst.U - (timeStep.S + correctionStepReg).asUInt())
  }.otherwise{
    when(ppsHoldCountReg > timeStep.U){
      ppsHoldCountReg := ppsHoldCountReg - timeStep.U
    }.otherwise {
      ppsHoldCountReg := ppsNanoDurationConst.U
      ppsReg := false.B
    }
  }

  //Update Seconds/Nanoseconds
  when(updateNsReg) {
    nsTickReg := timeReg(nanoWidth - 1, 0)
  }.elsewhen(tickEnPulse){
   when(nsTickReg >= (secInNanoConst.U - (timeStep.S + correctionStepReg).asUInt())){
    secTickReg := secTickReg + 1.U
    nsTickReg := 0.U + (timeStep.S + correctionStepReg).asUInt()
   }.otherwise{
    nsTickReg := nsTickReg + (timeStep.S + correctionStepReg).asUInt()
    nsOffsetReg := nsOffsetReg + correctionStepReg // Always correct towards zero offset on every tickPulse
   }
  }

  //Smooth Adjustment
  when(nsOffsetReg =/= 0.S) {
    when(nsOffsetReg < -microInNanoConst.S && nsOffsetReg >= -milliInNanoConst.S) { //-1ms to 1us
      correctionStepReg := timeStep.S
    }.elsewhen(nsOffsetReg < -hundredNanoConst.S && nsOffsetReg >= -microInNanoConst.S) { //-1us to -100ns
      correctionStepReg := (timeStep/2).S
    }.elsewhen(nsOffsetReg < -fiftyNanoConst.S && nsOffsetReg >= -hundredNanoConst.S) {   //-100ns to -50ns
      correctionStepReg := (timeStep/4).S
    }.elsewhen(nsOffsetReg < 0.S && nsOffsetReg >= -fiftyNanoConst.S) {                 //-50ns to -1ns
      correctionStepReg := 1.S
    }.elsewhen(nsOffsetReg > 0.S && nsOffsetReg <= fiftyNanoConst.S){                   //1ns to 50ns
      correctionStepReg := -1.S
    }.elsewhen(nsOffsetReg > fiftyNanoConst.S && nsOffsetReg <= hundredNanoConst.S) {     //50ns to 100ns
      correctionStepReg := (-timeStep/4).S
    }.elsewhen(nsOffsetReg > hundredNanoConst.S && nsOffsetReg <= microInNanoConst.S) {   //100ns to 1us
      correctionStepReg := (-timeStep/2).S
    }.elsewhen(nsOffsetReg > microInNanoConst.S && nsOffsetReg <= milliInNanoConst.S) {   //1us to 1ms
      correctionStepReg := -timeStep.S
    }.otherwise{
      correctionStepReg := 0.S
    }
  }.otherwise {
    correctionStepReg := 0.S
  }

  //Register current time when it is not being updated
  when(masterReg.Cmd =/= OcpCmd.WR){
    timeReg := Cat(secTickReg, nsTickReg)
  }

  // Write response
  updateSecReg := false.B
  updateNsReg := false.B
  when(masterReg.Cmd === OcpCmd.WR) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(5, 0)){
      is(Bits("h00")){
        updateNsReg := true.B
        timeReg(DATA_WIDTH-1, 0) := masterReg.Data
      }
      is(Bits("h04")){
        updateSecReg := true.B
        timeReg(2*DATA_WIDTH-1, DATA_WIDTH) := masterReg.Data
      }
      is(Bits("h10")){
        periodSelReg := masterReg.Data
      }
      is(Bits("h14")){
        periodSelReg := masterReg.Data 
      }
      is(Bits("h20")){
        nsOffsetReg := masterReg.Data.toSInt()
      }
    }
  }

  // Read response
  when(masterReg.Cmd === OcpCmd.RD) {
    respReg := OcpResp.DVA
    switch(masterReg.Addr(5, 0)){
      is(Bits("h00")){
        dataReg := timeReg(DATA_WIDTH-1, 0)
      }
      is(Bits("h04")){
        dataReg := timeReg(2*DATA_WIDTH-1, DATA_WIDTH)
      }
      is(Bits("h10")){
        dataReg := periodSelReg
      }
      is(Bits("h14")){
        dataReg := periodSelReg
      }
      is(Bits("h20")){
        dataReg := nsOffsetReg
      }
    }
  }

  // No interrupts by default
  io.periodIntr := timeReg(periodSelReg)

  // Connections to master
  io.ocp.S.Resp := respReg
  io.ocp.S.Data := dataReg

  // Connections to PTP
  io.ptpTimestamp := timeReg
  io.pps := ppsReg
}

object RTC {
  def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated/RTC"),
      () => Module(new RTC(clockFreq = 80000000, secondsWidth = 32, nanoWidth = 32, initialTime = 0x5ac385dcL, timeStep = 100)))
  }
}

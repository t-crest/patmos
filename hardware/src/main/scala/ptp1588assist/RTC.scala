package ptp1588assist

import scala.math._
import Chisel._
import io.CoreDeviceIO
import ocp.{OcpCmd, OcpCoreSlavePort, OcpResp}
import patmos.Constants._

 class RTC(clockFreq: Int = CLOCK_FREQ, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt, ppsDuration: Int = 10) extends Module {
   val io = new Bundle() {
     val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
     val ptpTimestamp = Output(UInt((secondsWidth + nanoWidth).W))
     val pps = Output(Bool())
   }

   // Constants
   val TIME_STEP = 25 //12.5 in 32.1 fixed point decimals
   val SEC_IN_NANO = (1000000000 << 1)
   val MILLI_IN_NANO = 1000000
   val MICRO_IN_NANO = 1000
   val HUNDRED_NANO = 100
   val FIFTY_NANO = 50
   val PPS_DURATION = 2 * max(ppsDuration, 10) * MICRO_IN_NANO //should be between 10us to 500ms enough for a microcontroller to sample it (http://digitalsubstation.com/en/2016/11/08/white-paper-on-implementing-ptp-in-substations/)

   println("--RTC instantiated @ " + clockFreq / 1000000 + " MHz with a PPS pulse width= " + ppsDuration + "us")

   // Register command
   val masterReg = RegNext(io.ocp.M)

   // Registers
   val correctionStepReg = Wire(0.S((nanoWidth + 1).W))
   val nsOffsetReg = RegInit(0.S((nanoWidth + 1).W))
   val timeReg = RegInit(0.U((secondsWidth + nanoWidth).W))
   val secTickReg = RegInit(initialTime.U(secondsWidth.W))
   val nsTickReg = RegInit(0.U((nanoWidth + 1).W)) //we need an extra bit for precision loss due to fixed point calculations
   val updateSecReg = RegInit(false.B)
   val updateNsReg = RegInit(false.B)
   val ppsReg = RegInit(false.B)
   val ppsHoldCountReg = RegInit(PPS_DURATION.U(log2Up(PPS_DURATION).W))

   // Default response
   val respReg = RegInit(OcpResp.NULL)
   respReg := OcpResp.NULL
   val dataReg = RegInit(0.U(DATA_WIDTH.W))

   //Clock Engine
   val nsIncrement = (TIME_STEP.S + correctionStepReg).asUInt()
   val nextNano = nsTickReg + nsIncrement
   //update seconds
   when(updateSecReg) {
     secTickReg := timeReg(3 * DATA_WIDTH - 1, DATA_WIDTH)
   }
   //update nanoseconds
   when(updateNsReg) {
     nsTickReg := timeReg(nanoWidth - 1, 0) ## 0.U(1.W)
   }.otherwise {
     nsTickReg := nextNano
     nsOffsetReg := nsOffsetReg + correctionStepReg // Always correct towards zero offset
     when(nsTickReg >= SEC_IN_NANO.U.asUInt()) {
       nsTickReg := nextNano - SEC_IN_NANO.U
       secTickReg := secTickReg + 1.U
     }
   }

   //PPS Engine
   when(!ppsReg) {
     ppsReg := nsTickReg >= (SEC_IN_NANO.U - nsIncrement)
   }.otherwise {
     when(ppsHoldCountReg > TIME_STEP.U) {
       ppsHoldCountReg := ppsHoldCountReg - TIME_STEP.U
     }.otherwise {
       ppsHoldCountReg := PPS_DURATION.U
       ppsReg := false.B
     }
   }

  //Smooth Adjustment
   when(nsOffsetReg < -MICRO_IN_NANO.S && nsOffsetReg >= -MILLI_IN_NANO.S) { //-1ms to 1us
     correctionStepReg := TIME_STEP.S
   }.elsewhen(nsOffsetReg < -HUNDRED_NANO.S && nsOffsetReg >= -MICRO_IN_NANO.S) { //-1us to -100ns
     correctionStepReg := (TIME_STEP / 2).S
   }.elsewhen(nsOffsetReg < -FIFTY_NANO.S && nsOffsetReg >= -HUNDRED_NANO.S) { //-100ns to -50ns
     correctionStepReg := (TIME_STEP / 4).S
   }.elsewhen(nsOffsetReg < 0.S && nsOffsetReg >= -FIFTY_NANO.S) { //-50ns to -1ns
     correctionStepReg := 1.S
   }.elsewhen(nsOffsetReg > 0.S && nsOffsetReg <= FIFTY_NANO.S) { //1ns to 50ns
     correctionStepReg := -1.S
   }.elsewhen(nsOffsetReg > FIFTY_NANO.S && nsOffsetReg <= HUNDRED_NANO.S) { //50ns to 100ns
     correctionStepReg := (-TIME_STEP / 4).S
   }.elsewhen(nsOffsetReg > HUNDRED_NANO.S && nsOffsetReg <= MICRO_IN_NANO.S) { //100ns to 1us
     correctionStepReg := (-TIME_STEP / 2).S
   }.elsewhen(nsOffsetReg > MICRO_IN_NANO.S && nsOffsetReg <= MILLI_IN_NANO.S) { //1us to 1ms
     correctionStepReg := -TIME_STEP.S
   }.otherwise {
     correctionStepReg := 0.S
   }

   //Register current time when it is not being updated
   when(masterReg.Cmd =/= OcpCmd.WR) {
     timeReg := Cat(secTickReg, nsTickReg(32, 1))
   }

   // Write response
   updateSecReg := false.B
   updateNsReg := false.B
   when(masterReg.Cmd === OcpCmd.WR) {
     respReg := OcpResp.DVA
     switch(masterReg.Addr(5, 0)) {
       is(Bits("h00")) {
         updateNsReg := true.B
         timeReg := Cat(secTickReg, masterReg.Data(DATA_WIDTH-1,0))
       }
       is(Bits("h04")) {
         updateSecReg := true.B
         timeReg := Cat(Cat( timeReg(3*DATA_WIDTH - 1, 2*DATA_WIDTH), masterReg.Data(DATA_WIDTH-1,0)), timeReg(DATA_WIDTH - 1, 0))
       }
       is(Bits("h08")) {
         updateSecReg := true.B
         timeReg := Cat(masterReg.Data(DATA_WIDTH-1,0), timeReg(2*DATA_WIDTH - 1, 0))
       }
       is(Bits("h20")) {
         nsOffsetReg := masterReg.Data.asSInt()
       }
     }
   }

   // Read response
   when(masterReg.Cmd === OcpCmd.RD) {
     respReg := OcpResp.DVA
     switch(masterReg.Addr(5, 0)) {
       is(Bits("h00")) {
         dataReg := timeReg(DATA_WIDTH - 1, 0)
       }
       is(Bits("h04")) {
         dataReg := timeReg(2 * DATA_WIDTH - 1, DATA_WIDTH)
       }
       is(Bits("h08")) {
         dataReg := timeReg(3 * DATA_WIDTH - 1, 2* DATA_WIDTH)
       }
       is(Bits("h20")) {
         dataReg := nsOffsetReg(32, 1)
       }
     }
   }

   // Connections to master
   io.ocp.S.Resp := respReg
   io.ocp.S.Data := dataReg

   // Connections to PTP
   io.ptpTimestamp := timeReg
   io.pps := ppsReg
 }

//class RTC(clockFreq: Int = CLOCK_FREQ, secondsWidth: Int = 32, nanoWidth: Int = 32, initialTime: BigInt, ppsDuration: Int = 10) extends Module {
//  override val io = new Bundle() {
//    val ocp = new OcpCoreSlavePort(ADDR_WIDTH, DATA_WIDTH)
//    val ptpTimestamp = Output(UInt((secondsWidth + nanoWidth).W))
//    val periodIntr = Output(Bool())
//    val pps = Output(Bool())
//  }
//
//  // Constants
//  val TIME_STEP = 25
//  val SEC_IN_NANO = (1000000000)
//  val MILLI_IN_NANO = 1000000
//  val MICRO_IN_NANO = 1000
//  val HUNDRED_NANO = 100
//  val FIFTY_NANO = 50
//  val PPS_DURATION = 2 * max(ppsDuration, 10) * MICRO_IN_NANO //should be between 10us to 500ms enough for a microcontroller to sample it (http://digitalsubstation.com/en/2016/11/08/white-paper-on-implementing-ptp-in-substations/)
//  val PRESCALER_WIDTH = log2Up(((TIME_STEP.toFloat/SEC_IN_NANO.toFloat)/(1f/clockFreq.toFloat)).toInt)
//
//  println("IEEE 1588 RTC instantiated @ " + clockFreq/1000000 + " MHz with TIME_STEP= "+TIME_STEP+" and calculated PRESCALER_WIDTH=" + PRESCALER_WIDTH)
//
//  // Register command
//  val masterReg = Reg(next = io.ocp.M)
//
//  // Registers
//  val prescaleReg = Reg(init = 0.U(PRESCALER_WIDTH.W))
//  val correctionStepReg = Reg(init = SInt(0, width = nanoWidth+1))
//  val nsOffsetReg = Reg(init = SInt(0, width = nanoWidth+1))
//  val timeReg = Reg(init = 0.U((secondsWidth + nanoWidth).W))
//  val secTickReg = Reg(init = initialTime.U((secondsWidth).W))
//  val nsTickReg = Reg(init = 0.U(nanoWidth.W))
//  val updateSecReg = Reg(init = false.B)
//  val updateNsReg = Reg(init = false.B)
//  val periodSelReg = Reg(init = (secondsWidth + nanoWidth - 1).U(log2Up(secondsWidth + nanoWidth).W))
//  val ppsReg = Reg(init = false.B)
//  val ppsHoldCountReg = Reg(init = PPS_DURATION.U(log2Up(PPS_DURATION).W))
//
//  // Default response
//  val respReg = Reg(init = OcpResp.NULL)
//  respReg := OcpResp.NULL
//  val dataReg = Reg(init = Bits(0, width = DATA_WIDTH))
//
//  // Clock engine
//  prescaleReg := prescaleReg + 1.U
//  val tickEnPulse = prescaleReg(PRESCALER_WIDTH-1) & ~Reg(next = prescaleReg(PRESCALER_WIDTH-1))
//
//  //Update Seconds
//  when(updateSecReg) {
//    secTickReg := timeReg(2 * DATA_WIDTH - 1, DATA_WIDTH)
//  }
//
//  //PPS Engine
//  when(~ppsReg){
//    ppsReg := nsTickReg >= (SEC_IN_NANO.U - (TIME_STEP.S + correctionStepReg).asUInt())
//  }.otherwise{
//    when(ppsHoldCountReg > TIME_STEP.U){
//      ppsHoldCountReg := ppsHoldCountReg - TIME_STEP.U
//    }.otherwise {
//      ppsHoldCountReg := PPS_DURATION.U
//      ppsReg := false.B
//    }
//  }
//
//  //Update Seconds/Nanoseconds
//  when(updateNsReg) {
//    nsTickReg := timeReg(nanoWidth - 1, 0)
//  }.elsewhen(tickEnPulse){
//   when(nsTickReg >= (SEC_IN_NANO.U - (TIME_STEP.S + correctionStepReg).asUInt())){
//    secTickReg := secTickReg + 1.U
//    nsTickReg := (nsTickReg - SEC_IN_NANO.U) + (TIME_STEP.S + correctionStepReg).asUInt()
//   }.otherwise{
//    nsTickReg := nsTickReg + (TIME_STEP.S + correctionStepReg).asUInt()
//    nsOffsetReg := nsOffsetReg + correctionStepReg // Always correct towards zero offset on every tickPulse
//   }
//  }
//
//  //Smooth Adjustment
//  when(nsOffsetReg =/= 0.S) {
//    when(nsOffsetReg < -MICRO_IN_NANO.S && nsOffsetReg >= -MILLI_IN_NANO.S) { //-1ms to 1us
//      correctionStepReg := TIME_STEP.S
//    }.elsewhen(nsOffsetReg < -HUNDRED_NANO.S && nsOffsetReg >= -MICRO_IN_NANO.S) { //-1us to -100ns
//      correctionStepReg := (TIME_STEP/2).S
//    }.elsewhen(nsOffsetReg < -FIFTY_NANO.S && nsOffsetReg >= -HUNDRED_NANO.S) {   //-100ns to -50ns
//      correctionStepReg := (TIME_STEP/4).S
//    }.elsewhen(nsOffsetReg < 0.S && nsOffsetReg >= -FIFTY_NANO.S) {                 //-50ns to -1ns
//      correctionStepReg := 1.S
//    }.elsewhen(nsOffsetReg > 0.S && nsOffsetReg <= FIFTY_NANO.S){                   //1ns to 50ns
//      correctionStepReg := -1.S
//    }.elsewhen(nsOffsetReg > FIFTY_NANO.S && nsOffsetReg <= HUNDRED_NANO.S) {     //50ns to 100ns
//      correctionStepReg := (-TIME_STEP/4).S
//    }.elsewhen(nsOffsetReg > HUNDRED_NANO.S && nsOffsetReg <= MICRO_IN_NANO.S) {   //100ns to 1us
//      correctionStepReg := (-TIME_STEP/2).S
//    }.elsewhen(nsOffsetReg > MICRO_IN_NANO.S && nsOffsetReg <= MILLI_IN_NANO.S) {   //1us to 1ms
//      correctionStepReg := -TIME_STEP.S
//    }.otherwise{
//      correctionStepReg := 0.S
//    }
//  }.otherwise {
//    correctionStepReg := 0.S
//  }
//
//  //Register current time when it is not being updated
//  when(masterReg.Cmd =/= OcpCmd.WR){
//    timeReg := Cat(secTickReg, nsTickReg)
//  }
//
//  // Write response
//  updateSecReg := false.B
//  updateNsReg := false.B
//  when(masterReg.Cmd === OcpCmd.WR) {
//    respReg := OcpResp.DVA
//    switch(masterReg.Addr(5, 0)){
//      is(Bits("h00")){
//        updateNsReg := true.B
//        timeReg(DATA_WIDTH-1, 0) := masterReg.Data
//      }
//      is(Bits("h04")){
//        updateSecReg := true.B
//        timeReg(2*DATA_WIDTH-1, DATA_WIDTH) := masterReg.Data
//      }
//      is(Bits("h10")){
//        periodSelReg := masterReg.Data
//      }
//      is(Bits("h14")){
//        periodSelReg := masterReg.Data
//      }
//      is(Bits("h20")){
//        nsOffsetReg := masterReg.Data.toSInt()
//      }
//    }
//  }
//
//  // Read response
//  when(masterReg.Cmd === OcpCmd.RD) {
//    respReg := OcpResp.DVA
//    switch(masterReg.Addr(5, 0)){
//      is(Bits("h00")){
//        dataReg := timeReg(DATA_WIDTH-1, 0)
//      }
//      is(Bits("h04")){
//        dataReg := timeReg(2*DATA_WIDTH-1, DATA_WIDTH)
//      }
//      is(Bits("h10")){
//        dataReg := periodSelReg
//      }
//      is(Bits("h14")){
//        dataReg := periodSelReg
//      }
//      is(Bits("h20")){
//        dataReg := nsOffsetReg
//      }
//    }
//  }
//
//  // No interrupts by default
//  io.periodIntr := timeReg(periodSelReg)
//
//  // Connections to master
//  io.ocp.S.Resp := respReg
//  io.ocp.S.Data := dataReg
//
//  // Connections to PTP
//  io.ptpTimestamp := timeReg
//  io.pps := ppsReg
//}

object RTC {
  def main(args: Array[String]): Unit = {
    chiselMain(Array[String]("--backend", "v", "--targetDir", "generated/RTC"),
      () => Module(new RTC(80000000, 32, 32, 0x5ac385dcL, 10)))
  }
}

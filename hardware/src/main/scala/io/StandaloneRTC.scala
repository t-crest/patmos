package io

import chisel3._
import chisel3.util._

import patmos.Constants._
import ptp1588assist.RTC

object StandaloneRTC extends DeviceObject {
  var initialTime = 1522763228L
  var secondsWidth = 0
  var nanoWidth = 0


  def init(params: Map[String, String]) = {
    initialTime = 1522763228L
    secondsWidth = getPosIntParam(params, "secondsWidth")
    nanoWidth = getPosIntParam(params, "nanoWidth")
  }

  def create(params: Map[String, String]) : StandaloneRTC = {
    Module(new StandaloneRTC(secondsWidth, nanoWidth, initialTime))
  }
}

class StandaloneRTC(secondsWidth: Int = 40, nanoWidth: Int = 24, initialTime: BigInt = 0L, timeStep: Int = 25) extends CoreDevice() {
  override val io = new CoreDeviceIO() with patmos.HasPins {
    val pins: Bundle {
      val hexDisp: Vec[UInt]
    } = new Bundle {
      val hexDisp = Output(VecInit(Seq.fill(8)(0.U(7.W)))) // MS: this should not be a fill, just a Vec without defaults
    }
  }

  // Decode hardware
  def sevenSegBCDDecode(data : Bits, segmentPolarity: Int) : Bits = {
    val result = UInt(7.W)
    result := "b1000001".U
    switch(data(3,0)){
      is("b0000".U){
        result := "b1000000".U    //0
      }
      is("b0001".U){
        result := "b1111001".U    //1
      }
      is("b0010".U){
        result := "b0100100".U    //2
      }
      is("b0011".U){
        result := "b0110000".U    //3
      }
      is("b0100".U){
        result := "b0011001".U    //4
      }
      is("b0101".U){
        result := "b0010010".U    //5
      }
      is("b0110".U){
        result := "b0000010".U    //6
      }
      is("b0111".U){
        result := "b1111000".U    //7
      }
      is("b1000".U){
        result := "b0000000".U    //8
      }
      is("b1001".U){
        result := "b0011000".U    //9
      }
      is("b1010".U){
        result := "b0001000".U    //A
      }
      is("b1011".U){
        result := "b0000011".U    //B
      }
      is("b1100".U){
        result := "b1000110".U    //C
      }
      is("b1101".U){
        result := "b0100001".U    //D
      }
      is("b1110".U){
        result := "b0000110".U    //E
      }
      is("b1111".U){
        result := "b0001110".U    //F
      }
    }
    if (segmentPolarity==0) {
      result
    } else {
      ~result
    }
  }

  val rtc = Module(new RTC(CLOCK_FREQ, secondsWidth, nanoWidth, initialTime, timeStep))
  rtc.io.ocp <> io.ocp

  val dispRegVec = RegInit(VecInit(Seq.fill(8)(Bits(7.W))))

  dispRegVec(0) := sevenSegBCDDecode(rtc.io.ptpTimestamp(35, 32), segmentPolarity = 0)
  dispRegVec(1) := sevenSegBCDDecode(rtc.io.ptpTimestamp(39, 36), segmentPolarity = 0)
  dispRegVec(2) := sevenSegBCDDecode(rtc.io.ptpTimestamp(43, 40), segmentPolarity = 0)
  dispRegVec(3) := sevenSegBCDDecode(rtc.io.ptpTimestamp(47, 44), segmentPolarity = 0)
  dispRegVec(4) := sevenSegBCDDecode(rtc.io.ptpTimestamp(51, 48), segmentPolarity = 0)
  dispRegVec(5) := sevenSegBCDDecode(rtc.io.ptpTimestamp(55, 52), segmentPolarity = 0)
  dispRegVec(6) := sevenSegBCDDecode(rtc.io.ptpTimestamp(59, 56), segmentPolarity = 0)
  dispRegVec(7) := sevenSegBCDDecode(rtc.io.ptpTimestamp(63, 60), segmentPolarity = 0)

  io.pins.hexDisp := dispRegVec
}

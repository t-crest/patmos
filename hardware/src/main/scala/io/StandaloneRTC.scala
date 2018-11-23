package io

import Chisel._
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

  trait Pins {
    val standaloneRTCPins: Bundle {
      val hexDisp: Vec[UInt]
    } = new Bundle() {
      val hexDisp = Vec.fill(8) {Bits(OUTPUT, 7)}
    }
  }

  trait Intrs{
    val periodIntr = Bool(OUTPUT)
  }
}

class StandaloneRTC(secondsWidth: Int = 40, nanoWidth: Int = 24, initialTime: BigInt) extends CoreDevice() {
  override val io = new CoreDeviceIO() with StandaloneRTC.Pins with StandaloneRTC.Intrs

  // Decode hardware
  def sevenSegBCDDecode(data : Bits, segmentPolarity: Int) : Bits = {
    val result = Bits(width = 7)
    result := Bits("b1000001")
    switch(data(3,0)){
      is(Bits("b0000")){
        result := Bits("b1000000")    //0
      }
      is(Bits("b0001")){
        result := Bits("b1111001")    //1
      }
      is(Bits("b0010")){
        result := Bits("b0100100")    //2
      }
      is(Bits("b0011")){
        result := Bits("b0110000")    //3
      }
      is(Bits("b0100")){
        result := Bits("b0011001")    //4
      }
      is(Bits("b0101")){
        result := Bits("b0010010")    //5
      }
      is(Bits("b0110")){
        result := Bits("b0000010")    //6
      }
      is(Bits("b0111")){
        result := Bits("b1111000")    //7
      }
      is(Bits("b1000")){
        result := Bits("b0000000")    //8
      }
      is(Bits("b1001")){
        result := Bits("b0011000")    //9
      }
      is(Bits("b1010")){
        result := Bits("b0001000")    //A
      }
      is(Bits("b1011")){
        result := Bits("b0000011")    //B
      }
      is(Bits("b1100")){
        result := Bits("b1000110")    //C
      }
      is(Bits("b1101")){
        result := Bits("b0100001")    //D
      }
      is(Bits("b1110")){
        result := Bits("b0000110")    //E
      }
      is(Bits("b1111")){
        result := Bits("b0001110")    //F
      }
    }
    if (segmentPolarity==0) {
      result
    } else {
      ~result
    }
  }

  val rtc = Module(new RTC(CLOCK_FREQ, secondsWidth, nanoWidth, initialTime))
  rtc.io.ocp <> io.ocp
  io.periodIntr := rtc.io.periodIntr

//  io.standaloneRTCPins.hexDisp.setName("io_sevenSegmentDisplayPins_hexDisp")

  val dispRegVec = RegInit(Vec.fill(8){Bits(0, width = 7)})

  dispRegVec(0) := sevenSegBCDDecode(rtc.io.ptpTimestamp(35, 32), segmentPolarity = 0)
  dispRegVec(1) := sevenSegBCDDecode(rtc.io.ptpTimestamp(39, 36), segmentPolarity = 0)
  dispRegVec(2) := sevenSegBCDDecode(rtc.io.ptpTimestamp(43, 40), segmentPolarity = 0)
  dispRegVec(3) := sevenSegBCDDecode(rtc.io.ptpTimestamp(47, 44), segmentPolarity = 0)
  dispRegVec(4) := sevenSegBCDDecode(rtc.io.ptpTimestamp(51, 48), segmentPolarity = 0)
  dispRegVec(5) := sevenSegBCDDecode(rtc.io.ptpTimestamp(55, 52), segmentPolarity = 0)
  dispRegVec(6) := sevenSegBCDDecode(rtc.io.ptpTimestamp(59, 56), segmentPolarity = 0)
  dispRegVec(7) := sevenSegBCDDecode(rtc.io.ptpTimestamp(63, 60), segmentPolarity = 0)

  io.standaloneRTCPins.hexDisp := dispRegVec
}

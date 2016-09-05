// Simulation model for the WM8731 audio CODEC (only ADC part)
// first writes data (00:01) (LFT:RGT) and then increments each value by 2

package io

import Chisel._

class AudioWM8731ADCModel(AUDIOBITLENGTH: Int) extends Module {

  // IOs
  val io = new Bundle {
    val BCLK = UInt(INPUT, 1)
    val ADCLRC = UInt(INPUT, 1)
    val ADCDAT = UInt(OUTPUT, 1)
  }

  // audio data registers
  val audioLReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRReg = Reg(init = UInt(1, AUDIOBITLENGTH))

  //register for output data bit
  val adcDatReg = Reg(init = UInt(0, 1))
  io.ADCDAT = adcDatReg

  //Counter
  val CNTLIMIT = UInt(AUDIOBITLENGTH - 1)
  val CntReg = Reg(init = UInt(0, 5))

  //state machine
  val sIdle :: sReady :: sLeftLo :: sLeftHi :: sRightLo :: sRightHi :: Nil = Enum(UInt(), 6)
  val state = Reg(init = sIdle)

  switch (state) {
    is (sIdle) {
      when(io.ADCLRC === UInt(1)) {
        state := sReady
      }
    }
    is (sReady) {
      CntReg := CNTLIMIT
      when(io.ADCLRC === UInt(0)) {
        state := sLeftLo
      }
    }
    is (sLeftLo) {
      adcDatReg := audioLReg(CntReg)
      when (io.BCLK === UInt(1)) {
        state := sLeftHi
      }
    }
    is (sLeftHi) {
      when (io.BCLK === UInt(0)) {
        when (CntReg === UInt(0)) { //limit reached
          CntReg := CNTLIMIT
          state := sRightLo
        }
        .otherwise {
          CntReg := CntReg - UInt(1) //decrement counter
          state := sLeftLo
        }
      }
    }
    is (sRightLo) {
      adcDatReg := audioRReg(CntReg)
      when (io.BCLK === UInt(1)) {
        state := sRightHi
      }
    }
    is (sRightHi) {
      when (io.BCLK === UInt(0)) {
        when (CntReg === UInt(0)) { //limit reached
          CntReg := CNTLIMIT
          audioLReg := audioLReg + UInt(2)
          audioRReg := audioRReg + UInt(2)
          state := idle
        }
        .otherwise {
          CntReg := CntReg - UInt(1) //decrement counter
          state := sRightLo
        }
      }
    }
  }

}

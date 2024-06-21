// Simulation model for the WM8731 audio CODEC (only ADC part)
// first writes data (1024:1025) (LFT:RGT) and then increments each value by 2

package io

import chisel3._
import chisel3.util._

class AudioWM8731ADCModel(AUDIOBITLENGTH: Int) extends Module {

  // IOs
  val io = new Bundle {
    val bClk = Input(UInt(1.W))
    val adcLrc = Input(UInt(1.W))
    val adcDat = Output(UInt(1.W))
  }

  // audio data registers
  val audioLReg = RegInit(init = 1024.U(AUDIOBITLENGTH.W))
  val audioRReg = RegInit(init = 1025.U(AUDIOBITLENGTH.W))

  //register for output data bit
  val adcDatReg = RegInit(init = 0.U(1.W))
  io.adcDat := adcDatReg

  //Counter
  val CNTLIMIT = (AUDIOBITLENGTH - 1).U
  val CntReg = RegInit(init = 0.U(5.W))

  //state machine
  val sIdle :: sReady :: sLeftLo :: sLeftHi :: sRightLo :: sRightHi :: Nil = Enum(6)
  val state = RegInit(init = sIdle)

  switch (state) {
    is (sIdle) {
      when(io.adcLrc === 1.U) {
        state := sReady
      }
    }
    is (sReady) {
      CntReg := CNTLIMIT
      when(io.adcLrc === 0.U) {
        state := sLeftLo
      }
    }
    is (sLeftLo) {
      adcDatReg := audioLReg(CntReg)
      when (io.bClk === 1.U) {
        state := sLeftHi
      }
    }
    is (sLeftHi) {
      when (io.bClk === 0.U) {
        when (CntReg === 0.U) { //limit reached
          CntReg := CNTLIMIT
          state := sRightLo
        }
        .otherwise {
          CntReg := CntReg - 1.U //decrement counter
          state := sLeftLo
        }
      }
    }
    is (sRightLo) {
      adcDatReg := audioRReg(CntReg)
      when (io.bClk === 1.U) {
        state := sRightHi
      }
    }
    is (sRightHi) {
      when (io.bClk === 0.U) {
        when (CntReg === 0.U) { //limit reached
          CntReg := CNTLIMIT
          audioLReg := audioLReg + 2.U
          audioRReg := audioRReg + 2.U
          state := sIdle
        }
        .otherwise {
          CntReg := CntReg - 1.U //decrement counter
          state := sRightLo
        }
      }
    }
  }

}

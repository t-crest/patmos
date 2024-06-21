// DAC converter for WM8731 audio codec.
// receives audio data into audioLI and audioRI registers
// converts every time enable signal is set to high
// sets writeEnDacO to low while converting: during (1 + AUDIOBITLENGTH*2) cycles of BCLK

//always performs a conversion of empty data first
//to then be synced with the ADC: always 1 sample behind

package io

import chisel3._
import chisel3.util._

class AudioDAC(AUDIOBITLENGTH: Int, FSDIV: Int) extends Module { //constants: from CONFIG parameters
  //val AUDIOBITLENGTH = 16;
  //val FSDIV = 256;

  //IOs
  val io = new Bundle {
    // from/to AudioDACBuffer
    val audioLI = Input(UInt(AUDIOBITLENGTH.W))
    val audioRI = Input(UInt(AUDIOBITLENGTH.W))
    val enDacI = Input(Bool()) //enable signal
    val writeEnDacO = Output(UInt(1.W)) // used for sync
    val convEndO = Output(UInt(1.W)) // indicates end of conversion
    // from AudioClkGen
    val bclkI = Input(UInt(1.W))
    // to WM8731
    val dacLrcO = Output(UInt(1.W))
    val dacDatO = Output(UInt(1.W))
  }


  //Counter for audio sampling
  val FSCYCLES = (FSDIV - 1).U;
  val fsCntReg = RegInit(init = 0.U(9.W)) //counter register for Fs

  val audioCntReg = RegInit(init = 0.U(5.W)) //counter register for Audio bits: max 32 bits: 5 bit counter

  //states
  val sIdle :: sStart :: sLeft :: sRight :: Nil = Enum(4)
  //state register
  val state = RegInit(init = sIdle)

  //Registers for outputs:
  val dacLrcReg = RegInit(init = 0.U(1.W))
  val dacDatReg = RegInit(init = 0.U(1.W)) //assign to ouputs
  io.dacLrcO := dacLrcReg
  io.dacDatO := dacDatReg

  //register for write enable signal to buffer
  val writeEnDacReg = RegInit(init = 0.U(1.W)) // starts with writing disabled
  io.writeEnDacO := writeEnDacReg

  //registers for audio data
  val audioLReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))
  val audioRReg = RegInit(init = 0.U(AUDIOBITLENGTH.W))

  //register for bclkI
  val bclkReg = RegInit(init = 0.U(1.W))
  bclkReg := io.bclkI

  //end of conversion indicator
  val convEndReg = RegInit(init = 0.U(1.W))
  io.convEndO := convEndReg

  //connect inputs to registers when writing is enabled
  when(writeEnDacReg === 1.U) {
    audioLReg := io.audioLI
    audioRReg := io.audioRI
  }


  when(io.enDacI === 1.U) { //when conversion is enabled

    //state machine: on falling edge of BCLK
    when((io.bclkI =/= bclkReg) && (io.bclkI === 0.U)) {

      //counter for audio sampling
      fsCntReg := fsCntReg + 1.U
      when(fsCntReg === FSCYCLES) {
        fsCntReg := 0.U //reset to 0
        convEndReg := 1.U // Indicate end of conversion cycle
      }.otherwise {
        convEndReg := 0.U
      }

      //FSM for audio conversion
      switch(state) {
        is(sIdle) {
          dacLrcReg := 0.U
          dacDatReg := 0.U
          when(fsCntReg === 0.U) {
            writeEnDacReg := 0.U //to start disabled
            state := sStart
          }.otherwise {
            writeEnDacReg := 1.U
          }
        }
        is(sStart) {
          dacLrcReg := 1.U
          writeEnDacReg := 0.U
          state := sLeft //directly jump to next state
        }
        is(sLeft) {
          dacLrcReg := 0.U
          writeEnDacReg := 0.U
          dacDatReg := audioLReg(AUDIOBITLENGTH.U - audioCntReg - 1.U)
          when(audioCntReg < (AUDIOBITLENGTH - 1).U) {
            audioCntReg := audioCntReg + 1.U
          }.otherwise //bit AUDIOBITLENGTH-1
          {
            audioCntReg := 0.U //restart counter
            state := sRight
          }
        }
        is(sRight) {
          writeEnDacReg := 0.U
          dacDatReg := audioRReg(AUDIOBITLENGTH.U - audioCntReg - 1.U)
          when(audioCntReg < (AUDIOBITLENGTH - 1).U) {
            audioCntReg := audioCntReg + 1.U
          }.otherwise //bit AUDIOBITLENGTH-1
          {
            audioCntReg := 0.U //restart counter
            state := sIdle
          }
        }
      }
    }
  }.otherwise //when conversion is disabled
  {
    state := sIdle
    fsCntReg := 0.U
    audioCntReg := 0.U
    writeEnDacReg := 0.U
    dacLrcReg := 0.U
    dacDatReg := 0.U
    convEndReg := 0.U
    audioLReg := 0.U
    audioRReg := 0.U
  }

}

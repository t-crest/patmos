// ADC converter for WM8731 audio codec.
// receives audio data from WM8731 and stores it into audioLO and audioRO registers
// converts every time enable signal is set to high
// sets readEn to low while converting: during (1 + AUDIOBITLENGTH*2) cycles of BCLK

package io

import Chisel._

class AudioADC(AUDIOBITLENGTH: Int, FSDIV: Int) extends Module
{
  //constants: from CONFIG parameters
  //val AUDIOBITLENGTH = 16;
  //val FSDIV = 256;

  //IOs
  val io = new Bundle
  {
    //to/from AudioADCBuffer
    val audioLO = Output(UInt(AUDIOBITLENGTH.W))
    val audioRO = Output(UInt(AUDIOBITLENGTH.W))
    val enAdcI = Input(Bool()) //enable signal
    val readEnAdcO = Output(UInt(1.W)) // used for sync
    //from AudioClkGen
    val bclkI = Input(UInt(1.W))
    //to/from WM8731
    val adcLrcO = Output(UInt(1.W))
    val adcDatI = Input(UInt(1.W))
  }

  //Counter for audio sampling
  val FSCYCLES = (FSDIV-1).U;
  val fsCntReg = Reg(init = 0.U(9.W)) //counter register for Fs

  val audioCntReg = Reg(init = 0.U(5.W)) //counter register for Audio bits: max 32 bits: 5 bit counter

  //states
  val sIdle :: sStart1 :: sStart2 :: sLeft :: sRight :: Nil = Enum(UInt(), 5)
  //state register
  val state = Reg(init = sIdle)

  //Registers for outputs:
  val adcLrcReg = Reg(init = 0.U(1.W))

  // register for read enable signal to buffer
  val readEnAdcReg = Reg(init = 0.U(1.W)) // starts with read disabled
  io.readEnAdcO := readEnAdcReg

  //assign to inputs/uputs
  io.adcLrcO 	:= adcLrcReg

  //register for bclkI
  val bclkReg = Reg(init = 0.U(1.W))
  bclkReg := io.bclkI

  //registers for audio data
  val audioLReg = RegInit(Vec(Seq.fill(AUDIOBITLENGTH)(0.U(1.W))))
  val audioRReg = RegInit(Vec(Seq.fill(AUDIOBITLENGTH)(0.U(1.W))))
  val audioLRegO = Reg(init = 0.U(AUDIOBITLENGTH.W))
  val audioRRegO = Reg(init = 0.U(AUDIOBITLENGTH.W))
  //connect registers to ouputs when conversion is not busy
  io.audioLO := audioLRegO
  io.audioRO := audioRRegO
  when(readEnAdcReg === 1.U) {
    audioLRegO := Cat(audioLReg.reverse)
    audioRRegO := Cat(audioRReg.reverse)
  }

  //conversion when enabled
  when (io.enAdcI === 1.U) {

    //state machine: on falling edge of BCLK for idle, start and wait
    when( (io.bclkI =/= bclkReg) && (io.bclkI === 0.U) ) {

      //counter for audio sampling
      fsCntReg := fsCntReg + 1.U
      when(fsCntReg === FSCYCLES) {
	      fsCntReg := 0.U //reset to 0
      }
      //FSM for audio conversion
      switch (state) {
        is (sIdle)
        {
	        adcLrcReg := 0.U
          when (fsCntReg === 0.U) {
            readEnAdcReg := 0.U // to avoid initial readEn pulse
	          state := sStart1
          }
          .otherwise {
            readEnAdcReg := 1.U
          }
	      }
	      is (sStart1)
	      {
	        adcLrcReg := 1.U
          readEnAdcReg := 0.U
	        state := sStart2 //directly jump to next state
	      }
	      is (sStart2)
	      {
          readEnAdcReg := 0.U
	        adcLrcReg := 0.U //lrclk low already
	        state := sLeft //directly jump to next state
	      }
      }
    }

    //state machine: on raising edge of BCLK for left and right
    .elsewhen( (io.bclkI =/= bclkReg) && (io.bclkI === 1.U) ) {
      //FSM for audio conversion
      switch (state) {
	      is (sLeft)
	      {
          readEnAdcReg := 0.U
	        audioLReg(AUDIOBITLENGTH.U - audioCntReg - 1.U) := io.adcDatI
	        when (audioCntReg < (AUDIOBITLENGTH-1).U)
	        {
	          audioCntReg := audioCntReg + 1.U
	        }
	          .otherwise //bit AUDIOBITLENGTH-1
	        {
	          audioCntReg := 0.U //restart counter
	          state := sRight
	        }
	      }
	      is (sRight)
	      {
          readEnAdcReg := 0.U
	        audioRReg(AUDIOBITLENGTH.U - audioCntReg - 1.U) := io.adcDatI
	        when (audioCntReg < (AUDIOBITLENGTH-1).U)
	        {
	          audioCntReg := audioCntReg + 1.U
	        }
	          .otherwise //bit AUDIOBITLENGTH-1
	        {
	          audioCntReg := 0.U //restart counter
	          state := sIdle
	        }
	      }
      }
    }
  }
    .otherwise {//when not enable
    state := sIdle
    fsCntReg := 0.U
    audioCntReg := 0.U
    readEnAdcReg := 0.U
    adcLrcReg := 0.U
  }
}

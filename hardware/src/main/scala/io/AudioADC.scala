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
  val FSCYCLES = UInt(FSDIV-1);
  val fsCntReg = Reg(init = UInt(0, 9)) //counter register for Fs

  val audioCntReg = Reg(init = UInt(0, 5)) //counter register for Audio bits: max 32 bits: 5 bit counter

  //states
  val sIdle :: sStart1 :: sStart2 :: sLeft :: sRight :: Nil = Enum(UInt(), 5)
  //state register
  val state = Reg(init = sIdle)

  //Registers for outputs:
  val adcLrcReg = Reg(init = UInt(0, 1))

  // register for read enable signal to buffer
  val readEnAdcReg = Reg(init = UInt(0, 1)) // starts with read disabled
  io.readEnAdcO := readEnAdcReg

  //assign to inputs/uputs
  io.adcLrcO 	:= adcLrcReg

  //register for bclkI
  val bclkReg = Reg(init = UInt(0, 1))
  bclkReg := io.bclkI

  //registers for audio data
  val audioLReg = RegInit(Vec(Seq.fill(AUDIOBITLENGTH)(0.U(1.W))))
  val audioRReg = RegInit(Vec(Seq.fill(AUDIOBITLENGTH)(0.U(1.W))))
  val audioLRegO = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRRegO = Reg(init = UInt(0, AUDIOBITLENGTH))
  //connect registers to ouputs when conversion is not busy
  io.audioLO := audioLRegO
  io.audioRO := audioRRegO
  when(readEnAdcReg === UInt(1)) {
    audioLRegO := Cat(audioLReg.reverse)
    audioRRegO := Cat(audioRReg.reverse)
  }

  //conversion when enabled
  when (io.enAdcI === UInt(1)) {

    //state machine: on falling edge of BCLK for idle, start and wait
    when( (io.bclkI =/= bclkReg) && (io.bclkI === UInt(0)) ) {

      //counter for audio sampling
      fsCntReg := fsCntReg + UInt(1)
      when(fsCntReg === FSCYCLES) {
	      fsCntReg := UInt(0) //reset to 0
      }
      //FSM for audio conversion
      switch (state) {
        is (sIdle)
        {
	        adcLrcReg := UInt(0)
          when (fsCntReg === UInt(0)) {
            readEnAdcReg := UInt(0) // to avoid initial readEn pulse
	          state := sStart1
          }
          .otherwise {
            readEnAdcReg := UInt(1)
          }
	      }
	      is (sStart1)
	      {
	        adcLrcReg := UInt(1)
          readEnAdcReg := UInt(0)
	        state := sStart2 //directly jump to next state
	      }
	      is (sStart2)
	      {
          readEnAdcReg := UInt(0)
	        adcLrcReg := UInt(0) //lrclk low already
	        state := sLeft //directly jump to next state
	      }
      }
    }

    //state machine: on raising edge of BCLK for left and right
    .elsewhen( (io.bclkI =/= bclkReg) && (io.bclkI === UInt(1)) ) {
      //FSM for audio conversion
      switch (state) {
	      is (sLeft)
	      {
          readEnAdcReg := UInt(0)
	        audioLReg(UInt(AUDIOBITLENGTH) - audioCntReg - UInt(1)) := io.adcDatI
	        when (audioCntReg < UInt(AUDIOBITLENGTH-1))
	        {
	          audioCntReg := audioCntReg + UInt(1)
	        }
	          .otherwise //bit AUDIOBITLENGTH-1
	        {
	          audioCntReg := UInt(0) //restart counter
	          state := sRight
	        }
	      }
	      is (sRight)
	      {
          readEnAdcReg := UInt(0)
	        audioRReg(UInt(AUDIOBITLENGTH) - audioCntReg - UInt(1)) := io.adcDatI
	        when (audioCntReg < UInt(AUDIOBITLENGTH-1))
	        {
	          audioCntReg := audioCntReg + UInt(1)
	        }
	          .otherwise //bit AUDIOBITLENGTH-1
	        {
	          audioCntReg := UInt(0) //restart counter
	          state := sIdle
	        }
	      }
      }
    }
  }
    .otherwise {//when not enable
    state := sIdle
    fsCntReg := UInt(0)
    audioCntReg := UInt(0)
    readEnAdcReg := UInt(0)
    adcLrcReg := UInt(0)
  }
}

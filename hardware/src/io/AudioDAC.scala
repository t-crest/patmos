// DAC converter for WM8731 audio codec.
// receives audio data into audioLI and audioRI registers
// converts every time enable signal is set to high
// sets writeEnDacO to low while converting: during (1 + AUDIOBITLENGTH*2) cycles of BCLK

package io

import Chisel._

class AudioDAC(AUDIOBITLENGTH: Int, FSDIV: Int) extends Module
{
  //constants: from CONFIG parameters
  //val AUDIOBITLENGTH = 16;
  //val FSDIV = 256;

  //IOs
  val io = new Bundle
  {
    // from/to AudioDACBuffer
    val audioLI = UInt(INPUT, AUDIOBITLENGTH)
    val audioRI = UInt(INPUT, AUDIOBITLENGTH)
    val enDacI = Bool(dir = INPUT) //enable signal
    val writeEnDacO = UInt(OUTPUT, 1) // used for sync
    // from AudioClkGen
    val bclkI = UInt(INPUT, 1)
    // to WM8731
    val dacLrcO = UInt(OUTPUT, 1)
    val dacDatO = UInt(OUTPUT, 1)
  }


  //Counter for audio sampling
  val FSCYCLES = UInt(FSDIV - 1);
  val fsCntReg = Reg(init = UInt(0, 9)) //counter register for Fs

  val audioCntReg = Reg(init = UInt(0, 5)) //counter register for Audio bits: max 32 bits: 5 bit counter

  //states
  val sIdle :: sStart :: sLeft :: sRight :: Nil = Enum(UInt(), 4)
  //state register
  val state = Reg(init = sIdle)

  //Registers for outputs:
  val dacLrcReg = Reg(init = UInt(0, 1))
  val dacDatReg = Reg(init = UInt(0, 1))
  //assign to ouputs
  io.dacLrcO 	:= dacLrcReg
  io.dacDatO 	:= dacDatReg

  //register for write enable signal to buffer
  val writeEnDacReg = Reg(init = UInt(1, 1)) // starts with writing enabled
  io.writeEnDacO := writeEnDacReg

  //registers for audio data
  val audioLReg = Reg(init = UInt(0, AUDIOBITLENGTH))
  val audioRReg = Reg(init = UInt(0, AUDIOBITLENGTH))

  //register for bclkI
  val bclkReg = Reg(init = UInt(0, 1))
  bclkReg := io.bclkI

  //connect inputs to registers when writing is enabled
  when(writeEnDacReg === UInt(1)) {
    audioLReg := io.audioLI
    audioRReg := io.audioRI
  }


  when(io.enDacI === UInt(1)) { //when conversion is enabled

    //state machine: on falling edge of BCLK
    when( (io.bclkI =/= bclkReg) && (io.bclkI === UInt(0)) ) {

      //counter for audio sampling
      fsCntReg := fsCntReg + UInt(1)
      when(fsCntReg === FSCYCLES)
      {
	fsCntReg := UInt(0) //reset to 0
      }

      //FSM for audio conversion
      switch (state) {
	is (sIdle)
	{
	  dacLrcReg := UInt(0)
	  dacDatReg := UInt(0)
	  writeEnDacReg := UInt(1)
	  when(fsCntReg === UInt(0)) {
	    state := sStart
	  }
	}
	is (sStart)
	{
	  dacLrcReg := UInt(1)
	  writeEnDacReg := UInt(0)
	  state := sLeft //directly jump to next state
	}
	is (sLeft)
	{
	  dacLrcReg := UInt(0)
	  writeEnDacReg := UInt(0)
	  dacDatReg := audioLReg( UInt(AUDIOBITLENGTH) - audioCntReg - UInt(1))
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
	  writeEnDacReg := UInt(0)
	  dacDatReg := audioRReg(UInt(AUDIOBITLENGTH) - audioCntReg - UInt(1))
	  when (audioCntReg < UInt(AUDIOBITLENGTH-1))
	  {
	    audioCntReg := audioCntReg + UInt(1)
	  }
	    .otherwise 	 //bit AUDIOBITLENGTH-1
	  {
	    audioCntReg := UInt(0) //restart counter
	    state := sIdle
	  }
	}
      }
    }
  }
    .otherwise //when conversion is disabled
  {
    state := sIdle
    fsCntReg := UInt(0)
    audioCntReg := UInt(0)
    writeEnDacReg := UInt(1)
    dacLrcReg := UInt(0)
    dacDatReg := UInt(0)
  }

}

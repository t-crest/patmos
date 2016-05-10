// DAC converter for WM8731 audio codec.
// receives audio data into audio_l_i and audio_r_i registers
// converts every time enable signal is set to high
// sets busy to high while converting: during (1 + AUDIO_BITLENGTH*2) cycles of BCLK

package io

import Chisel._

class AudioDAC(AUDIO_BITLENGTH: Int, FS_DIV: Int) extends Module
{
	//constants: from CONFIG parameters
	//val AUDIO_BITLENGTH = 16;
	//val FS_DIV = 256;

	//IOs
	val io = new Bundle
	{
		//inputs from PATMOS
		val audio_l_i = UInt(INPUT, AUDIO_BITLENGTH)
		val audio_r_i = UInt(INPUT, AUDIO_BITLENGTH)
		val en_dac_i = Bool(dir = INPUT) //enable signal
		//from audio_clk_gen
		val BCLK_i = UInt(INPUT, 1)
		//outputs
		val busy_o = UInt(OUTPUT, 1) //to PATMOS
		val DACLRC_o = UInt(OUTPUT, 1) //to WM8731
		val DACDAT_o = UInt(OUTPUT, 1) //to WM8731
	}


	//Counter for audio sampling
	val Fs_CYCLES = UInt(FS_DIV - 1);
	val Fs_cntReg = Reg(init = UInt(0, 9)) //counter register for Fs

	val audio_cntReg = Reg(init = UInt(0, 5)) //counter register for Audio bits: max 32 bits: 5 bit counter

	//states
	val s_idle :: s_start :: s_left :: s_right :: Nil = Enum(UInt(), 4)
	//state register
	val state = Reg(init = s_idle)

	//Registers for outputs:
	val DACLRC_reg = Reg(init = UInt(0, 1))
	val DACDAT_reg = Reg(init = UInt(0, 1))
	val busy_reg = Reg(init = UInt(0, 1))
	//assign to ouputs
	io.DACLRC_o 	:= DACLRC_reg
	io.DACDAT_o 	:= DACDAT_reg
	io.busy_o 	:= busy_reg

	//registers for audio data
	val audio_l_reg = Reg(init = UInt(0, AUDIO_BITLENGTH))
	val audio_r_reg = Reg(init = UInt(0, AUDIO_BITLENGTH))

	//register for BCLK_i
	val BCLK_reg = Reg(init = UInt(0, 1))
	BCLK_reg := io.BCLK_i

	//connect inputs to registers when not busy
	when(busy_reg === UInt(0)) {
		audio_l_reg := io.audio_l_i
		audio_r_reg := io.audio_r_i
	}


	when(io.en_dac_i === UInt(1)) { //when conversion is enabled

		//state machine: on falling edge of BCLK
		when( (io.BCLK_i =/= BCLK_reg) && (io.BCLK_i === UInt(0)) ) {

			//counter for audio sampling
			Fs_cntReg := Fs_cntReg + UInt(1)
			when(Fs_cntReg === Fs_CYCLES)
			{
				Fs_cntReg := UInt(0) //reset to 0
			}

			//FSM for audio conversion
			switch (state) {
				is (s_idle)
				{
					DACLRC_reg := UInt(0)
					DACDAT_reg := UInt(0)
					busy_reg := UInt(0)
					when(Fs_cntReg === UInt(0)) {
						state := s_start
					}
				}
				is (s_start)
				{
					DACLRC_reg := UInt(1)
					busy_reg := UInt(1)
					state := s_left //directly jump to next state
				}
				is (s_left)
				{
					DACLRC_reg := UInt(0)
					busy_reg := UInt(1)
					DACDAT_reg := audio_l_reg( UInt(AUDIO_BITLENGTH) - audio_cntReg - UInt(1))
					when (audio_cntReg < UInt(AUDIO_BITLENGTH-1))
					{
						audio_cntReg := audio_cntReg + UInt(1)
					}
					.otherwise //bit AUDIO_BITLENGTH-1
					{
		   				audio_cntReg := UInt(0) //restart counter
						state := s_right
					}
				}
				is (s_right)
				{
					busy_reg := UInt(1)
					DACDAT_reg := audio_r_reg(UInt(AUDIO_BITLENGTH) - audio_cntReg - UInt(1))
					when (audio_cntReg < UInt(AUDIO_BITLENGTH-1))
					{
						audio_cntReg := audio_cntReg + UInt(1)
					}
					.otherwise 	 //bit AUDIO_BITLENGTH-1
					{
						audio_cntReg := UInt(0) //restart counter
						state := s_idle
					}
				}
			}
		}
	}
	.otherwise //when conversion is disabled
	{
		state := s_idle
		Fs_cntReg := UInt(0)
		audio_cntReg := UInt(0)
		busy_reg := UInt(0)
		DACLRC_reg := UInt(0)
		DACDAT_reg := UInt(0)
	}

}

// ADC converter for WM8731 audio codec.
// receives audio data from WM8731 and stores it into audio_l_o and audio_r_o registers
// converts every time enable signal is set to high
// sets busy to high while converting: during (1 + AUDIO_BITLENGTH*2) cycles of BCLK

package io

import Chisel._

class audio_adc(AUDIO_BITLENGTH: Int, FS_DIV: Int) extends Module 
{
	//constants: from CONFIG parameters
	//val AUDIO_BITLENGTH = 16;
	//val FS_DIV = 256;
	
	//IOs
	val io = new Bundle 
	{
		//to/from PATMOS
		val audio_l_o = UInt(OUTPUT, AUDIO_BITLENGTH)
		val audio_r_o = UInt(OUTPUT, AUDIO_BITLENGTH)
		val en_adc_i = Bool(dir = INPUT) //enable signal
		val busy_o = UInt(OUTPUT, 1)
		//from audio_clk_gen 
		val BCLK_i = UInt(INPUT, 1)
		//to/from WM8731
		val ADCLRC_o = UInt(OUTPUT, 1)
		val ADCDAT_i = UInt(INPUT, 1)
	}

	//Counter for audio sampling
	val Fs_CYCLES = UInt(FS_DIV-1);
	val Fs_cntReg = Reg(init = UInt(0, 9)) //counter register for Fs

	val audio_cntReg = Reg(init = UInt(0, 5)) //counter register for Audio bits: max 32 bits: 5 bit counter

	//states
	val s_idle :: s_start_1 :: s_start_2 :: s_left :: s_right :: Nil = Enum(UInt(), 5)
	//state register
	val state = Reg(init = s_idle)

	//Registers for outputs:
	val ADCLRC_reg = Reg(init = UInt(0, 1))
	val busy_reg = Reg(init = UInt(0, 1))
	//assign to inputs/uputs
	io.ADCLRC_o 	:= ADCLRC_reg
	io.busy_o 	:= busy_reg

	//register for BCLK_i
	val BCLK_reg = Reg(init = UInt(0, 1))
	BCLK_reg := io.BCLK_i

	//registers for audio data
	val audio_l_reg = Reg(init = UInt(0, AUDIO_BITLENGTH))
	val audio_r_reg = Reg(init = UInt(0, AUDIO_BITLENGTH))
	val audio_l_reg_o = Reg(init = UInt(0, AUDIO_BITLENGTH))
	val audio_r_reg_o = Reg(init = UInt(0, AUDIO_BITLENGTH))
	//connect registers to ouputs when conversion is not busy
	io.audio_l_o := audio_l_reg_o
	io.audio_r_o := audio_r_reg_o
	when(busy_reg === UInt(0)) {
		audio_l_reg_o := audio_l_reg
		audio_r_reg_o := audio_r_reg
	}

	//conversion when enabled
	when (io.en_adc_i === UInt(1)) {

		//state machine: on falling edge of BCLK for idle, start and wait
		when( (io.BCLK_i =/= BCLK_reg) && (io.BCLK_i === UInt(0)) ) {

			//counter for audio sampling
			Fs_cntReg := Fs_cntReg + UInt(1)
			when(Fs_cntReg === Fs_CYCLES) {
				Fs_cntReg := UInt(0) //reset to 0
			}
			//FSM for audio conversion
			switch (state) {
				is (s_idle) 
				{
					ADCLRC_reg := UInt(0)
					busy_reg := UInt(0)
					when (Fs_cntReg === UInt(0)) 
					{ 
						state := s_start_1 
					}
				}
				is (s_start_1) 
				{
					ADCLRC_reg := UInt(1)
					busy_reg := UInt(1)
					state := s_start_2 //directly jump to next state
				}
				is (s_start_2)
				{
					busy_reg := UInt(1)
					ADCLRC_reg := UInt(0) //lrclk low already
					state := s_left //directly jump to next state
				}
			}
		}

		//state machine: on raising edge of BCLK for left and right
		.elsewhen( (io.BCLK_i =/= BCLK_reg) && (io.BCLK_i === UInt(1)) ) {
			//FSM for audio conversion
			switch (state) {
				is (s_left) 
				{
					busy_reg := UInt(1)
					audio_l_reg(UInt(AUDIO_BITLENGTH) - audio_cntReg - UInt(1)) := io.ADCDAT_i
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
					audio_r_reg(UInt(AUDIO_BITLENGTH) - audio_cntReg - UInt(1)) := io.ADCDAT_i
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
	.otherwise {//when not enable
		state := s_idle
		Fs_cntReg := UInt(0)
		audio_cntReg := UInt(0)
		busy_reg := UInt(0)
		ADCLRC_reg := UInt(0)
	}
}


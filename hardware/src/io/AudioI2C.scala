


package io

import Chisel._

class audio_i2c extends Module 
{
	//constant: I2C slave address
	val SLAVE_ADDR = "b0011010"
	//IOs
	val io = new Bundle 
	{
		//inputs from PATMOS
		val data_i = UInt(INPUT, 9)
		val addr_i = UInt(INPUT, 7)
		//val rw_i = UInt(INPUT, 1) //temporarily disabled
		//wires to  to WM8731
		val sdin_i = UInt(INPUT, 1) //inout
		val sdin_o = UInt(OUTPUT, 1) //inout
		val sclk_o = UInt(OUTPUT, 1) //output
		val we_o = UInt(OUTPUT, 1) // write enable: 1->sdin_o, 0->sdin_i
		//handshake with patmos
		val req_i = UInt(INPUT, 1)
		val ack_o = UInt(OUTPUT, 1)
	}

	//registers for audio data
	val data_reg = Reg(init = UInt(0, 9))
	val addr_reg = Reg(init = UInt(0, 7))
	
	//connect inputs to registers
	data_reg := io.data_i
	addr_reg := io.addr_i

	//clock divider for SCLK
	val sclk_cntReg	 = Reg(init = UInt(0, 10)) 	//10 bit counter
	val sclk_cntMax  = UInt(400/2) 				// 80 MHz -> 400 cycles -> 200 kHz
	val sclk_cntMax2 = UInt(400) 				// Only used for finishing conditions

	val word_cntReg = Reg(init = UInt(0, 3)) 	//counter register for each word: 8 bit counter (000 to 111)

	//register for SCLK
	val sclk_reg = Reg(init = UInt(1, 1)) 		//default high
	io.sclk_o := sclk_reg
	//register for SDIN_O
	val sdin_reg = Reg(init = UInt(1, 1)) 		//default high
	io.sdin_o := sdin_reg
	//register for WE_O
	val we_reg = Reg(init = UInt(1, 1)) 		//default high: master controls SDIN
	io.we_o := we_reg

	//register for R_ADDR: constant
	val r_addr_reg = Reg(init = UInt(SLAVE_ADDR, 7))

	//states
	val s_idle :: s_start :: s_ack_1 :: s_data_msb :: s_ack_2 :: s_data_lsb :: s_ack_3 :: s_finish_1 :: s_finish_2 :: s_finish_patmos :: Nil = Enum(UInt(), 10)
	//state register
	val state = Reg(init = s_idle)

	//Default values:
	io.ack_o 	:= UInt(0)

	when (io.req_i === UInt(1)) 
	{ //PATMOS starts handshake

		//initial transition: idle to start
		when(state === s_idle) {
			we_reg := UInt(1)
			sdin_reg := UInt(0)
			state := s_start
		}
		.elsewhen(state === s_finish_2) {
			//counter for SCLK
			sclk_cntReg := sclk_cntReg + UInt(1)
			when(sclk_cntReg === sclk_cntMax) {
				sclk_reg := UInt(1)
			}
			.elsewhen(sclk_cntReg === sclk_cntMax2) {
				sclk_cntReg := UInt(0)
				sdin_reg := UInt(1)
				io.ack_o := UInt(1) //to PATMOS
				state := s_finish_patmos
			}
		}
		.elsewhen(state === s_finish_patmos) {
			io.ack_o := UInt(1) //to PATMOS
		}

		//in any other state, enable counter and everything
		.otherwise{
			//counter for SCLK
			sclk_cntReg := sclk_cntReg + UInt(1)
			//when limit - 1 -> switch
			when( sclk_cntReg === (sclk_cntMax - UInt(1)) ) {
				sclk_reg := ~sclk_reg
			}
			//when limit -> restart counter
			.elsewhen(sclk_cntReg === sclk_cntMax) 
			{
				sclk_cntReg := UInt(0)
				//update state machine only at falling edge
				when (sclk_reg === UInt(0)) {
					switch (state) {
						is (s_start) {
							when(word_cntReg < UInt(7)){
								sdin_reg := r_addr_reg(UInt(6)-word_cntReg)
								word_cntReg := word_cntReg + UInt(1)
							}
							.otherwise {//word_cntReg === UInt(7)) 
								sdin_reg := UInt(0) //for write
								word_cntReg := UInt(0)
								state := s_ack_1
							}
						}
						is (s_ack_1) {
							we_reg := UInt(0)
							sdin_reg := UInt(1) //doesn't really matter
							state := s_data_msb

						}
						is (s_data_msb) {
							we_reg := UInt(1)
							//check if WM8731 responded with ACK correctly on the first CC of this state
							when(  (sclk_cntReg === UInt(0)) && (io.sdin_i === UInt(1)) ) {
								//in this case, reset
								sclk_cntReg := UInt(0)
								state := s_idle
							}
							//if it ACK is correct (sdin === 0), then proceed
							when(word_cntReg < UInt(7)){
								sdin_reg := addr_reg(UInt(6)-word_cntReg)
								word_cntReg := word_cntReg + UInt(1)
							}
							.otherwise {//word_cntReg === UInt(7)) 
								sdin_reg := data_reg(UInt(8))
								word_cntReg := UInt(0)
								state := s_ack_2
							}
						}
						is (s_ack_2) {
							we_reg := UInt(0)
							sdin_reg := UInt(1) //doesn't really matter
							state := s_data_lsb
						}
						is (s_data_lsb) {
							we_reg := UInt(1)
							//check if WM8731 responded with ACK correctly on the first CC of this state
							when(  (sclk_cntReg === UInt(0)) && (io.sdin_i === UInt(1)) ) {
								//in this case, reset
								sclk_cntReg := UInt(0)
								state := s_idle
							}
							//if it ACK is correct (sdin === 0), then proceed
							when(word_cntReg < UInt(7)){
								sdin_reg := data_reg(UInt(7)-word_cntReg)
								word_cntReg := word_cntReg + UInt(1)
							}
							.otherwise {//word_cntReg === UInt(7)) 
								sdin_reg := data_reg(UInt(0))
								word_cntReg := UInt(0)
								state := s_ack_3
							}
						}
						is (s_ack_3) {
							we_reg := UInt(0)
							sdin_reg := UInt(0) //I think it matters on this case
							state := s_finish_1
						}
						//added
						is (s_finish_1) {
							we_reg := UInt(1)
							sdin_reg := UInt(0)
							//check if WM8731 responded with ACK correctly
							when(io.sdin_i === UInt(0)) {
								state := s_finish_2
							}
							.otherwise { // if no ack
								state := s_idle
							}
						}
					}
				}
			}	
		}
	}			
	.otherwise { //io.req_i === UInt(0)
		when (state === s_finish_patmos) {
			state := s_idle
		}
	}
			      

		
  
}



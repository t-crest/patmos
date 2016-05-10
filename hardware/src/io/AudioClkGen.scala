// BCLK & XCLK generator for WM8731 audio codec.
// BCLK & XCLK run at F = Patmos_freq/clkDivider_i
//currently clkDivider = 6,  F = 13.33 MHz (patmos 80 MHz)

package io

import Chisel._

class audio_clk_gen(CLK_DIV: Int) extends Module
{
	//constants: from CONFIG parameters
	//val CLK_DIV = 6;

	//IOs
	val io = new Bundle 
	{
		//inputs from PATMOS
		val en_adc_i = UInt(INPUT, 1)
		val en_dac_i = UInt(INPUT, 1)
		//outputs to WM8731	
		val BCLK_o = UInt(OUTPUT, 1)
		val XCLK_o = UInt(OUTPUT, 1)
	}

	//register containing divider value
	val clk_lim_reg = Reg(init = UInt(6, 5)) //5 bits, initialized to 6
	clk_lim_reg := UInt(CLK_DIV/2) //get from params!

	//Counter: clock divider for BCLK and XCLK
	val clk_cnt_reg = Reg(init = UInt(0, 5)) //counter reg for BCLK and XCLK

	//registers for XCLK and BCLK
	val CLK_reg = Reg(init = UInt(0, 1))

	//connect outputs
	io.BCLK_o := CLK_reg
	io.XCLK_o := CLK_reg

	//count for CLK only when any enable signal is high
	when( (io.en_adc_i === UInt(1)) || (io.en_dac_i === UInt(1)) ) {		
		when( clk_cnt_reg === clk_lim_reg - UInt(1)) {
			clk_cnt_reg := UInt(0)
			CLK_reg := ~CLK_reg
		}
		.otherwise {
			clk_cnt_reg := clk_cnt_reg + UInt(1)
		}
	}
	.otherwise {
		clk_cnt_reg := UInt(0)
		CLK_reg := UInt(0)
	}
}

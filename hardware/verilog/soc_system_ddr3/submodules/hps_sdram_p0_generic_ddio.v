// (C) 2001-2016 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.



`timescale 1 ps / 1 ps

module hps_sdram_p0_generic_ddio(
	datain,
	halfratebypass,
	dataout,
	clk_hr,
	clk_fr
);

	parameter WIDTH = 1;
	localparam DATA_IN_WIDTH = 4 * WIDTH;
	localparam DATA_OUT_WIDTH = WIDTH;

	input [DATA_IN_WIDTH-1:0] datain;
	input halfratebypass;
	input [WIDTH-1:0] clk_hr;
	input [WIDTH-1:0] clk_fr;
	output [DATA_OUT_WIDTH-1:0] dataout;


	generate
	genvar pin;
	for (pin = 0; pin < WIDTH; pin = pin + 1)
	begin:acblock
		wire fr_data_hi;
		wire fr_data_lo;

		cyclonev_ddio_out
		#(
			.half_rate_mode("true"),
			.use_new_clocking_model("true"),
			.async_mode("none")
		) hr_to_fr_hi (							    
			.datainhi(datain[pin * 4]),
			.datainlo(datain[pin * 4 + 2]),
			.dataout(fr_data_hi),
			.clkhi (clk_hr[pin]),
			.clklo (clk_hr[pin]),
			.hrbypass(halfratebypass),
			.muxsel (clk_hr[pin])
		);
		
		cyclonev_ddio_out
		#(
			.half_rate_mode("true"),
			.use_new_clocking_model("true"),
			.async_mode("none")
		) hr_to_fr_lo (							    
			.datainhi(datain[pin * 4 + 1]),
			.datainlo(datain[pin * 4 + 3]),
			.dataout(fr_data_lo),
			.clkhi (clk_hr[pin]),
			.clklo (clk_hr[pin]),
			.hrbypass(halfratebypass),
			.muxsel (clk_hr[pin])
		);
		
		cyclonev_ddio_out
		#(
			.async_mode("none"),
			.half_rate_mode("false"),
			.sync_mode("none"),
			.use_new_clocking_model("true")
		) ddio_out (
			.datainhi(fr_data_hi),  
			.datainlo(fr_data_lo),
			.dataout(dataout[pin]),
			.clkhi (clk_fr[pin]),
			.clklo (clk_fr[pin]),
			.muxsel (clk_fr[pin])
		);
	end
	endgenerate
endmodule

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

module hps_sdram_p0_acv_hard_addr_cmd_pads(
	/*
    config_data_in, 
    config_clock_in, 
    config_io_ena, 
    config_update, 
	*/
    reset_n,
    reset_n_afi_clk,
    pll_hr_clk, 
    pll_avl_phy_clk,
    pll_afi_clk,
    pll_mem_clk,
    pll_write_clk,
    phy_ddio_address,
    dll_delayctrl_in,
    phy_ddio_bank,
    phy_ddio_cs_n,
    phy_ddio_cke,
    phy_ddio_odt,
    phy_ddio_we_n,
    phy_ddio_ras_n,
    phy_ddio_cas_n,
    phy_ddio_ck,
    phy_ddio_reset_n,
    phy_mem_address,
    phy_mem_bank,
    phy_mem_cs_n,
    phy_mem_cke,
    phy_mem_odt,
    phy_mem_we_n,
    phy_mem_ras_n,
    phy_mem_cas_n,
    phy_mem_reset_n,
    phy_mem_ck,
    phy_mem_ck_n
);

	parameter DEVICE_FAMILY = "";
	parameter MEM_ADDRESS_WIDTH     = ""; 
	parameter MEM_BANK_WIDTH        = ""; 
	parameter MEM_CHIP_SELECT_WIDTH = ""; 
	parameter MEM_CLK_EN_WIDTH 		= ""; 
	parameter MEM_CK_WIDTH 			= ""; 
	parameter MEM_ODT_WIDTH 		= ""; 
	parameter MEM_CONTROL_WIDTH     = ""; 

	parameter AFI_ADDRESS_WIDTH         = ""; 
	parameter AFI_BANK_WIDTH            = ""; 
	parameter AFI_CHIP_SELECT_WIDTH     = ""; 
	parameter AFI_CLK_EN_WIDTH 			= ""; 
	parameter AFI_ODT_WIDTH 			= ""; 
	parameter AFI_CONTROL_WIDTH         = ""; 
	parameter DLL_WIDTH = "";
	parameter ADC_PHASE_SETTING = "";
	parameter ADC_INVERT_PHASE = "";
	parameter IS_HHP_HPS = "";

	/*
    input config_data_in; 
    input config_clock_in; 
    input config_io_ena; 
    input config_update; 
	*/
	input	reset_n;
	input	reset_n_afi_clk;
	input	pll_afi_clk;
	input   pll_hr_clk;
	input   pll_avl_phy_clk;
	input	pll_mem_clk;
	input	pll_write_clk;
	input 	[DLL_WIDTH-1:0] dll_delayctrl_in;

	input	[AFI_ADDRESS_WIDTH-1:0]	phy_ddio_address;

	input   [AFI_BANK_WIDTH-1:0]    phy_ddio_bank;
	input   [AFI_CHIP_SELECT_WIDTH-1:0] phy_ddio_cs_n;
	input   [AFI_CLK_EN_WIDTH-1:0] phy_ddio_cke;
	input   [AFI_ODT_WIDTH-1:0] phy_ddio_odt;
	input   [AFI_CONTROL_WIDTH-1:0] phy_ddio_ras_n;
	input   [AFI_CONTROL_WIDTH-1:0] phy_ddio_cas_n;
	input   [AFI_CONTROL_WIDTH-1:0] phy_ddio_ck;
	input   [AFI_CONTROL_WIDTH-1:0] phy_ddio_we_n;
	input   [AFI_CONTROL_WIDTH-1:0] phy_ddio_reset_n;

	output  [MEM_ADDRESS_WIDTH-1:0] phy_mem_address;
	output  [MEM_BANK_WIDTH-1:0]    phy_mem_bank;
	output  [MEM_CHIP_SELECT_WIDTH-1:0] phy_mem_cs_n;
	output  [MEM_CLK_EN_WIDTH-1:0] phy_mem_cke;
	output  [MEM_ODT_WIDTH-1:0] phy_mem_odt;
	output  [MEM_CONTROL_WIDTH-1:0] phy_mem_we_n;
	output  [MEM_CONTROL_WIDTH-1:0] phy_mem_ras_n;
	output  [MEM_CONTROL_WIDTH-1:0] phy_mem_cas_n;
	output  phy_mem_reset_n;
	output  [MEM_CK_WIDTH-1:0]	phy_mem_ck;
	output  [MEM_CK_WIDTH-1:0]	phy_mem_ck_n;

	/* ********* *
	 * A/C Logic *
	 * ********* */

	localparam CMD_WIDTH = 
		MEM_CHIP_SELECT_WIDTH + 
		MEM_CLK_EN_WIDTH + 
		MEM_ODT_WIDTH + 
		MEM_CONTROL_WIDTH + 
		MEM_CONTROL_WIDTH + 
		MEM_CONTROL_WIDTH; 
	
	localparam AC_CLK_WIDTH = MEM_ADDRESS_WIDTH + MEM_BANK_WIDTH + CMD_WIDTH + 1;

	localparam IMPLEMENT_MEM_CLK_IN_SOFT_LOGIC = "false";
	
	wire [AC_CLK_WIDTH-1:0] ac_clk;
	generate
	genvar i;
	for (i = 0; i < AC_CLK_WIDTH; i = i + 1)
	begin: address_gen
		wire addr_cmd_clk;
		hps_sdram_p0_acv_ldc # (
			.DLL_DELAY_CTRL_WIDTH(DLL_WIDTH),
			.ADC_PHASE_SETTING(ADC_PHASE_SETTING),
			.ADC_INVERT_PHASE(ADC_INVERT_PHASE),
			.IS_HHP_HPS(IS_HHP_HPS)
		) acv_ac_ldc (
			.pll_hr_clk(pll_avl_phy_clk),
			.pll_dq_clk(pll_write_clk),
			.pll_dqs_clk (pll_mem_clk),
			.dll_phy_delayctrl(dll_delayctrl_in),
			.adc_clk_cps(ac_clk[i])
		);
	end
	endgenerate
	
	hps_sdram_p0_generic_ddio uaddress_pad(
		.datain(phy_ddio_address),
		.halfratebypass(1'b1), 
		.dataout(phy_mem_address),
		.clk_hr({MEM_ADDRESS_WIDTH{pll_hr_clk}}),
		.clk_fr(ac_clk[MEM_ADDRESS_WIDTH-1:0])
	);
	defparam uaddress_pad.WIDTH = MEM_ADDRESS_WIDTH;
	
	hps_sdram_p0_generic_ddio ubank_pad(
		.datain(phy_ddio_bank),
		.halfratebypass(1'b1), 
		.dataout(phy_mem_bank),
		.clk_hr({MEM_BANK_WIDTH{pll_hr_clk}}),
		.clk_fr(ac_clk[MEM_ADDRESS_WIDTH + MEM_BANK_WIDTH - 1: MEM_ADDRESS_WIDTH])
	);
	defparam ubank_pad.WIDTH = MEM_BANK_WIDTH;

	hps_sdram_p0_generic_ddio ucmd_pad(
		.datain({ 
			phy_ddio_we_n,
			phy_ddio_cas_n,
			phy_ddio_ras_n,
			phy_ddio_odt,
			phy_ddio_cke,
			phy_ddio_cs_n
		}),
		.halfratebypass(1'b1), 
		.dataout({ 
			phy_mem_we_n,
			phy_mem_cas_n,
			phy_mem_ras_n,
			phy_mem_odt,
			phy_mem_cke,
			phy_mem_cs_n
		}),
		.clk_hr({CMD_WIDTH{pll_hr_clk}}),
		.clk_fr(ac_clk[MEM_ADDRESS_WIDTH + MEM_BANK_WIDTH + CMD_WIDTH - 1: MEM_ADDRESS_WIDTH + MEM_BANK_WIDTH])
	);
	defparam ucmd_pad.WIDTH = CMD_WIDTH;
	
	hps_sdram_p0_generic_ddio ureset_n_pad(
		.datain(phy_ddio_reset_n),
		.halfratebypass(1'b1), 
		.dataout(phy_mem_reset_n),
		.clk_hr(pll_hr_clk),
		.clk_fr(ac_clk[MEM_ADDRESS_WIDTH + MEM_BANK_WIDTH + CMD_WIDTH])
	);
	defparam ureset_n_pad.WIDTH = 1;

	/* ************ *
	 * Config Logic *
	 * ************ */

	wire [4:0] outputdelaysetting;
	wire [4:0] outputenabledelaysetting;
	wire outputhalfratebypass;
	wire [4:0] inputdelaysetting;
		
	wire [1:0] rfifo_clock_select;
	wire [2:0] rfifo_mode;
	
	/* 	
	cyclonev_io_config ioconfig (
	    .datain(config_data_in),          
	    .clk(config_clock_in),
	    .ena(config_io_ena),
	    .update(config_update),       

	    .outputregdelaysetting(outputdelaysetting), 
	    .outputenabledelaysetting(outputenabledelaysetting),
	    .outputhalfratebypass(outputhalfratebypass),
	    .readfiforeadclockselect(rfifo_clock_select),
	    .readfifomode(rfifo_mode),
	    
	    .padtoinputregisterdelaysetting(inputdelaysetting),
	    .dataout()
	);
	*/

	/* *************** *
	 * Mem Clock Logic *
	 * *************** */

	wire    [MEM_CK_WIDTH-1:0] mem_ck_source;
	wire	[MEM_CK_WIDTH-1:0] mem_ck;	

	generate
	genvar clock_width;
    	for (clock_width=0; clock_width<MEM_CK_WIDTH; clock_width=clock_width+1)
    	begin: clock_gen

	if(IMPLEMENT_MEM_CLK_IN_SOFT_LOGIC == "true")
	begin
		hps_sdram_p0_acv_ldc # (
			.DLL_DELAY_CTRL_WIDTH(DLL_WIDTH),
			.ADC_PHASE_SETTING(ADC_PHASE_SETTING),
			.ADC_INVERT_PHASE(ADC_INVERT_PHASE),
			.IS_HHP_HPS(IS_HHP_HPS)
		) acv_ck_ldc (
			.pll_hr_clk(pll_avl_phy_clk),
			.pll_dq_clk(pll_write_clk),
			.pll_dqs_clk (pll_mem_clk),
			.dll_phy_delayctrl(dll_delayctrl_in),
			.adc_clk_cps(mem_ck_source[clock_width])
		);
	end
	else
	begin
		wire [3:0] phy_clk_in;
		wire [3:0] phy_clk_out;
		assign phy_clk_in = {pll_avl_phy_clk,pll_write_clk,pll_mem_clk,1'b0};

		if (IS_HHP_HPS == "true") begin
			assign phy_clk_out = phy_clk_in;
		end else begin
			cyclonev_phy_clkbuf phy_clkbuf (
	        		.inclk (phy_clk_in),
	        		.outclk (phy_clk_out)
			);
		end
	
		wire [3:0] leveled_dqs_clocks;
		cyclonev_leveling_delay_chain leveling_delay_chain_dqs (
			.clkin (phy_clk_out[1]),
			.delayctrlin (dll_delayctrl_in),
			.clkout(leveled_dqs_clocks)
		);
		defparam leveling_delay_chain_dqs.physical_clock_source = "DQS";
		
		cyclonev_clk_phase_select clk_phase_select_dqs (
			`ifndef SIMGEN
			.clkin (leveled_dqs_clocks[0]),
			`else
			.clkin (leveled_dqs_clocks),
			`endif
			.clkout (mem_ck_source[clock_width])
		);
		defparam clk_phase_select_dqs.physical_clock_source = "DQS";
		defparam clk_phase_select_dqs.use_phasectrlin = "false";
		defparam clk_phase_select_dqs.phase_setting = 0;
	end

	wire mem_ck_hi;
	wire mem_ck_lo;

	if(IMPLEMENT_MEM_CLK_IN_SOFT_LOGIC == "true")
	begin
		assign mem_ck_hi = 1'b0;
		assign mem_ck_lo = 1'b1;
	end
	else
	begin
		assign mem_ck_hi = phy_ddio_ck[0];
		assign mem_ck_lo = phy_ddio_ck[1];
	end

    	altddio_out umem_ck_pad(
    		.aclr       (1'b0),
    		.aset       (1'b0),
    		.datain_h   (mem_ck_hi),
    		.datain_l   (mem_ck_lo),
    		.dataout    (mem_ck[clock_width]),
    		.oe         (1'b1),
    		.outclock   (mem_ck_source[clock_width]),
    		.outclocken (1'b1)
    	);

    	defparam
    		umem_ck_pad.extend_oe_disable = "UNUSED",
    		umem_ck_pad.intended_device_family = DEVICE_FAMILY,
    		umem_ck_pad.invert_output = "OFF",
    		umem_ck_pad.lpm_hint = "UNUSED",
    		umem_ck_pad.lpm_type = "altddio_out",
    		umem_ck_pad.oe_reg = "UNUSED",
    		umem_ck_pad.power_up_high = "OFF",
    		umem_ck_pad.width = 1;

		wire mem_ck_temp;

		assign mem_ck_temp = mem_ck[clock_width];

    	hps_sdram_p0_clock_pair_generator    uclk_generator(
        	.datain     (mem_ck_temp),
        	.dataout    (phy_mem_ck[clock_width]),
        	.dataout_b  (phy_mem_ck_n[clock_width])
    	);
	end
endgenerate


endmodule

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

module hps_sdram_p0_acv_hard_io_pads(
	reset_n_addr_cmd_clk,
	reset_n_afi_clk,
	oct_ctl_rs_value,
	oct_ctl_rt_value,
	phy_ddio_address,
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
	pll_afi_clk,
	pll_afi_phy_clk,
	pll_avl_phy_clk,
	pll_avl_clk,
	avl_clk,
	pll_mem_clk,
	pll_mem_phy_clk,
	pll_write_clk,
	pll_dqs_ena_clk,
	pll_addr_cmd_clk,
	phy_mem_dq,
	phy_mem_dm,
	phy_mem_ck,
	phy_mem_ck_n,
	mem_dqs,
	mem_dqs_n,
	dll_phy_delayctrl,
	scc_clk,
	scc_data,
	scc_dqs_ena,
	scc_dqs_io_ena,
	scc_dq_ena,
	scc_dm_ena,
	scc_upd,
	seq_read_latency_counter,
	seq_read_increment_vfifo_fr,
	seq_read_increment_vfifo_hr,
	phy_ddio_dmdout,
	phy_ddio_dqdout,
	phy_ddio_dqs_oe,
	phy_ddio_dqsdout,
	phy_ddio_dqsb_oe,
	phy_ddio_dqslogic_oct,
	phy_ddio_dqslogic_fiforeset,
	phy_ddio_dqslogic_aclr_pstamble,
	phy_ddio_dqslogic_aclr_fifoctrl,
	phy_ddio_dqslogic_incwrptr,
	phy_ddio_dqslogic_readlatency,
	ddio_phy_dqslogic_rdatavalid,
	ddio_phy_dqdin,
	phy_ddio_dqslogic_incrdataen,
	phy_ddio_dqslogic_dqsena,
	phy_ddio_dqoe,
	capture_strobe_tracking
);


parameter DEVICE_FAMILY = "";
parameter FAST_SIM_MODEL            = 0;
parameter OCT_SERIES_TERM_CONTROL_WIDTH   = "";
parameter OCT_PARALLEL_TERM_CONTROL_WIDTH = "";
parameter MEM_ADDRESS_WIDTH     = "";
parameter MEM_BANK_WIDTH        = "";
parameter MEM_CHIP_SELECT_WIDTH = ""; 
parameter MEM_CLK_EN_WIDTH      = "";
parameter MEM_CK_WIDTH          = "";
parameter MEM_ODT_WIDTH         = "";
parameter MEM_DQS_WIDTH         = "";
parameter MEM_DM_WIDTH          = "";
parameter MEM_CONTROL_WIDTH     = "";
parameter MEM_DQ_WIDTH          = "";
parameter MEM_READ_DQS_WIDTH    = "";
parameter MEM_WRITE_DQS_WIDTH   = "";
parameter DLL_DELAY_CTRL_WIDTH  = "";
parameter ADC_PHASE_SETTING     = "";
parameter ADC_INVERT_PHASE      = "";
parameter IS_HHP_HPS            = "";

localparam AFI_ADDRESS_WIDTH         = 64; 
localparam AFI_BANK_WIDTH            = 12; 
localparam AFI_CHIP_SELECT_WIDTH     = 8; 
localparam AFI_CLK_EN_WIDTH 			= 8; 
localparam AFI_ODT_WIDTH 			= 8; 
localparam AFI_DATA_MASK_WIDTH       = 20; 
localparam AFI_CONTROL_WIDTH         = 4; 

input	reset_n_afi_clk;
input	reset_n_addr_cmd_clk;

input   [OCT_SERIES_TERM_CONTROL_WIDTH-1:0] oct_ctl_rs_value;
input   [OCT_PARALLEL_TERM_CONTROL_WIDTH-1:0] oct_ctl_rt_value;

input	[AFI_ADDRESS_WIDTH-1:0]	phy_ddio_address;
input	[AFI_BANK_WIDTH-1:0]    phy_ddio_bank;
input	[AFI_CHIP_SELECT_WIDTH-1:0] phy_ddio_cs_n;
input	[AFI_CLK_EN_WIDTH-1:0] phy_ddio_cke;
input	[AFI_ODT_WIDTH-1:0] phy_ddio_odt;
input	[AFI_CONTROL_WIDTH-1:0] phy_ddio_ras_n;
input	[AFI_CONTROL_WIDTH-1:0] phy_ddio_cas_n;
input	[AFI_CONTROL_WIDTH-1:0] phy_ddio_ck;
input	[AFI_CONTROL_WIDTH-1:0] phy_ddio_we_n;
input	[AFI_CONTROL_WIDTH-1:0] phy_ddio_reset_n;

output  [MEM_ADDRESS_WIDTH-1:0]	phy_mem_address;
output	[MEM_BANK_WIDTH-1:0]	phy_mem_bank;
output	[MEM_CHIP_SELECT_WIDTH-1:0]	phy_mem_cs_n;
output  [MEM_CLK_EN_WIDTH-1:0]	phy_mem_cke;
output  [MEM_ODT_WIDTH-1:0]	phy_mem_odt;
output	[MEM_CONTROL_WIDTH-1:0]	phy_mem_we_n;
output	[MEM_CONTROL_WIDTH-1:0] phy_mem_ras_n;
output	[MEM_CONTROL_WIDTH-1:0] phy_mem_cas_n;
output	phy_mem_reset_n;

input	pll_afi_clk;
input pll_afi_phy_clk;
input pll_avl_phy_clk;
input pll_avl_clk;
input avl_clk;
input	pll_mem_clk;
input pll_mem_phy_clk;
input	pll_write_clk;
input	pll_dqs_ena_clk; 
input pll_addr_cmd_clk;


inout	[MEM_DQ_WIDTH-1:0]	phy_mem_dq;
output	[MEM_DM_WIDTH-1:0]	phy_mem_dm;
output	[MEM_CK_WIDTH-1:0]	phy_mem_ck;
output	[MEM_CK_WIDTH-1:0]	phy_mem_ck_n;
inout	[MEM_DQS_WIDTH-1:0]	mem_dqs;
inout	[MEM_DQS_WIDTH-1:0]	mem_dqs_n;

input   [DLL_DELAY_CTRL_WIDTH-1:0]  dll_phy_delayctrl;

input	scc_clk;
input	scc_data;
input	[MEM_READ_DQS_WIDTH - 1:0] scc_dqs_ena; 
input	[MEM_READ_DQS_WIDTH - 1:0] scc_dqs_io_ena; 
input	[MEM_DQ_WIDTH - 1:0] scc_dq_ena; 
input	[MEM_DM_WIDTH - 1:0] scc_dm_ena; 

input [4:0] seq_read_latency_counter;
input [MEM_READ_DQS_WIDTH-1:0] seq_read_increment_vfifo_fr;
input [MEM_READ_DQS_WIDTH-1:0] seq_read_increment_vfifo_hr;
input	scc_upd;
output	[MEM_READ_DQS_WIDTH - 1:0] capture_strobe_tracking;


input [24 : 0] phy_ddio_dmdout;
input [179 : 0] phy_ddio_dqdout;
input [9 : 0] phy_ddio_dqs_oe;
input [19 : 0] phy_ddio_dqsdout;
input [9 : 0] phy_ddio_dqsb_oe; 
input [9 : 0] phy_ddio_dqslogic_oct;
input [4 : 0] phy_ddio_dqslogic_fiforeset;
input [4 : 0] phy_ddio_dqslogic_aclr_pstamble;
input [4 : 0] phy_ddio_dqslogic_aclr_fifoctrl;
input [9 : 0] phy_ddio_dqslogic_incwrptr;
input [24 : 0] phy_ddio_dqslogic_readlatency;
output [4 : 0] ddio_phy_dqslogic_rdatavalid;
output [179 : 0] ddio_phy_dqdin;
input [9 : 0] phy_ddio_dqslogic_incrdataen; 
input [9 : 0] phy_ddio_dqslogic_dqsena; 
input [89 : 0] phy_ddio_dqoe;

wire	[MEM_DQ_WIDTH-1:0] mem_phy_dq;
wire	[DLL_DELAY_CTRL_WIDTH-1:0] read_bidir_dll_phy_delayctrl;
wire	[MEM_READ_DQS_WIDTH-1:0] bidir_read_dqs_bus_out;
wire	[MEM_DQ_WIDTH-1:0] bidir_read_dq_input_data_out_high;
wire	[MEM_DQ_WIDTH-1:0] bidir_read_dq_input_data_out_low;
wire	dqs_busout;

wire	hr_clk = pll_avl_clk;
wire	core_clk = pll_afi_clk;
wire	reset_n_core_clk = reset_n_afi_clk;

	hps_sdram_p0_acv_hard_addr_cmd_pads uaddr_cmd_pads(
	/*
    	.config_data_in(config_data_in), 
    	.config_clock_in(config_clock_in), 
    	.config_io_ena(config_io_ena), 
    	.config_update(config_update), 
		*/
		.reset_n				(reset_n_addr_cmd_clk),
		.reset_n_afi_clk		(reset_n_afi_clk),
		.pll_afi_clk            (pll_afi_phy_clk),
		.pll_mem_clk            (pll_mem_phy_clk),
		.pll_hr_clk				(hr_clk),
		.pll_avl_phy_clk		(pll_avl_phy_clk),
		.pll_write_clk			(pll_write_clk),
		.dll_delayctrl_in		(dll_phy_delayctrl),
		.phy_ddio_address 		(phy_ddio_address),
		.phy_ddio_bank		    (phy_ddio_bank),
		.phy_ddio_cs_n		    (phy_ddio_cs_n),
		.phy_ddio_cke			(phy_ddio_cke),
		.phy_ddio_odt			(phy_ddio_odt),
		.phy_ddio_we_n		    (phy_ddio_we_n),	
		.phy_ddio_ras_n		    (phy_ddio_ras_n),
		.phy_ddio_cas_n		    (phy_ddio_cas_n),
		.phy_ddio_ck		    (phy_ddio_ck),
		.phy_ddio_reset_n		(phy_ddio_reset_n),

		.phy_mem_address		(phy_mem_address),
		.phy_mem_bank			(phy_mem_bank),
		.phy_mem_cs_n			(phy_mem_cs_n),
		.phy_mem_cke			(phy_mem_cke),
		.phy_mem_odt			(phy_mem_odt),
		.phy_mem_we_n			(phy_mem_we_n),
		.phy_mem_ras_n			(phy_mem_ras_n),
		.phy_mem_cas_n			(phy_mem_cas_n),
		.phy_mem_reset_n		(phy_mem_reset_n),
		.phy_mem_ck				(phy_mem_ck),
		.phy_mem_ck_n			(phy_mem_ck_n)
	);
	defparam uaddr_cmd_pads.DEVICE_FAMILY			= DEVICE_FAMILY;
	defparam uaddr_cmd_pads.MEM_ADDRESS_WIDTH		= MEM_ADDRESS_WIDTH;
	defparam uaddr_cmd_pads.MEM_BANK_WIDTH			= MEM_BANK_WIDTH;
	defparam uaddr_cmd_pads.MEM_CHIP_SELECT_WIDTH	= MEM_CHIP_SELECT_WIDTH;
	defparam uaddr_cmd_pads.MEM_CLK_EN_WIDTH		= MEM_CLK_EN_WIDTH;
	defparam uaddr_cmd_pads.MEM_CK_WIDTH			= MEM_CK_WIDTH;
	defparam uaddr_cmd_pads.MEM_ODT_WIDTH			= MEM_ODT_WIDTH;
	defparam uaddr_cmd_pads.MEM_CONTROL_WIDTH		= MEM_CONTROL_WIDTH;
	defparam uaddr_cmd_pads.AFI_ADDRESS_WIDTH       = MEM_ADDRESS_WIDTH * 4; 
	defparam uaddr_cmd_pads.AFI_BANK_WIDTH          = MEM_BANK_WIDTH * 4; 
	defparam uaddr_cmd_pads.AFI_CHIP_SELECT_WIDTH   = MEM_CHIP_SELECT_WIDTH * 4; 
	defparam uaddr_cmd_pads.AFI_CLK_EN_WIDTH        = MEM_CLK_EN_WIDTH * 4; 
	defparam uaddr_cmd_pads.AFI_ODT_WIDTH           = MEM_ODT_WIDTH * 4; 
	defparam uaddr_cmd_pads.AFI_CONTROL_WIDTH       = MEM_CONTROL_WIDTH * 4; 
	defparam uaddr_cmd_pads.DLL_WIDTH      		= DLL_DELAY_CTRL_WIDTH; 
	defparam uaddr_cmd_pads.ADC_PHASE_SETTING = ADC_PHASE_SETTING;
	defparam uaddr_cmd_pads.ADC_INVERT_PHASE = ADC_INVERT_PHASE;
	defparam uaddr_cmd_pads.IS_HHP_HPS       = IS_HHP_HPS;
		
	localparam NUM_OF_DQDQS = MEM_WRITE_DQS_WIDTH;
	localparam DQDQS_DATA_WIDTH = MEM_DQ_WIDTH / NUM_OF_DQDQS;

	localparam NATIVE_GROUP_SIZE = 
		(DQDQS_DATA_WIDTH == 8) ? 9 : DQDQS_DATA_WIDTH;
	
	localparam DQDQS_DM_WIDTH = MEM_DM_WIDTH / MEM_WRITE_DQS_WIDTH;
		
	localparam NUM_OF_DQDQS_WITH_DM = MEM_WRITE_DQS_WIDTH;		
	
	generate
	genvar i;
	for (i=0; i<NUM_OF_DQDQS; i=i+1)
	begin: dq_ddio
			hps_sdram_p0_altdqdqs ubidir_dq_dqs (
				.write_strobe_clock_in (pll_mem_phy_clk),
				.reset_n_core_clock_in (reset_n_core_clk),
				.core_clock_in (core_clk),
				.fr_clock_in (pll_write_clk),
				.hr_clock_in (pll_avl_phy_clk),
				.parallelterminationcontrol_in(oct_ctl_rt_value),
				.seriesterminationcontrol_in(oct_ctl_rs_value),
				.strobe_ena_hr_clock_in (hr_clk),
				.capture_strobe_tracking (capture_strobe_tracking[i]),
				.read_write_data_io (phy_mem_dq[(DQDQS_DATA_WIDTH*(i+1)-1) : DQDQS_DATA_WIDTH*i]),
				.read_data_out (ddio_phy_dqdin[((NATIVE_GROUP_SIZE*i+DQDQS_DATA_WIDTH)*4-1) : (NATIVE_GROUP_SIZE*i*4)]),
				.capture_strobe_out(dqs_busout), 
				.extra_write_data_in (phy_ddio_dmdout[(i + 1) * 4 - 1 : (i * 4)]),
				.write_data_in (phy_ddio_dqdout[((NATIVE_GROUP_SIZE*i+DQDQS_DATA_WIDTH)*4-1) : (NATIVE_GROUP_SIZE*i*4)]),
				.write_oe_in (phy_ddio_dqoe[((NATIVE_GROUP_SIZE*i+DQDQS_DATA_WIDTH)*2-1) : (NATIVE_GROUP_SIZE*i*2)]),
				.strobe_io (mem_dqs[i]),
				.strobe_n_io (mem_dqs_n[i]),
				.output_strobe_ena(phy_ddio_dqs_oe[(i + 1) * 2 - 1 : (i * 2)]),
				.write_strobe(phy_ddio_dqsdout[(i + 1) * 4 - 1 : (i * 4)]),
				.oct_ena_in(phy_ddio_dqslogic_oct[(i + 1) * 2 - 1 : (i * 2)]),
				.extra_write_data_out (phy_mem_dm[i]),
				.config_data_in (scc_data),
				.config_dqs_ena (scc_dqs_ena[i]),
				.config_io_ena (scc_dq_ena[(DQDQS_DATA_WIDTH*(i+1)-1) : DQDQS_DATA_WIDTH*i]),
				.config_dqs_io_ena (scc_dqs_io_ena[i]),
				.config_update (scc_upd),
				.config_clock_in (scc_clk),
				.config_extra_io_ena (scc_dm_ena[i]),
				.lfifo_rdata_en(phy_ddio_dqslogic_incrdataen[(i + 1) * 2 - 1 : (i * 2)]),
				.lfifo_rdata_en_full(phy_ddio_dqslogic_dqsena[(i + 1) * 2 - 1 : (i * 2)]),
				.lfifo_rd_latency(phy_ddio_dqslogic_readlatency[(i + 1) * 5 - 1 : (i * 5)]),
				.lfifo_reset_n (phy_ddio_dqslogic_aclr_fifoctrl[i]),
				.lfifo_rdata_valid(ddio_phy_dqslogic_rdatavalid[i]),
				.vfifo_qvld(phy_ddio_dqslogic_dqsena[(i + 1) * 2 - 1 : (i * 2)]),
				.vfifo_inc_wr_ptr(phy_ddio_dqslogic_incwrptr[(i + 1) * 2 - 1 : (i * 2)]),
				.vfifo_reset_n (phy_ddio_dqslogic_aclr_pstamble[i]),
				.dll_delayctrl_in (dll_phy_delayctrl),
				.rfifo_reset_n(phy_ddio_dqslogic_fiforeset[i])
				);
	end
	endgenerate

	generate
	genvar j;
	for (j = NUM_OF_DQDQS; j < 5; j=j+1)
	begin: to_vcc
		assign ddio_phy_dqslogic_rdatavalid[j] = 1'b1;
	end
	endgenerate

endmodule

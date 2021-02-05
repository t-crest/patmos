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


module soc_system_ddr3_0_hps_fpga_interfaces(
// h2f_reset
  output wire [1 - 1 : 0 ] h2f_rst_n
// f2h_cold_reset_req
 ,input wire [1 - 1 : 0 ] f2h_cold_rst_req_n
// f2h_warm_reset_req
 ,input wire [1 - 1 : 0 ] f2h_warm_rst_req_n
// f2h_sdram0_data
 ,input wire [28 - 1 : 0 ] f2h_sdram0_ADDRESS
 ,input wire [8 - 1 : 0 ] f2h_sdram0_BURSTCOUNT
 ,output wire [1 - 1 : 0 ] f2h_sdram0_WAITREQUEST
 ,output wire [128 - 1 : 0 ] f2h_sdram0_READDATA
 ,output wire [1 - 1 : 0 ] f2h_sdram0_READDATAVALID
 ,input wire [1 - 1 : 0 ] f2h_sdram0_READ
 ,input wire [128 - 1 : 0 ] f2h_sdram0_WRITEDATA
 ,input wire [16 - 1 : 0 ] f2h_sdram0_BYTEENABLE
 ,input wire [1 - 1 : 0 ] f2h_sdram0_WRITE
// f2h_sdram0_clock
 ,input wire [1 - 1 : 0 ] f2h_sdram0_clk
);


wire [11 - 1 : 0] intermediate;
assign intermediate[0:0] = ~intermediate[1:1];
assign intermediate[8:8] = intermediate[4:4]|intermediate[7:7];
assign intermediate[2:2] = intermediate[9:9];
assign intermediate[3:3] = intermediate[9:9];
assign intermediate[5:5] = intermediate[9:9];
assign intermediate[6:6] = intermediate[9:9];
assign intermediate[10:10] = intermediate[9:9];
assign f2h_sdram0_WAITREQUEST[0:0] = intermediate[0:0];
assign intermediate[4:4] = f2h_sdram0_READ[0:0];
assign intermediate[7:7] = f2h_sdram0_WRITE[0:0];
assign intermediate[9:9] = f2h_sdram0_clk[0:0];

cyclonev_hps_interface_clocks_resets clocks_resets(
 .f2h_warm_rst_req_n({
    f2h_warm_rst_req_n[0:0] // 0:0
  })
,.f2h_pending_rst_ack({
    1'b1 // 0:0
  })
,.f2h_dbg_rst_req_n({
    1'b1 // 0:0
  })
,.h2f_rst_n({
    h2f_rst_n[0:0] // 0:0
  })
,.f2h_cold_rst_req_n({
    f2h_cold_rst_req_n[0:0] // 0:0
  })
);


cyclonev_hps_interface_dbg_apb debug_apb(
 .DBG_APB_DISABLE({
    1'b0 // 0:0
  })
,.P_CLK_EN({
    1'b0 // 0:0
  })
);


cyclonev_hps_interface_tpiu_trace tpiu(
 .traceclk_ctl({
    1'b1 // 0:0
  })
);


cyclonev_hps_interface_boot_from_fpga boot_from_fpga(
 .boot_from_fpga_ready({
    1'b0 // 0:0
  })
,.boot_from_fpga_on_failure({
    1'b0 // 0:0
  })
,.bsel_en({
    1'b0 // 0:0
  })
,.csel_en({
    1'b0 // 0:0
  })
,.csel({
    2'b01 // 1:0
  })
,.bsel({
    3'b001 // 2:0
  })
);


cyclonev_hps_interface_fpga2hps fpga2hps(
 .port_size_config({
    2'b11 // 1:0
  })
);


cyclonev_hps_interface_hps2fpga hps2fpga(
 .port_size_config({
    2'b11 // 1:0
  })
);


cyclonev_hps_interface_fpga2sdram f2sdram(
 .cmd_data_0({
    18'b000000000000000000 // 59:42
   ,f2h_sdram0_BURSTCOUNT[7:0] // 41:34
   ,4'b0000 // 33:30
   ,f2h_sdram0_ADDRESS[27:0] // 29:2
   ,intermediate[7:7] // 1:1
   ,intermediate[4:4] // 0:0
  })
,.cfg_port_width({
    12'b000000000010 // 11:0
  })
,.rd_valid_1({
    f2h_sdram0_READDATAVALID[0:0] // 0:0
  })
,.wr_clk_1({
    intermediate[6:6] // 0:0
  })
,.wr_clk_0({
    intermediate[5:5] // 0:0
  })
,.wr_data_1({
    2'b00 // 89:88
   ,f2h_sdram0_BYTEENABLE[15:8] // 87:80
   ,16'b0000000000000000 // 79:64
   ,f2h_sdram0_WRITEDATA[127:64] // 63:0
  })
,.cfg_cport_type({
    12'b000000000011 // 11:0
  })
,.wr_data_0({
    2'b00 // 89:88
   ,f2h_sdram0_BYTEENABLE[7:0] // 87:80
   ,16'b0000000000000000 // 79:64
   ,f2h_sdram0_WRITEDATA[63:0] // 63:0
  })
,.cfg_rfifo_cport_map({
    16'b0000000000000000 // 15:0
  })
,.cfg_cport_wfifo_map({
    18'b000000000000000000 // 17:0
  })
,.cmd_port_clk_0({
    intermediate[10:10] // 0:0
  })
,.cfg_cport_rfifo_map({
    18'b000000000000000000 // 17:0
  })
,.rd_ready_1({
    1'b1 // 0:0
  })
,.rd_ready_0({
    1'b1 // 0:0
  })
,.rd_clk_1({
    intermediate[3:3] // 0:0
  })
,.cmd_ready_0({
    intermediate[1:1] // 0:0
  })
,.rd_clk_0({
    intermediate[2:2] // 0:0
  })
,.cfg_wfifo_cport_map({
    16'b0000000000000000 // 15:0
  })
,.wrack_ready_0({
    1'b1 // 0:0
  })
,.cmd_valid_0({
    intermediate[8:8] // 0:0
  })
,.rd_data_1({
    f2h_sdram0_READDATA[127:64] // 63:0
  })
,.rd_data_0({
    f2h_sdram0_READDATA[63:0] // 63:0
  })
,.cfg_axi_mm_select({
    6'b000000 // 5:0
  })
);

endmodule


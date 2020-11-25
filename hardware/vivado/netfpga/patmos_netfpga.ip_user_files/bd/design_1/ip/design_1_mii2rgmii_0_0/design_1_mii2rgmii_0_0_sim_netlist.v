// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
// Date        : Fri Oct 30 13:07:56 2020
// Host        : DESKTOP-SS2DKB0 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode funcsim
//               C:/Users/chris-PC/OneDrive/1DTU/AAATHESIS/patmos_netfpga/patmos_netfpga/patmos_netfpga.srcs/sources_1/bd/design_1/ip/design_1_mii2rgmii_0_0/design_1_mii2rgmii_0_0_sim_netlist.v
// Design      : design_1_mii2rgmii_0_0
// Purpose     : This verilog netlist is a functional simulation representation of the design and should not be modified
//               or synthesized. This netlist cannot be used for SDF annotated simulation.
// Device      : xc7k325tffg676-1
// --------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

(* CHECK_LICENSE_TYPE = "design_1_mii2rgmii_0_0,mii2rgmii,{}" *) (* downgradeipidentifiedwarnings = "yes" *) (* ip_definition_source = "module_ref" *) 
(* x_core_info = "mii2rgmii,Vivado 2019.2" *) 
(* NotValidForBitStream *)
module design_1_mii2rgmii_0_0
   (rgmii_clk,
    mii_phy_tx_data,
    mii_phy_tx_en,
    mii_phy_tx_er,
    mii_phy_rx_data,
    mii_phy_dv,
    mii_phy_rx_er,
    mii_phy_crs,
    mii_phy_col,
    rgmii_phy_txc,
    rgmii_phy_txd,
    rgmii_phy_tx_ctl,
    rgmii_phy_rxc,
    rgmii_phy_rxd,
    rgmii_phy_rx_ctl);
  (* x_interface_info = "xilinx.com:signal:clock:1.0 rgmii_clk CLK" *) (* x_interface_parameter = "XIL_INTERFACENAME rgmii_clk, FREQ_HZ 25000000, PHASE 0.0, CLK_DOMAIN /clk_wiz_1_clk_out1, INSERT_VIP 0" *) input rgmii_clk;
  input [3:0]mii_phy_tx_data;
  input mii_phy_tx_en;
  input mii_phy_tx_er;
  output [3:0]mii_phy_rx_data;
  output mii_phy_dv;
  output mii_phy_rx_er;
  output mii_phy_crs;
  output mii_phy_col;
  output rgmii_phy_txc;
  output [3:0]rgmii_phy_txd;
  output rgmii_phy_tx_ctl;
  input rgmii_phy_rxc;
  input [3:0]rgmii_phy_rxd;
  input rgmii_phy_rx_ctl;

  wire \<const0> ;
  wire mii_phy_dv;
  wire [3:0]mii_phy_tx_data;
  wire mii_phy_tx_en;
  wire mii_phy_tx_er;
  wire rgmii_clk;
  wire rgmii_phy_rx_ctl;
  wire [3:0]rgmii_phy_rxd;
  wire rgmii_phy_tx_ctl;

  assign mii_phy_col = \<const0> ;
  assign mii_phy_crs = \<const0> ;
  assign mii_phy_rx_data[3:0] = rgmii_phy_rxd;
  assign rgmii_phy_txd[3:0] = mii_phy_tx_data;
  GND GND
       (.G(\<const0> ));
  design_1_mii2rgmii_0_0_mii2rgmii U0
       (.mii_phy_dv(mii_phy_dv),
        .mii_phy_tx_en(mii_phy_tx_en),
        .mii_phy_tx_er(mii_phy_tx_er),
        .rgmii_clk(rgmii_clk),
        .rgmii_phy_rx_ctl(rgmii_phy_rx_ctl),
        .rgmii_phy_tx_ctl(rgmii_phy_tx_ctl));
endmodule

(* ORIG_REF_NAME = "mii2rgmii" *) 
module design_1_mii2rgmii_0_0_mii2rgmii
   (mii_phy_dv,
    rgmii_phy_tx_ctl,
    rgmii_phy_rx_ctl,
    rgmii_clk,
    mii_phy_tx_er,
    mii_phy_tx_en);
  output mii_phy_dv;
  output rgmii_phy_tx_ctl;
  input rgmii_phy_rx_ctl;
  input rgmii_clk;
  input mii_phy_tx_er;
  input mii_phy_tx_en;

  wire mii_phy_dv;
  wire mii_phy_tx_en;
  wire mii_phy_tx_er;
  wire rgmii_clk;
  wire rgmii_phy_rx_ctl;
  wire rgmii_phy_tx_ctl;
  wire rgmii_phy_tx_ctl_i_1_n_0;

  FDRE #(
    .IS_C_INVERTED(1'b1)) 
    mii_phy_dv_reg
       (.C(rgmii_clk),
        .CE(1'b1),
        .D(rgmii_phy_rx_ctl),
        .Q(mii_phy_dv),
        .R(1'b0));
  LUT2 #(
    .INIT(4'h6)) 
    rgmii_phy_tx_ctl_i_1
       (.I0(mii_phy_tx_er),
        .I1(mii_phy_tx_en),
        .O(rgmii_phy_tx_ctl_i_1_n_0));
  FDRE #(
    .IS_C_INVERTED(1'b1)) 
    rgmii_phy_tx_ctl_reg
       (.C(rgmii_clk),
        .CE(1'b1),
        .D(rgmii_phy_tx_ctl_i_1_n_0),
        .Q(rgmii_phy_tx_ctl),
        .R(1'b0));
endmodule
`ifndef GLBL
`define GLBL
`timescale  1 ps / 1 ps

module glbl ();

    parameter ROC_WIDTH = 100000;
    parameter TOC_WIDTH = 0;

//--------   STARTUP Globals --------------
    wire GSR;
    wire GTS;
    wire GWE;
    wire PRLD;
    tri1 p_up_tmp;
    tri (weak1, strong0) PLL_LOCKG = p_up_tmp;

    wire PROGB_GLBL;
    wire CCLKO_GLBL;
    wire FCSBO_GLBL;
    wire [3:0] DO_GLBL;
    wire [3:0] DI_GLBL;
   
    reg GSR_int;
    reg GTS_int;
    reg PRLD_int;

//--------   JTAG Globals --------------
    wire JTAG_TDO_GLBL;
    wire JTAG_TCK_GLBL;
    wire JTAG_TDI_GLBL;
    wire JTAG_TMS_GLBL;
    wire JTAG_TRST_GLBL;

    reg JTAG_CAPTURE_GLBL;
    reg JTAG_RESET_GLBL;
    reg JTAG_SHIFT_GLBL;
    reg JTAG_UPDATE_GLBL;
    reg JTAG_RUNTEST_GLBL;

    reg JTAG_SEL1_GLBL = 0;
    reg JTAG_SEL2_GLBL = 0 ;
    reg JTAG_SEL3_GLBL = 0;
    reg JTAG_SEL4_GLBL = 0;

    reg JTAG_USER_TDO1_GLBL = 1'bz;
    reg JTAG_USER_TDO2_GLBL = 1'bz;
    reg JTAG_USER_TDO3_GLBL = 1'bz;
    reg JTAG_USER_TDO4_GLBL = 1'bz;

    assign (strong1, weak0) GSR = GSR_int;
    assign (strong1, weak0) GTS = GTS_int;
    assign (weak1, weak0) PRLD = PRLD_int;

    initial begin
	GSR_int = 1'b1;
	PRLD_int = 1'b1;
	#(ROC_WIDTH)
	GSR_int = 1'b0;
	PRLD_int = 1'b0;
    end

    initial begin
	GTS_int = 1'b1;
	#(TOC_WIDTH)
	GTS_int = 1'b0;
    end

endmodule
`endif

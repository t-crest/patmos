// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
// Date        : Fri Oct 30 13:07:56 2020
// Host        : DESKTOP-SS2DKB0 running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               C:/Users/chris-PC/OneDrive/1DTU/AAATHESIS/patmos_netfpga/patmos_netfpga/patmos_netfpga.srcs/sources_1/bd/design_1/ip/design_1_mii2rgmii_0_0/design_1_mii2rgmii_0_0_stub.v
// Design      : design_1_mii2rgmii_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7k325tffg676-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "mii2rgmii,Vivado 2019.2" *)
module design_1_mii2rgmii_0_0(rgmii_clk, mii_phy_tx_data, mii_phy_tx_en, 
  mii_phy_tx_er, mii_phy_rx_data, mii_phy_dv, mii_phy_rx_er, mii_phy_crs, mii_phy_col, 
  rgmii_phy_txc, rgmii_phy_txd, rgmii_phy_tx_ctl, rgmii_phy_rxc, rgmii_phy_rxd, 
  rgmii_phy_rx_ctl)
/* synthesis syn_black_box black_box_pad_pin="rgmii_clk,mii_phy_tx_data[3:0],mii_phy_tx_en,mii_phy_tx_er,mii_phy_rx_data[3:0],mii_phy_dv,mii_phy_rx_er,mii_phy_crs,mii_phy_col,rgmii_phy_txc,rgmii_phy_txd[3:0],rgmii_phy_tx_ctl,rgmii_phy_rxc,rgmii_phy_rxd[3:0],rgmii_phy_rx_ctl" */;
  input rgmii_clk;
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
endmodule

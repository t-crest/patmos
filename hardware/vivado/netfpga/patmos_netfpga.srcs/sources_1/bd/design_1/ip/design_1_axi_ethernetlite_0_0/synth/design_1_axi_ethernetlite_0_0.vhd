-- (c) Copyright 1995-2020 Xilinx, Inc. All rights reserved.
-- 
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
-- 
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
-- 
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
-- 
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
-- 
-- DO NOT MODIFY THIS FILE.

-- IP VLNV: xilinx.com:ip:axi_ethernetlite:3.0
-- IP Revision: 18

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

LIBRARY axi_ethernetlite_v3_0_18;
USE axi_ethernetlite_v3_0_18.axi_ethernetlite;

ENTITY design_1_axi_ethernetlite_0_0 IS
  PORT (
    s_axi_aclk : IN STD_LOGIC;
    s_axi_aresetn : IN STD_LOGIC;
    ip2intc_irpt : OUT STD_LOGIC;
    s_axi_awaddr : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
    s_axi_awvalid : IN STD_LOGIC;
    s_axi_awready : OUT STD_LOGIC;
    s_axi_wdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axi_wstrb : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
    s_axi_wvalid : IN STD_LOGIC;
    s_axi_wready : OUT STD_LOGIC;
    s_axi_bresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    s_axi_bvalid : OUT STD_LOGIC;
    s_axi_bready : IN STD_LOGIC;
    s_axi_araddr : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
    s_axi_arvalid : IN STD_LOGIC;
    s_axi_arready : OUT STD_LOGIC;
    s_axi_rdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    s_axi_rresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    s_axi_rvalid : OUT STD_LOGIC;
    s_axi_rready : IN STD_LOGIC;
    phy_tx_clk : IN STD_LOGIC;
    phy_rx_clk : IN STD_LOGIC;
    phy_crs : IN STD_LOGIC;
    phy_dv : IN STD_LOGIC;
    phy_rx_data : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
    phy_col : IN STD_LOGIC;
    phy_rx_er : IN STD_LOGIC;
    phy_rst_n : OUT STD_LOGIC;
    phy_tx_en : OUT STD_LOGIC;
    phy_tx_data : OUT STD_LOGIC_VECTOR(3 DOWNTO 0)
  );
END design_1_axi_ethernetlite_0_0;

ARCHITECTURE design_1_axi_ethernetlite_0_0_arch OF design_1_axi_ethernetlite_0_0 IS
  ATTRIBUTE DowngradeIPIdentifiedWarnings : STRING;
  ATTRIBUTE DowngradeIPIdentifiedWarnings OF design_1_axi_ethernetlite_0_0_arch: ARCHITECTURE IS "yes";
  COMPONENT axi_ethernetlite IS
    GENERIC (
      C_FAMILY : STRING;
      C_SELECT_XPM : INTEGER;
      C_INSTANCE : STRING;
      C_S_AXI_ACLK_PERIOD_PS : INTEGER;
      C_S_AXI_ADDR_WIDTH : INTEGER;
      C_S_AXI_DATA_WIDTH : INTEGER;
      C_S_AXI_ID_WIDTH : INTEGER;
      C_S_AXI_PROTOCOL : STRING;
      C_INCLUDE_MDIO : INTEGER;
      C_INCLUDE_INTERNAL_LOOPBACK : INTEGER;
      C_INCLUDE_GLOBAL_BUFFERS : INTEGER;
      C_DUPLEX : INTEGER;
      C_TX_PING_PONG : INTEGER;
      C_RX_PING_PONG : INTEGER
    );
    PORT (
      s_axi_aclk : IN STD_LOGIC;
      s_axi_aresetn : IN STD_LOGIC;
      ip2intc_irpt : OUT STD_LOGIC;
      s_axi_awid : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_awaddr : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
      s_axi_awlen : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      s_axi_awsize : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_awburst : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_awcache : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_awvalid : IN STD_LOGIC;
      s_axi_awready : OUT STD_LOGIC;
      s_axi_wdata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_wstrb : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_wlast : IN STD_LOGIC;
      s_axi_wvalid : IN STD_LOGIC;
      s_axi_wready : OUT STD_LOGIC;
      s_axi_bid : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_bresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_bvalid : OUT STD_LOGIC;
      s_axi_bready : IN STD_LOGIC;
      s_axi_arid : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_araddr : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
      s_axi_arlen : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
      s_axi_arsize : IN STD_LOGIC_VECTOR(2 DOWNTO 0);
      s_axi_arburst : IN STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_arcache : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      s_axi_arvalid : IN STD_LOGIC;
      s_axi_arready : OUT STD_LOGIC;
      s_axi_rid : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
      s_axi_rdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
      s_axi_rresp : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
      s_axi_rlast : OUT STD_LOGIC;
      s_axi_rvalid : OUT STD_LOGIC;
      s_axi_rready : IN STD_LOGIC;
      phy_tx_clk : IN STD_LOGIC;
      phy_rx_clk : IN STD_LOGIC;
      phy_crs : IN STD_LOGIC;
      phy_dv : IN STD_LOGIC;
      phy_rx_data : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      phy_col : IN STD_LOGIC;
      phy_rx_er : IN STD_LOGIC;
      phy_rst_n : OUT STD_LOGIC;
      phy_tx_en : OUT STD_LOGIC;
      phy_tx_data : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
      phy_mdio_i : IN STD_LOGIC;
      phy_mdio_o : OUT STD_LOGIC;
      phy_mdio_t : OUT STD_LOGIC;
      phy_mdc : OUT STD_LOGIC
    );
  END COMPONENT axi_ethernetlite;
  ATTRIBUTE X_CORE_INFO : STRING;
  ATTRIBUTE X_CORE_INFO OF design_1_axi_ethernetlite_0_0_arch: ARCHITECTURE IS "axi_ethernetlite,Vivado 2019.2";
  ATTRIBUTE CHECK_LICENSE_TYPE : STRING;
  ATTRIBUTE CHECK_LICENSE_TYPE OF design_1_axi_ethernetlite_0_0_arch : ARCHITECTURE IS "design_1_axi_ethernetlite_0_0,axi_ethernetlite,{}";
  ATTRIBUTE CORE_GENERATION_INFO : STRING;
  ATTRIBUTE CORE_GENERATION_INFO OF design_1_axi_ethernetlite_0_0_arch: ARCHITECTURE IS "design_1_axi_ethernetlite_0_0,axi_ethernetlite,{x_ipProduct=Vivado 2019.2,x_ipVendor=xilinx.com,x_ipLibrary=ip,x_ipName=axi_ethernetlite,x_ipVersion=3.0,x_ipCoreRevision=18,x_ipLanguage=VHDL,x_ipSimLanguage=MIXED,C_FAMILY=kintex7,C_SELECT_XPM=1,C_INSTANCE=axi_ethernetlite_inst,C_S_AXI_ACLK_PERIOD_PS=10000,C_S_AXI_ADDR_WIDTH=13,C_S_AXI_DATA_WIDTH=32,C_S_AXI_ID_WIDTH=1,C_S_AXI_PROTOCOL=AXI4LITE,C_INCLUDE_MDIO=0,C_INCLUDE_INTERNAL_LOOPBACK=0,C_INCLUDE_GLOBAL_BUFFERS=1,C_DUPLEX=1,C_TX_PING_PONG=0,C_" & 
"RX_PING_PONG=0}";
  ATTRIBUTE X_INTERFACE_INFO : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
  ATTRIBUTE X_INTERFACE_INFO OF phy_tx_data: SIGNAL IS "xilinx.com:interface:mii:1.0 MII TXD";
  ATTRIBUTE X_INTERFACE_INFO OF phy_tx_en: SIGNAL IS "xilinx.com:interface:mii:1.0 MII TX_EN";
  ATTRIBUTE X_INTERFACE_INFO OF phy_rst_n: SIGNAL IS "xilinx.com:interface:mii:1.0 MII RST_N";
  ATTRIBUTE X_INTERFACE_INFO OF phy_rx_er: SIGNAL IS "xilinx.com:interface:mii:1.0 MII RX_ER";
  ATTRIBUTE X_INTERFACE_INFO OF phy_col: SIGNAL IS "xilinx.com:interface:mii:1.0 MII COL";
  ATTRIBUTE X_INTERFACE_INFO OF phy_rx_data: SIGNAL IS "xilinx.com:interface:mii:1.0 MII RXD";
  ATTRIBUTE X_INTERFACE_INFO OF phy_dv: SIGNAL IS "xilinx.com:interface:mii:1.0 MII RX_DV";
  ATTRIBUTE X_INTERFACE_INFO OF phy_crs: SIGNAL IS "xilinx.com:interface:mii:1.0 MII CRS";
  ATTRIBUTE X_INTERFACE_INFO OF phy_rx_clk: SIGNAL IS "xilinx.com:interface:mii:1.0 MII RX_CLK";
  ATTRIBUTE X_INTERFACE_PARAMETER OF phy_tx_clk: SIGNAL IS "XIL_INTERFACENAME MII, BOARD.ASSOCIATED_PARAM MII_BOARD_INTERFACE";
  ATTRIBUTE X_INTERFACE_INFO OF phy_tx_clk: SIGNAL IS "xilinx.com:interface:mii:1.0 MII TX_CLK";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rresp: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RRESP";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_rdata: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI RDATA";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_arready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_arvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_araddr: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI ARADDR";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_bresp: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI BRESP";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WVALID";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wstrb: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WSTRB";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_wdata: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI WDATA";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awready: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWREADY";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awvalid: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWVALID";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_awaddr: SIGNAL IS "XIL_INTERFACENAME S_AXI, DATA_WIDTH 32, PROTOCOL AXI4LITE, FREQ_HZ 100000000, ID_WIDTH 0, ADDR_WIDTH 13, AWUSER_WIDTH 0, ARUSER_WIDTH 0, WUSER_WIDTH 0, RUSER_WIDTH 0, BUSER_WIDTH 0, READ_WRITE_MODE READ_WRITE, HAS_BURST 0, HAS_LOCK 0, HAS_PROT 0, HAS_CACHE 0, HAS_QOS 0, HAS_REGION 0, HAS_WSTRB 1, HAS_BRESP 1, HAS_RRESP 1, SUPPORTS_NARROW_BURST 0, NUM_READ_OUTSTANDING 1, NUM_WRITE_OUTSTANDING 1, MAX_BURST_LENGTH 1, PHASE 0.0, CLK_DOMAIN design_1_clk_wiz_0_0_clk_out1, NUM_READ_THREADS 1, NUM_WRITE" & 
"_THREADS 1, RUSER_BITS_PER_BYTE 0, WUSER_BITS_PER_BYTE 0, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_awaddr: SIGNAL IS "xilinx.com:interface:aximm:1.0 S_AXI AWADDR";
  ATTRIBUTE X_INTERFACE_PARAMETER OF ip2intc_irpt: SIGNAL IS "XIL_INTERFACENAME interrupt, SUGGESTED_PRIORITY HIGH, SENSITIVITY EDGE_RISING, PortWidth 1";
  ATTRIBUTE X_INTERFACE_INFO OF ip2intc_irpt: SIGNAL IS "xilinx.com:signal:interrupt:1.0 interrupt INTERRUPT";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_aresetn: SIGNAL IS "XIL_INTERFACENAME s_axi_aresetn, POLARITY ACTIVE_LOW, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_aresetn: SIGNAL IS "xilinx.com:signal:reset:1.0 s_axi_aresetn RST";
  ATTRIBUTE X_INTERFACE_PARAMETER OF s_axi_aclk: SIGNAL IS "XIL_INTERFACENAME s_axi_aclk, ASSOCIATED_BUSIF S_AXI, ASSOCIATED_RESET s_axi_aresetn, FREQ_HZ 100000000, PHASE 0.0, CLK_DOMAIN design_1_clk_wiz_0_0_clk_out1, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF s_axi_aclk: SIGNAL IS "xilinx.com:signal:clock:1.0 s_axi_aclk CLK";
BEGIN
  U0 : axi_ethernetlite
    GENERIC MAP (
      C_FAMILY => "kintex7",
      C_SELECT_XPM => 1,
      C_INSTANCE => "axi_ethernetlite_inst",
      C_S_AXI_ACLK_PERIOD_PS => 10000,
      C_S_AXI_ADDR_WIDTH => 13,
      C_S_AXI_DATA_WIDTH => 32,
      C_S_AXI_ID_WIDTH => 1,
      C_S_AXI_PROTOCOL => "AXI4LITE",
      C_INCLUDE_MDIO => 0,
      C_INCLUDE_INTERNAL_LOOPBACK => 0,
      C_INCLUDE_GLOBAL_BUFFERS => 1,
      C_DUPLEX => 1,
      C_TX_PING_PONG => 0,
      C_RX_PING_PONG => 0
    )
    PORT MAP (
      s_axi_aclk => s_axi_aclk,
      s_axi_aresetn => s_axi_aresetn,
      ip2intc_irpt => ip2intc_irpt,
      s_axi_awid => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 1)),
      s_axi_awaddr => s_axi_awaddr,
      s_axi_awlen => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 8)),
      s_axi_awsize => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 3)),
      s_axi_awburst => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 2)),
      s_axi_awcache => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 4)),
      s_axi_awvalid => s_axi_awvalid,
      s_axi_awready => s_axi_awready,
      s_axi_wdata => s_axi_wdata,
      s_axi_wstrb => s_axi_wstrb,
      s_axi_wlast => '1',
      s_axi_wvalid => s_axi_wvalid,
      s_axi_wready => s_axi_wready,
      s_axi_bresp => s_axi_bresp,
      s_axi_bvalid => s_axi_bvalid,
      s_axi_bready => s_axi_bready,
      s_axi_arid => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 1)),
      s_axi_araddr => s_axi_araddr,
      s_axi_arlen => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 8)),
      s_axi_arsize => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 3)),
      s_axi_arburst => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 2)),
      s_axi_arcache => STD_LOGIC_VECTOR(TO_UNSIGNED(0, 4)),
      s_axi_arvalid => s_axi_arvalid,
      s_axi_arready => s_axi_arready,
      s_axi_rdata => s_axi_rdata,
      s_axi_rresp => s_axi_rresp,
      s_axi_rvalid => s_axi_rvalid,
      s_axi_rready => s_axi_rready,
      phy_tx_clk => phy_tx_clk,
      phy_rx_clk => phy_rx_clk,
      phy_crs => phy_crs,
      phy_dv => phy_dv,
      phy_rx_data => phy_rx_data,
      phy_col => phy_col,
      phy_rx_er => phy_rx_er,
      phy_rst_n => phy_rst_n,
      phy_tx_en => phy_tx_en,
      phy_tx_data => phy_tx_data,
      phy_mdio_i => '0'
    );
END design_1_axi_ethernetlite_0_0_arch;

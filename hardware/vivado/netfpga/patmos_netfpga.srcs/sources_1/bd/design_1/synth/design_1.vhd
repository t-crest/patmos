--Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
--Date        : Mon Nov  2 17:26:41 2020
--Host        : DESKTOP-SS2DKB0 running 64-bit major release  (build 9200)
--Command     : generate_target design_1.bd
--Design      : design_1
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity s00_couplers_imp_1JW51VR is
  port (
    M_ACLK : in STD_LOGIC;
    M_ARESETN : in STD_LOGIC;
    M_AXI_araddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    M_AXI_arready : in STD_LOGIC;
    M_AXI_arvalid : out STD_LOGIC;
    M_AXI_awaddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    M_AXI_awready : in STD_LOGIC;
    M_AXI_awvalid : out STD_LOGIC;
    M_AXI_bready : out STD_LOGIC;
    M_AXI_bresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    M_AXI_bvalid : in STD_LOGIC;
    M_AXI_rdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    M_AXI_rready : out STD_LOGIC;
    M_AXI_rresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    M_AXI_rvalid : in STD_LOGIC;
    M_AXI_wdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    M_AXI_wready : in STD_LOGIC;
    M_AXI_wstrb : out STD_LOGIC_VECTOR ( 3 downto 0 );
    M_AXI_wvalid : out STD_LOGIC;
    S_ACLK : in STD_LOGIC;
    S_ARESETN : in STD_LOGIC;
    S_AXI_araddr : in STD_LOGIC_VECTOR ( 11 downto 0 );
    S_AXI_arready : out STD_LOGIC;
    S_AXI_arvalid : in STD_LOGIC;
    S_AXI_awaddr : in STD_LOGIC_VECTOR ( 11 downto 0 );
    S_AXI_awready : out STD_LOGIC;
    S_AXI_awvalid : in STD_LOGIC;
    S_AXI_bready : in STD_LOGIC;
    S_AXI_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_bvalid : out STD_LOGIC;
    S_AXI_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_rready : in STD_LOGIC;
    S_AXI_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S_AXI_rvalid : out STD_LOGIC;
    S_AXI_wdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S_AXI_wready : out STD_LOGIC;
    S_AXI_wstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S_AXI_wvalid : in STD_LOGIC
  );
end s00_couplers_imp_1JW51VR;

architecture STRUCTURE of s00_couplers_imp_1JW51VR is
  signal s00_couplers_to_s00_couplers_ARADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal s00_couplers_to_s00_couplers_ARREADY : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_ARVALID : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_AWADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal s00_couplers_to_s00_couplers_AWREADY : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_AWVALID : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_BREADY : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_BRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal s00_couplers_to_s00_couplers_BVALID : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_RDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal s00_couplers_to_s00_couplers_RREADY : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_RRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal s00_couplers_to_s00_couplers_RVALID : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_WDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal s00_couplers_to_s00_couplers_WREADY : STD_LOGIC;
  signal s00_couplers_to_s00_couplers_WSTRB : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal s00_couplers_to_s00_couplers_WVALID : STD_LOGIC;
begin
  M_AXI_araddr(11 downto 0) <= s00_couplers_to_s00_couplers_ARADDR(11 downto 0);
  M_AXI_arvalid <= s00_couplers_to_s00_couplers_ARVALID;
  M_AXI_awaddr(11 downto 0) <= s00_couplers_to_s00_couplers_AWADDR(11 downto 0);
  M_AXI_awvalid <= s00_couplers_to_s00_couplers_AWVALID;
  M_AXI_bready <= s00_couplers_to_s00_couplers_BREADY;
  M_AXI_rready <= s00_couplers_to_s00_couplers_RREADY;
  M_AXI_wdata(31 downto 0) <= s00_couplers_to_s00_couplers_WDATA(31 downto 0);
  M_AXI_wstrb(3 downto 0) <= s00_couplers_to_s00_couplers_WSTRB(3 downto 0);
  M_AXI_wvalid <= s00_couplers_to_s00_couplers_WVALID;
  S_AXI_arready <= s00_couplers_to_s00_couplers_ARREADY;
  S_AXI_awready <= s00_couplers_to_s00_couplers_AWREADY;
  S_AXI_bresp(1 downto 0) <= s00_couplers_to_s00_couplers_BRESP(1 downto 0);
  S_AXI_bvalid <= s00_couplers_to_s00_couplers_BVALID;
  S_AXI_rdata(31 downto 0) <= s00_couplers_to_s00_couplers_RDATA(31 downto 0);
  S_AXI_rresp(1 downto 0) <= s00_couplers_to_s00_couplers_RRESP(1 downto 0);
  S_AXI_rvalid <= s00_couplers_to_s00_couplers_RVALID;
  S_AXI_wready <= s00_couplers_to_s00_couplers_WREADY;
  s00_couplers_to_s00_couplers_ARADDR(11 downto 0) <= S_AXI_araddr(11 downto 0);
  s00_couplers_to_s00_couplers_ARREADY <= M_AXI_arready;
  s00_couplers_to_s00_couplers_ARVALID <= S_AXI_arvalid;
  s00_couplers_to_s00_couplers_AWADDR(11 downto 0) <= S_AXI_awaddr(11 downto 0);
  s00_couplers_to_s00_couplers_AWREADY <= M_AXI_awready;
  s00_couplers_to_s00_couplers_AWVALID <= S_AXI_awvalid;
  s00_couplers_to_s00_couplers_BREADY <= S_AXI_bready;
  s00_couplers_to_s00_couplers_BRESP(1 downto 0) <= M_AXI_bresp(1 downto 0);
  s00_couplers_to_s00_couplers_BVALID <= M_AXI_bvalid;
  s00_couplers_to_s00_couplers_RDATA(31 downto 0) <= M_AXI_rdata(31 downto 0);
  s00_couplers_to_s00_couplers_RREADY <= S_AXI_rready;
  s00_couplers_to_s00_couplers_RRESP(1 downto 0) <= M_AXI_rresp(1 downto 0);
  s00_couplers_to_s00_couplers_RVALID <= M_AXI_rvalid;
  s00_couplers_to_s00_couplers_WDATA(31 downto 0) <= S_AXI_wdata(31 downto 0);
  s00_couplers_to_s00_couplers_WREADY <= M_AXI_wready;
  s00_couplers_to_s00_couplers_WSTRB(3 downto 0) <= S_AXI_wstrb(3 downto 0);
  s00_couplers_to_s00_couplers_WVALID <= S_AXI_wvalid;
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_1_patmos_top_0_axi_periph_0 is
  port (
    ACLK : in STD_LOGIC;
    ARESETN : in STD_LOGIC;
    M00_ACLK : in STD_LOGIC;
    M00_ARESETN : in STD_LOGIC;
    M00_AXI_araddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    M00_AXI_arready : in STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_arvalid : out STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_awaddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    M00_AXI_awready : in STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_awvalid : out STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_bready : out STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_bresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    M00_AXI_bvalid : in STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_rdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    M00_AXI_rready : out STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_rresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    M00_AXI_rvalid : in STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_wdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    M00_AXI_wready : in STD_LOGIC_VECTOR ( 0 to 0 );
    M00_AXI_wstrb : out STD_LOGIC_VECTOR ( 3 downto 0 );
    M00_AXI_wvalid : out STD_LOGIC_VECTOR ( 0 to 0 );
    S00_ACLK : in STD_LOGIC;
    S00_ARESETN : in STD_LOGIC;
    S00_AXI_araddr : in STD_LOGIC_VECTOR ( 11 downto 0 );
    S00_AXI_arready : out STD_LOGIC;
    S00_AXI_arvalid : in STD_LOGIC;
    S00_AXI_awaddr : in STD_LOGIC_VECTOR ( 11 downto 0 );
    S00_AXI_awready : out STD_LOGIC;
    S00_AXI_awvalid : in STD_LOGIC;
    S00_AXI_bready : in STD_LOGIC;
    S00_AXI_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S00_AXI_bvalid : out STD_LOGIC;
    S00_AXI_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    S00_AXI_rready : in STD_LOGIC;
    S00_AXI_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    S00_AXI_rvalid : out STD_LOGIC;
    S00_AXI_wdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    S00_AXI_wready : out STD_LOGIC;
    S00_AXI_wstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    S00_AXI_wvalid : in STD_LOGIC
  );
end design_1_patmos_top_0_axi_periph_0;

architecture STRUCTURE of design_1_patmos_top_0_axi_periph_0 is
  signal S00_ACLK_1 : STD_LOGIC;
  signal S00_ARESETN_1 : STD_LOGIC;
  signal patmos_top_0_axi_periph_ACLK_net : STD_LOGIC;
  signal patmos_top_0_axi_periph_ARESETN_net : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_ARADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_ARREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_ARVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_AWADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_AWREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_AWVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_BREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_BRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_BVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_RDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_RREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_RRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_RVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_WDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_WREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_to_s00_couplers_WSTRB : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal patmos_top_0_axi_periph_to_s00_couplers_WVALID : STD_LOGIC;
  signal s00_couplers_to_patmos_top_0_axi_periph_ARADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_ARREADY : STD_LOGIC_VECTOR ( 0 to 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_ARVALID : STD_LOGIC;
  signal s00_couplers_to_patmos_top_0_axi_periph_AWADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_AWREADY : STD_LOGIC_VECTOR ( 0 to 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_AWVALID : STD_LOGIC;
  signal s00_couplers_to_patmos_top_0_axi_periph_BREADY : STD_LOGIC;
  signal s00_couplers_to_patmos_top_0_axi_periph_BRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_BVALID : STD_LOGIC_VECTOR ( 0 to 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_RDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_RREADY : STD_LOGIC;
  signal s00_couplers_to_patmos_top_0_axi_periph_RRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_RVALID : STD_LOGIC_VECTOR ( 0 to 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_WDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_WREADY : STD_LOGIC_VECTOR ( 0 to 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_WSTRB : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal s00_couplers_to_patmos_top_0_axi_periph_WVALID : STD_LOGIC;
begin
  M00_AXI_araddr(11 downto 0) <= s00_couplers_to_patmos_top_0_axi_periph_ARADDR(11 downto 0);
  M00_AXI_arvalid(0) <= s00_couplers_to_patmos_top_0_axi_periph_ARVALID;
  M00_AXI_awaddr(11 downto 0) <= s00_couplers_to_patmos_top_0_axi_periph_AWADDR(11 downto 0);
  M00_AXI_awvalid(0) <= s00_couplers_to_patmos_top_0_axi_periph_AWVALID;
  M00_AXI_bready(0) <= s00_couplers_to_patmos_top_0_axi_periph_BREADY;
  M00_AXI_rready(0) <= s00_couplers_to_patmos_top_0_axi_periph_RREADY;
  M00_AXI_wdata(31 downto 0) <= s00_couplers_to_patmos_top_0_axi_periph_WDATA(31 downto 0);
  M00_AXI_wstrb(3 downto 0) <= s00_couplers_to_patmos_top_0_axi_periph_WSTRB(3 downto 0);
  M00_AXI_wvalid(0) <= s00_couplers_to_patmos_top_0_axi_periph_WVALID;
  S00_ACLK_1 <= S00_ACLK;
  S00_ARESETN_1 <= S00_ARESETN;
  S00_AXI_arready <= patmos_top_0_axi_periph_to_s00_couplers_ARREADY;
  S00_AXI_awready <= patmos_top_0_axi_periph_to_s00_couplers_AWREADY;
  S00_AXI_bresp(1 downto 0) <= patmos_top_0_axi_periph_to_s00_couplers_BRESP(1 downto 0);
  S00_AXI_bvalid <= patmos_top_0_axi_periph_to_s00_couplers_BVALID;
  S00_AXI_rdata(31 downto 0) <= patmos_top_0_axi_periph_to_s00_couplers_RDATA(31 downto 0);
  S00_AXI_rresp(1 downto 0) <= patmos_top_0_axi_periph_to_s00_couplers_RRESP(1 downto 0);
  S00_AXI_rvalid <= patmos_top_0_axi_periph_to_s00_couplers_RVALID;
  S00_AXI_wready <= patmos_top_0_axi_periph_to_s00_couplers_WREADY;
  patmos_top_0_axi_periph_ACLK_net <= M00_ACLK;
  patmos_top_0_axi_periph_ARESETN_net <= M00_ARESETN;
  patmos_top_0_axi_periph_to_s00_couplers_ARADDR(11 downto 0) <= S00_AXI_araddr(11 downto 0);
  patmos_top_0_axi_periph_to_s00_couplers_ARVALID <= S00_AXI_arvalid;
  patmos_top_0_axi_periph_to_s00_couplers_AWADDR(11 downto 0) <= S00_AXI_awaddr(11 downto 0);
  patmos_top_0_axi_periph_to_s00_couplers_AWVALID <= S00_AXI_awvalid;
  patmos_top_0_axi_periph_to_s00_couplers_BREADY <= S00_AXI_bready;
  patmos_top_0_axi_periph_to_s00_couplers_RREADY <= S00_AXI_rready;
  patmos_top_0_axi_periph_to_s00_couplers_WDATA(31 downto 0) <= S00_AXI_wdata(31 downto 0);
  patmos_top_0_axi_periph_to_s00_couplers_WSTRB(3 downto 0) <= S00_AXI_wstrb(3 downto 0);
  patmos_top_0_axi_periph_to_s00_couplers_WVALID <= S00_AXI_wvalid;
  s00_couplers_to_patmos_top_0_axi_periph_ARREADY(0) <= M00_AXI_arready(0);
  s00_couplers_to_patmos_top_0_axi_periph_AWREADY(0) <= M00_AXI_awready(0);
  s00_couplers_to_patmos_top_0_axi_periph_BRESP(1 downto 0) <= M00_AXI_bresp(1 downto 0);
  s00_couplers_to_patmos_top_0_axi_periph_BVALID(0) <= M00_AXI_bvalid(0);
  s00_couplers_to_patmos_top_0_axi_periph_RDATA(31 downto 0) <= M00_AXI_rdata(31 downto 0);
  s00_couplers_to_patmos_top_0_axi_periph_RRESP(1 downto 0) <= M00_AXI_rresp(1 downto 0);
  s00_couplers_to_patmos_top_0_axi_periph_RVALID(0) <= M00_AXI_rvalid(0);
  s00_couplers_to_patmos_top_0_axi_periph_WREADY(0) <= M00_AXI_wready(0);
s00_couplers: entity work.s00_couplers_imp_1JW51VR
     port map (
      M_ACLK => patmos_top_0_axi_periph_ACLK_net,
      M_ARESETN => patmos_top_0_axi_periph_ARESETN_net,
      M_AXI_araddr(11 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_ARADDR(11 downto 0),
      M_AXI_arready => s00_couplers_to_patmos_top_0_axi_periph_ARREADY(0),
      M_AXI_arvalid => s00_couplers_to_patmos_top_0_axi_periph_ARVALID,
      M_AXI_awaddr(11 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_AWADDR(11 downto 0),
      M_AXI_awready => s00_couplers_to_patmos_top_0_axi_periph_AWREADY(0),
      M_AXI_awvalid => s00_couplers_to_patmos_top_0_axi_periph_AWVALID,
      M_AXI_bready => s00_couplers_to_patmos_top_0_axi_periph_BREADY,
      M_AXI_bresp(1 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_BRESP(1 downto 0),
      M_AXI_bvalid => s00_couplers_to_patmos_top_0_axi_periph_BVALID(0),
      M_AXI_rdata(31 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_RDATA(31 downto 0),
      M_AXI_rready => s00_couplers_to_patmos_top_0_axi_periph_RREADY,
      M_AXI_rresp(1 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_RRESP(1 downto 0),
      M_AXI_rvalid => s00_couplers_to_patmos_top_0_axi_periph_RVALID(0),
      M_AXI_wdata(31 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_WDATA(31 downto 0),
      M_AXI_wready => s00_couplers_to_patmos_top_0_axi_periph_WREADY(0),
      M_AXI_wstrb(3 downto 0) => s00_couplers_to_patmos_top_0_axi_periph_WSTRB(3 downto 0),
      M_AXI_wvalid => s00_couplers_to_patmos_top_0_axi_periph_WVALID,
      S_ACLK => S00_ACLK_1,
      S_ARESETN => S00_ARESETN_1,
      S_AXI_araddr(11 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_ARADDR(11 downto 0),
      S_AXI_arready => patmos_top_0_axi_periph_to_s00_couplers_ARREADY,
      S_AXI_arvalid => patmos_top_0_axi_periph_to_s00_couplers_ARVALID,
      S_AXI_awaddr(11 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_AWADDR(11 downto 0),
      S_AXI_awready => patmos_top_0_axi_periph_to_s00_couplers_AWREADY,
      S_AXI_awvalid => patmos_top_0_axi_periph_to_s00_couplers_AWVALID,
      S_AXI_bready => patmos_top_0_axi_periph_to_s00_couplers_BREADY,
      S_AXI_bresp(1 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_BRESP(1 downto 0),
      S_AXI_bvalid => patmos_top_0_axi_periph_to_s00_couplers_BVALID,
      S_AXI_rdata(31 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_RDATA(31 downto 0),
      S_AXI_rready => patmos_top_0_axi_periph_to_s00_couplers_RREADY,
      S_AXI_rresp(1 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_RRESP(1 downto 0),
      S_AXI_rvalid => patmos_top_0_axi_periph_to_s00_couplers_RVALID,
      S_AXI_wdata(31 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_WDATA(31 downto 0),
      S_AXI_wready => patmos_top_0_axi_periph_to_s00_couplers_WREADY,
      S_AXI_wstrb(3 downto 0) => patmos_top_0_axi_periph_to_s00_couplers_WSTRB(3 downto 0),
      S_AXI_wvalid => patmos_top_0_axi_periph_to_s00_couplers_WVALID
    );
end STRUCTURE;
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_1 is
  port (
    btn : in STD_LOGIC_VECTOR ( 3 downto 0 );
    clk_in_n : in STD_LOGIC;
    clk_in_p : in STD_LOGIC;
    gpio_rtl_0_tri_o_tri_o : out STD_LOGIC_VECTOR ( 3 downto 0 );
    iUartPins_rxd : out STD_LOGIC;
    led : out STD_LOGIC_VECTOR ( 3 downto 0 );
    oUartPins_txd : in STD_LOGIC;
    phy_rstn_1 : out STD_LOGIC;
    rgmii_rx_ctl_1 : in STD_LOGIC;
    rgmii_rxc_1 : in STD_LOGIC;
    rgmii_rxd_1 : in STD_LOGIC_VECTOR ( 3 downto 0 );
    rgmii_tx_ctl_1 : out STD_LOGIC;
    rgmii_txc_1 : out STD_LOGIC;
    rgmii_txd_1 : out STD_LOGIC_VECTOR ( 3 downto 0 )
  );
  attribute CORE_GENERATION_INFO : string;
  attribute CORE_GENERATION_INFO of design_1 : entity is "design_1,IP_Integrator,{x_ipVendor=xilinx.com,x_ipLibrary=BlockDiagram,x_ipName=design_1,x_ipVersion=1.00.a,x_ipLanguage=VHDL,numBlks=9,numReposBlks=7,numNonXlnxBlks=0,numHierBlks=2,maxHierDepth=0,numSysgenBlks=0,numHlsBlks=0,numHdlrefBlks=2,numPkgbdBlks=0,bdsource=USER,da_axi4_cnt=2,da_board_cnt=4,da_clkrst_cnt=2,synth_mode=OOC_per_IP}";
  attribute HW_HANDOFF : string;
  attribute HW_HANDOFF of design_1 : entity is "design_1.hwdef";
end design_1;

architecture STRUCTURE of design_1 is
  component design_1_clk_wiz_0_0 is
  port (
    clk_in1_p : in STD_LOGIC;
    clk_in1_n : in STD_LOGIC;
    clk_out1 : out STD_LOGIC;
    locked : out STD_LOGIC
  );
  end component design_1_clk_wiz_0_0;
  component design_1_axi_ethernetlite_0_0 is
  port (
    s_axi_aclk : in STD_LOGIC;
    s_axi_aresetn : in STD_LOGIC;
    ip2intc_irpt : out STD_LOGIC;
    s_axi_awaddr : in STD_LOGIC_VECTOR ( 12 downto 0 );
    s_axi_awvalid : in STD_LOGIC;
    s_axi_awready : out STD_LOGIC;
    s_axi_wdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_wstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_wvalid : in STD_LOGIC;
    s_axi_wready : out STD_LOGIC;
    s_axi_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_bvalid : out STD_LOGIC;
    s_axi_bready : in STD_LOGIC;
    s_axi_araddr : in STD_LOGIC_VECTOR ( 12 downto 0 );
    s_axi_arvalid : in STD_LOGIC;
    s_axi_arready : out STD_LOGIC;
    s_axi_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_rvalid : out STD_LOGIC;
    s_axi_rready : in STD_LOGIC;
    phy_tx_clk : in STD_LOGIC;
    phy_rx_clk : in STD_LOGIC;
    phy_crs : in STD_LOGIC;
    phy_dv : in STD_LOGIC;
    phy_rx_data : in STD_LOGIC_VECTOR ( 3 downto 0 );
    phy_col : in STD_LOGIC;
    phy_rx_er : in STD_LOGIC;
    phy_rst_n : out STD_LOGIC;
    phy_tx_en : out STD_LOGIC;
    phy_tx_data : out STD_LOGIC_VECTOR ( 3 downto 0 )
  );
  end component design_1_axi_ethernetlite_0_0;
  component design_1_rst_clk_wiz_0_100M_0 is
  port (
    slowest_sync_clk : in STD_LOGIC;
    ext_reset_in : in STD_LOGIC;
    aux_reset_in : in STD_LOGIC;
    mb_debug_sys_rst : in STD_LOGIC;
    dcm_locked : in STD_LOGIC;
    mb_reset : out STD_LOGIC;
    bus_struct_reset : out STD_LOGIC_VECTOR ( 0 to 0 );
    peripheral_reset : out STD_LOGIC_VECTOR ( 0 to 0 );
    interconnect_aresetn : out STD_LOGIC_VECTOR ( 0 to 0 );
    peripheral_aresetn : out STD_LOGIC_VECTOR ( 0 to 0 )
  );
  end component design_1_rst_clk_wiz_0_100M_0;
  component design_1_clk_wiz_0_1 is
  port (
    clk_in1 : in STD_LOGIC;
    clk_25 : out STD_LOGIC;
    locked : out STD_LOGIC
  );
  end component design_1_clk_wiz_0_1;
  component design_1_mii2rgmii_0_0 is
  port (
    rgmii_clk : in STD_LOGIC;
    mii_phy_tx_data : in STD_LOGIC_VECTOR ( 3 downto 0 );
    mii_phy_tx_en : in STD_LOGIC;
    mii_phy_tx_er : in STD_LOGIC;
    mii_phy_rx_data : out STD_LOGIC_VECTOR ( 3 downto 0 );
    mii_phy_dv : out STD_LOGIC;
    mii_phy_rx_er : out STD_LOGIC;
    mii_phy_crs : out STD_LOGIC;
    mii_phy_col : out STD_LOGIC;
    rgmii_phy_txc : out STD_LOGIC;
    rgmii_phy_txd : out STD_LOGIC_VECTOR ( 3 downto 0 );
    rgmii_phy_tx_ctl : out STD_LOGIC;
    rgmii_phy_rxc : in STD_LOGIC;
    rgmii_phy_rxd : in STD_LOGIC_VECTOR ( 3 downto 0 );
    rgmii_phy_rx_ctl : in STD_LOGIC
  );
  end component design_1_mii2rgmii_0_0;
  component design_1_axi_gpio_0_0 is
  port (
    s_axi_aclk : in STD_LOGIC;
    s_axi_aresetn : in STD_LOGIC;
    s_axi_awaddr : in STD_LOGIC_VECTOR ( 8 downto 0 );
    s_axi_awvalid : in STD_LOGIC;
    s_axi_awready : out STD_LOGIC;
    s_axi_wdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_wstrb : in STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_wvalid : in STD_LOGIC;
    s_axi_wready : out STD_LOGIC;
    s_axi_bresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_bvalid : out STD_LOGIC;
    s_axi_bready : in STD_LOGIC;
    s_axi_araddr : in STD_LOGIC_VECTOR ( 8 downto 0 );
    s_axi_arvalid : in STD_LOGIC;
    s_axi_arready : out STD_LOGIC;
    s_axi_rdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_rresp : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_rvalid : out STD_LOGIC;
    s_axi_rready : in STD_LOGIC;
    gpio_io_o : out STD_LOGIC_VECTOR ( 3 downto 0 )
  );
  end component design_1_axi_gpio_0_0;
  component design_1_patmos_top_0_0 is
  port (
    clk_int : in STD_LOGIC;
    locked : in STD_LOGIC;
    led : out STD_LOGIC_VECTOR ( 3 downto 0 );
    btn : in STD_LOGIC_VECTOR ( 3 downto 0 );
    oUartPins_txd : out STD_LOGIC;
    iUartPins_rxd : in STD_LOGIC;
    m_axi_awaddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    m_axi_bresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    m_axi_rresp : in STD_LOGIC_VECTOR ( 1 downto 0 );
    m_axi_wstrb : out STD_LOGIC_VECTOR ( 3 downto 0 );
    m_axi_wready : in STD_LOGIC;
    m_axi_rready : out STD_LOGIC;
    m_axi_bready : out STD_LOGIC;
    m_axi_arvalid : out STD_LOGIC;
    m_axi_araddr : out STD_LOGIC_VECTOR ( 11 downto 0 );
    m_axi_awready : in STD_LOGIC;
    m_axi_bvalid : in STD_LOGIC;
    m_axi_rvalid : in STD_LOGIC;
    m_axi_wvalid : out STD_LOGIC;
    m_axi_wdata : out STD_LOGIC_VECTOR ( 31 downto 0 );
    m_axi_rdata : in STD_LOGIC_VECTOR ( 31 downto 0 );
    m_axi_awvalid : out STD_LOGIC;
    m_axi_arready : in STD_LOGIC
  );
  end component design_1_patmos_top_0_0;
  signal axi_ethernetlite_0_phy_rst_n : STD_LOGIC;
  signal axi_ethernetlite_0_phy_tx_data : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal axi_ethernetlite_0_phy_tx_en : STD_LOGIC;
  signal axi_gpio_0_GPIO_TRI_O : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal btn_1 : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal clk_in_n_1 : STD_LOGIC;
  signal clk_in_p_1 : STD_LOGIC;
  signal clk_wiz_0_clk_out1 : STD_LOGIC;
  signal clk_wiz_0_locked : STD_LOGIC;
  signal clk_wiz_1_clk_25 : STD_LOGIC;
  signal mii2rgmii_0_RGMII_TXD : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal mii2rgmii_0_TXC : STD_LOGIC;
  signal mii2rgmii_0_TX_CTL : STD_LOGIC;
  signal mii2rgmii_0_phy_col : STD_LOGIC;
  signal mii2rgmii_0_phy_crs : STD_LOGIC;
  signal mii2rgmii_0_phy_dv : STD_LOGIC;
  signal mii2rgmii_0_phy_rx_data : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal mii2rgmii_0_phy_rx_er : STD_LOGIC;
  signal oUartPins_txd_1 : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_ARADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_ARREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_ARVALID : STD_LOGIC_VECTOR ( 0 to 0 );
  signal patmos_top_0_axi_periph_M00_AXI_AWADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_AWREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_AWVALID : STD_LOGIC_VECTOR ( 0 to 0 );
  signal patmos_top_0_axi_periph_M00_AXI_BREADY : STD_LOGIC_VECTOR ( 0 to 0 );
  signal patmos_top_0_axi_periph_M00_AXI_BRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_BVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_RDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_RREADY : STD_LOGIC_VECTOR ( 0 to 0 );
  signal patmos_top_0_axi_periph_M00_AXI_RRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_RVALID : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_WDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_WREADY : STD_LOGIC;
  signal patmos_top_0_axi_periph_M00_AXI_WSTRB : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal patmos_top_0_axi_periph_M00_AXI_WVALID : STD_LOGIC_VECTOR ( 0 to 0 );
  signal patmos_top_0_led : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal patmos_top_0_m_axi_ARADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_m_axi_ARREADY : STD_LOGIC;
  signal patmos_top_0_m_axi_ARVALID : STD_LOGIC;
  signal patmos_top_0_m_axi_AWADDR : STD_LOGIC_VECTOR ( 11 downto 0 );
  signal patmos_top_0_m_axi_AWREADY : STD_LOGIC;
  signal patmos_top_0_m_axi_AWVALID : STD_LOGIC;
  signal patmos_top_0_m_axi_BREADY : STD_LOGIC;
  signal patmos_top_0_m_axi_BRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_m_axi_BVALID : STD_LOGIC;
  signal patmos_top_0_m_axi_RDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_m_axi_RREADY : STD_LOGIC;
  signal patmos_top_0_m_axi_RRESP : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal patmos_top_0_m_axi_RVALID : STD_LOGIC;
  signal patmos_top_0_m_axi_WDATA : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal patmos_top_0_m_axi_WREADY : STD_LOGIC;
  signal patmos_top_0_m_axi_WSTRB : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal patmos_top_0_m_axi_WVALID : STD_LOGIC;
  signal patmos_top_0_oUartPins_txd : STD_LOGIC;
  signal rgmii_rx_ctl_1_1 : STD_LOGIC;
  signal rgmii_rxc_1_1 : STD_LOGIC;
  signal rgmii_rxd_1_1 : STD_LOGIC_VECTOR ( 3 downto 0 );
  signal rst_clk_wiz_0_100M_peripheral_aresetn : STD_LOGIC_VECTOR ( 0 to 0 );
  signal NLW_axi_ethernetlite_0_ip2intc_irpt_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_arready_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_awready_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_bvalid_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_rvalid_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_wready_UNCONNECTED : STD_LOGIC;
  signal NLW_axi_ethernetlite_0_s_axi_bresp_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal NLW_axi_ethernetlite_0_s_axi_rdata_UNCONNECTED : STD_LOGIC_VECTOR ( 31 downto 0 );
  signal NLW_axi_ethernetlite_0_s_axi_rresp_UNCONNECTED : STD_LOGIC_VECTOR ( 1 downto 0 );
  signal NLW_clk_wiz_1_locked_UNCONNECTED : STD_LOGIC;
  signal NLW_rst_clk_wiz_0_100M_mb_reset_UNCONNECTED : STD_LOGIC;
  signal NLW_rst_clk_wiz_0_100M_bus_struct_reset_UNCONNECTED : STD_LOGIC_VECTOR ( 0 to 0 );
  signal NLW_rst_clk_wiz_0_100M_interconnect_aresetn_UNCONNECTED : STD_LOGIC_VECTOR ( 0 to 0 );
  signal NLW_rst_clk_wiz_0_100M_peripheral_reset_UNCONNECTED : STD_LOGIC_VECTOR ( 0 to 0 );
  attribute X_INTERFACE_INFO : string;
  attribute X_INTERFACE_INFO of clk_in_n : signal is "xilinx.com:signal:clock:1.0 CLK.CLK_IN_N CLK";
  attribute X_INTERFACE_PARAMETER : string;
  attribute X_INTERFACE_PARAMETER of clk_in_n : signal is "XIL_INTERFACENAME CLK.CLK_IN_N, CLK_DOMAIN design_1_clk_in_n, FREQ_HZ 200000000, INSERT_VIP 0, PHASE 0.000";
  attribute X_INTERFACE_INFO of clk_in_p : signal is "xilinx.com:signal:clock:1.0 CLK.CLK_IN_P CLK";
  attribute X_INTERFACE_PARAMETER of clk_in_p : signal is "XIL_INTERFACENAME CLK.CLK_IN_P, CLK_DOMAIN design_1_clk_in_p, FREQ_HZ 200000000, INSERT_VIP 0, PHASE 0.000";
  attribute X_INTERFACE_INFO of phy_rstn_1 : signal is "xilinx.com:signal:reset:1.0 RST.PHY_RSTN_1 RST";
  attribute X_INTERFACE_PARAMETER of phy_rstn_1 : signal is "XIL_INTERFACENAME RST.PHY_RSTN_1, INSERT_VIP 0, POLARITY ACTIVE_LOW";
  attribute X_INTERFACE_INFO of gpio_rtl_0_tri_o_tri_o : signal is "xilinx.com:interface:gpio:1.0 gpio_rtl_0_tri_o ";
begin
  btn_1(3 downto 0) <= btn(3 downto 0);
  clk_in_n_1 <= clk_in_n;
  clk_in_p_1 <= clk_in_p;
  gpio_rtl_0_tri_o_tri_o(3 downto 0) <= axi_gpio_0_GPIO_TRI_O(3 downto 0);
  iUartPins_rxd <= patmos_top_0_oUartPins_txd;
  led(3 downto 0) <= patmos_top_0_led(3 downto 0);
  oUartPins_txd_1 <= oUartPins_txd;
  phy_rstn_1 <= axi_ethernetlite_0_phy_rst_n;
  rgmii_rx_ctl_1_1 <= rgmii_rx_ctl_1;
  rgmii_rxc_1_1 <= rgmii_rxc_1;
  rgmii_rxd_1_1(3 downto 0) <= rgmii_rxd_1(3 downto 0);
  rgmii_tx_ctl_1 <= mii2rgmii_0_TX_CTL;
  rgmii_txc_1 <= mii2rgmii_0_TXC;
  rgmii_txd_1(3 downto 0) <= mii2rgmii_0_RGMII_TXD(3 downto 0);
axi_ethernetlite_0: component design_1_axi_ethernetlite_0_0
     port map (
      ip2intc_irpt => NLW_axi_ethernetlite_0_ip2intc_irpt_UNCONNECTED,
      phy_col => mii2rgmii_0_phy_col,
      phy_crs => mii2rgmii_0_phy_crs,
      phy_dv => mii2rgmii_0_phy_dv,
      phy_rst_n => axi_ethernetlite_0_phy_rst_n,
      phy_rx_clk => clk_wiz_1_clk_25,
      phy_rx_data(3 downto 0) => mii2rgmii_0_phy_rx_data(3 downto 0),
      phy_rx_er => mii2rgmii_0_phy_rx_er,
      phy_tx_clk => clk_wiz_1_clk_25,
      phy_tx_data(3 downto 0) => axi_ethernetlite_0_phy_tx_data(3 downto 0),
      phy_tx_en => axi_ethernetlite_0_phy_tx_en,
      s_axi_aclk => clk_wiz_0_clk_out1,
      s_axi_araddr(12 downto 0) => B"0000000000000",
      s_axi_aresetn => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      s_axi_arready => NLW_axi_ethernetlite_0_s_axi_arready_UNCONNECTED,
      s_axi_arvalid => '0',
      s_axi_awaddr(12 downto 0) => B"0000000000000",
      s_axi_awready => NLW_axi_ethernetlite_0_s_axi_awready_UNCONNECTED,
      s_axi_awvalid => '0',
      s_axi_bready => '0',
      s_axi_bresp(1 downto 0) => NLW_axi_ethernetlite_0_s_axi_bresp_UNCONNECTED(1 downto 0),
      s_axi_bvalid => NLW_axi_ethernetlite_0_s_axi_bvalid_UNCONNECTED,
      s_axi_rdata(31 downto 0) => NLW_axi_ethernetlite_0_s_axi_rdata_UNCONNECTED(31 downto 0),
      s_axi_rready => '0',
      s_axi_rresp(1 downto 0) => NLW_axi_ethernetlite_0_s_axi_rresp_UNCONNECTED(1 downto 0),
      s_axi_rvalid => NLW_axi_ethernetlite_0_s_axi_rvalid_UNCONNECTED,
      s_axi_wdata(31 downto 0) => B"00000000000000000000000000000000",
      s_axi_wready => NLW_axi_ethernetlite_0_s_axi_wready_UNCONNECTED,
      s_axi_wstrb(3 downto 0) => B"1111",
      s_axi_wvalid => '0'
    );
axi_gpio_0: component design_1_axi_gpio_0_0
     port map (
      gpio_io_o(3 downto 0) => axi_gpio_0_GPIO_TRI_O(3 downto 0),
      s_axi_aclk => clk_wiz_0_clk_out1,
      s_axi_araddr(8 downto 0) => patmos_top_0_axi_periph_M00_AXI_ARADDR(8 downto 0),
      s_axi_aresetn => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      s_axi_arready => patmos_top_0_axi_periph_M00_AXI_ARREADY,
      s_axi_arvalid => patmos_top_0_axi_periph_M00_AXI_ARVALID(0),
      s_axi_awaddr(8 downto 0) => patmos_top_0_axi_periph_M00_AXI_AWADDR(8 downto 0),
      s_axi_awready => patmos_top_0_axi_periph_M00_AXI_AWREADY,
      s_axi_awvalid => patmos_top_0_axi_periph_M00_AXI_AWVALID(0),
      s_axi_bready => patmos_top_0_axi_periph_M00_AXI_BREADY(0),
      s_axi_bresp(1 downto 0) => patmos_top_0_axi_periph_M00_AXI_BRESP(1 downto 0),
      s_axi_bvalid => patmos_top_0_axi_periph_M00_AXI_BVALID,
      s_axi_rdata(31 downto 0) => patmos_top_0_axi_periph_M00_AXI_RDATA(31 downto 0),
      s_axi_rready => patmos_top_0_axi_periph_M00_AXI_RREADY(0),
      s_axi_rresp(1 downto 0) => patmos_top_0_axi_periph_M00_AXI_RRESP(1 downto 0),
      s_axi_rvalid => patmos_top_0_axi_periph_M00_AXI_RVALID,
      s_axi_wdata(31 downto 0) => patmos_top_0_axi_periph_M00_AXI_WDATA(31 downto 0),
      s_axi_wready => patmos_top_0_axi_periph_M00_AXI_WREADY,
      s_axi_wstrb(3 downto 0) => patmos_top_0_axi_periph_M00_AXI_WSTRB(3 downto 0),
      s_axi_wvalid => patmos_top_0_axi_periph_M00_AXI_WVALID(0)
    );
clk_wiz_0: component design_1_clk_wiz_0_0
     port map (
      clk_in1_n => clk_in_n_1,
      clk_in1_p => clk_in_p_1,
      clk_out1 => clk_wiz_0_clk_out1,
      locked => clk_wiz_0_locked
    );
clk_wiz_1: component design_1_clk_wiz_0_1
     port map (
      clk_25 => clk_wiz_1_clk_25,
      clk_in1 => clk_wiz_0_clk_out1,
      locked => NLW_clk_wiz_1_locked_UNCONNECTED
    );
mii2rgmii_0: component design_1_mii2rgmii_0_0
     port map (
      mii_phy_col => mii2rgmii_0_phy_col,
      mii_phy_crs => mii2rgmii_0_phy_crs,
      mii_phy_dv => mii2rgmii_0_phy_dv,
      mii_phy_rx_data(3 downto 0) => mii2rgmii_0_phy_rx_data(3 downto 0),
      mii_phy_rx_er => mii2rgmii_0_phy_rx_er,
      mii_phy_tx_data(3 downto 0) => axi_ethernetlite_0_phy_tx_data(3 downto 0),
      mii_phy_tx_en => axi_ethernetlite_0_phy_tx_en,
      mii_phy_tx_er => '0',
      rgmii_clk => clk_wiz_1_clk_25,
      rgmii_phy_rx_ctl => rgmii_rx_ctl_1_1,
      rgmii_phy_rxc => rgmii_rxc_1_1,
      rgmii_phy_rxd(3 downto 0) => rgmii_rxd_1_1(3 downto 0),
      rgmii_phy_tx_ctl => mii2rgmii_0_TX_CTL,
      rgmii_phy_txc => mii2rgmii_0_TXC,
      rgmii_phy_txd(3 downto 0) => mii2rgmii_0_RGMII_TXD(3 downto 0)
    );
patmos_top_0: component design_1_patmos_top_0_0
     port map (
      btn(3 downto 0) => btn_1(3 downto 0),
      clk_int => clk_wiz_0_clk_out1,
      iUartPins_rxd => oUartPins_txd_1,
      led(3 downto 0) => patmos_top_0_led(3 downto 0),
      locked => clk_wiz_0_locked,
      m_axi_araddr(11 downto 0) => patmos_top_0_m_axi_ARADDR(11 downto 0),
      m_axi_arready => patmos_top_0_m_axi_ARREADY,
      m_axi_arvalid => patmos_top_0_m_axi_ARVALID,
      m_axi_awaddr(11 downto 0) => patmos_top_0_m_axi_AWADDR(11 downto 0),
      m_axi_awready => patmos_top_0_m_axi_AWREADY,
      m_axi_awvalid => patmos_top_0_m_axi_AWVALID,
      m_axi_bready => patmos_top_0_m_axi_BREADY,
      m_axi_bresp(1 downto 0) => patmos_top_0_m_axi_BRESP(1 downto 0),
      m_axi_bvalid => patmos_top_0_m_axi_BVALID,
      m_axi_rdata(31 downto 0) => patmos_top_0_m_axi_RDATA(31 downto 0),
      m_axi_rready => patmos_top_0_m_axi_RREADY,
      m_axi_rresp(1 downto 0) => patmos_top_0_m_axi_RRESP(1 downto 0),
      m_axi_rvalid => patmos_top_0_m_axi_RVALID,
      m_axi_wdata(31 downto 0) => patmos_top_0_m_axi_WDATA(31 downto 0),
      m_axi_wready => patmos_top_0_m_axi_WREADY,
      m_axi_wstrb(3 downto 0) => patmos_top_0_m_axi_WSTRB(3 downto 0),
      m_axi_wvalid => patmos_top_0_m_axi_WVALID,
      oUartPins_txd => patmos_top_0_oUartPins_txd
    );
patmos_top_0_axi_periph: entity work.design_1_patmos_top_0_axi_periph_0
     port map (
      ACLK => clk_wiz_0_clk_out1,
      ARESETN => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      M00_ACLK => clk_wiz_0_clk_out1,
      M00_ARESETN => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      M00_AXI_araddr(11 downto 0) => patmos_top_0_axi_periph_M00_AXI_ARADDR(11 downto 0),
      M00_AXI_arready(0) => patmos_top_0_axi_periph_M00_AXI_ARREADY,
      M00_AXI_arvalid(0) => patmos_top_0_axi_periph_M00_AXI_ARVALID(0),
      M00_AXI_awaddr(11 downto 0) => patmos_top_0_axi_periph_M00_AXI_AWADDR(11 downto 0),
      M00_AXI_awready(0) => patmos_top_0_axi_periph_M00_AXI_AWREADY,
      M00_AXI_awvalid(0) => patmos_top_0_axi_periph_M00_AXI_AWVALID(0),
      M00_AXI_bready(0) => patmos_top_0_axi_periph_M00_AXI_BREADY(0),
      M00_AXI_bresp(1 downto 0) => patmos_top_0_axi_periph_M00_AXI_BRESP(1 downto 0),
      M00_AXI_bvalid(0) => patmos_top_0_axi_periph_M00_AXI_BVALID,
      M00_AXI_rdata(31 downto 0) => patmos_top_0_axi_periph_M00_AXI_RDATA(31 downto 0),
      M00_AXI_rready(0) => patmos_top_0_axi_periph_M00_AXI_RREADY(0),
      M00_AXI_rresp(1 downto 0) => patmos_top_0_axi_periph_M00_AXI_RRESP(1 downto 0),
      M00_AXI_rvalid(0) => patmos_top_0_axi_periph_M00_AXI_RVALID,
      M00_AXI_wdata(31 downto 0) => patmos_top_0_axi_periph_M00_AXI_WDATA(31 downto 0),
      M00_AXI_wready(0) => patmos_top_0_axi_periph_M00_AXI_WREADY,
      M00_AXI_wstrb(3 downto 0) => patmos_top_0_axi_periph_M00_AXI_WSTRB(3 downto 0),
      M00_AXI_wvalid(0) => patmos_top_0_axi_periph_M00_AXI_WVALID(0),
      S00_ACLK => clk_wiz_0_clk_out1,
      S00_ARESETN => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      S00_AXI_araddr(11 downto 0) => patmos_top_0_m_axi_ARADDR(11 downto 0),
      S00_AXI_arready => patmos_top_0_m_axi_ARREADY,
      S00_AXI_arvalid => patmos_top_0_m_axi_ARVALID,
      S00_AXI_awaddr(11 downto 0) => patmos_top_0_m_axi_AWADDR(11 downto 0),
      S00_AXI_awready => patmos_top_0_m_axi_AWREADY,
      S00_AXI_awvalid => patmos_top_0_m_axi_AWVALID,
      S00_AXI_bready => patmos_top_0_m_axi_BREADY,
      S00_AXI_bresp(1 downto 0) => patmos_top_0_m_axi_BRESP(1 downto 0),
      S00_AXI_bvalid => patmos_top_0_m_axi_BVALID,
      S00_AXI_rdata(31 downto 0) => patmos_top_0_m_axi_RDATA(31 downto 0),
      S00_AXI_rready => patmos_top_0_m_axi_RREADY,
      S00_AXI_rresp(1 downto 0) => patmos_top_0_m_axi_RRESP(1 downto 0),
      S00_AXI_rvalid => patmos_top_0_m_axi_RVALID,
      S00_AXI_wdata(31 downto 0) => patmos_top_0_m_axi_WDATA(31 downto 0),
      S00_AXI_wready => patmos_top_0_m_axi_WREADY,
      S00_AXI_wstrb(3 downto 0) => patmos_top_0_m_axi_WSTRB(3 downto 0),
      S00_AXI_wvalid => patmos_top_0_m_axi_WVALID
    );
rst_clk_wiz_0_100M: component design_1_rst_clk_wiz_0_100M_0
     port map (
      aux_reset_in => '1',
      bus_struct_reset(0) => NLW_rst_clk_wiz_0_100M_bus_struct_reset_UNCONNECTED(0),
      dcm_locked => clk_wiz_0_locked,
      ext_reset_in => '0',
      interconnect_aresetn(0) => NLW_rst_clk_wiz_0_100M_interconnect_aresetn_UNCONNECTED(0),
      mb_debug_sys_rst => '0',
      mb_reset => NLW_rst_clk_wiz_0_100M_mb_reset_UNCONNECTED,
      peripheral_aresetn(0) => rst_clk_wiz_0_100M_peripheral_aresetn(0),
      peripheral_reset(0) => NLW_rst_clk_wiz_0_100M_peripheral_reset_UNCONNECTED(0),
      slowest_sync_clk => clk_wiz_0_clk_out1
    );
end STRUCTURE;

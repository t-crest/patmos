--Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
--Date        : Mon Nov  2 17:26:41 2020
--Host        : DESKTOP-SS2DKB0 running 64-bit major release  (build 9200)
--Command     : generate_target design_1_wrapper.bd
--Design      : design_1_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_1_wrapper is
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
end design_1_wrapper;

architecture STRUCTURE of design_1_wrapper is
  component design_1 is
  port (
    clk_in_p : in STD_LOGIC;
    clk_in_n : in STD_LOGIC;
    btn : in STD_LOGIC_VECTOR ( 3 downto 0 );
    led : out STD_LOGIC_VECTOR ( 3 downto 0 );
    oUartPins_txd : in STD_LOGIC;
    iUartPins_rxd : out STD_LOGIC;
    rgmii_txd_1 : out STD_LOGIC_VECTOR ( 3 downto 0 );
    rgmii_rxd_1 : in STD_LOGIC_VECTOR ( 3 downto 0 );
    rgmii_tx_ctl_1 : out STD_LOGIC;
    rgmii_txc_1 : out STD_LOGIC;
    rgmii_rxc_1 : in STD_LOGIC;
    rgmii_rx_ctl_1 : in STD_LOGIC;
    phy_rstn_1 : out STD_LOGIC;
    gpio_rtl_0_tri_o_tri_o : out STD_LOGIC_VECTOR ( 3 downto 0 )
  );
  end component design_1;
begin
design_1_i: component design_1
     port map (
      btn(3 downto 0) => btn(3 downto 0),
      clk_in_n => clk_in_n,
      clk_in_p => clk_in_p,
      gpio_rtl_0_tri_o_tri_o(3 downto 0) => gpio_rtl_0_tri_o_tri_o(3 downto 0),
      iUartPins_rxd => iUartPins_rxd,
      led(3 downto 0) => led(3 downto 0),
      oUartPins_txd => oUartPins_txd,
      phy_rstn_1 => phy_rstn_1,
      rgmii_rx_ctl_1 => rgmii_rx_ctl_1,
      rgmii_rxc_1 => rgmii_rxc_1,
      rgmii_rxd_1(3 downto 0) => rgmii_rxd_1(3 downto 0),
      rgmii_tx_ctl_1 => rgmii_tx_ctl_1,
      rgmii_txc_1 => rgmii_txc_1,
      rgmii_txd_1(3 downto 0) => rgmii_txd_1(3 downto 0)
    );
end STRUCTURE;

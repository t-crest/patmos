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

-- IP VLNV: xilinx.com:module_ref:mii2rgmii:1.0
-- IP Revision: 1

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY design_1_mii2rgmii_0_0 IS
  PORT (
    rgmii_clk : IN STD_LOGIC;
    mii_phy_tx_data : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
    mii_phy_tx_en : IN STD_LOGIC;
    mii_phy_tx_er : IN STD_LOGIC;
    mii_phy_rx_data : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    mii_phy_dv : OUT STD_LOGIC;
    mii_phy_rx_er : OUT STD_LOGIC;
    mii_phy_crs : OUT STD_LOGIC;
    mii_phy_col : OUT STD_LOGIC;
    rgmii_phy_txc : OUT STD_LOGIC;
    rgmii_phy_txd : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
    rgmii_phy_tx_ctl : OUT STD_LOGIC;
    rgmii_phy_rxc : IN STD_LOGIC;
    rgmii_phy_rxd : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
    rgmii_phy_rx_ctl : IN STD_LOGIC
  );
END design_1_mii2rgmii_0_0;

ARCHITECTURE design_1_mii2rgmii_0_0_arch OF design_1_mii2rgmii_0_0 IS
  ATTRIBUTE DowngradeIPIdentifiedWarnings : STRING;
  ATTRIBUTE DowngradeIPIdentifiedWarnings OF design_1_mii2rgmii_0_0_arch: ARCHITECTURE IS "yes";
  COMPONENT mii2rgmii IS
    PORT (
      rgmii_clk : IN STD_LOGIC;
      mii_phy_tx_data : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      mii_phy_tx_en : IN STD_LOGIC;
      mii_phy_tx_er : IN STD_LOGIC;
      mii_phy_rx_data : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
      mii_phy_dv : OUT STD_LOGIC;
      mii_phy_rx_er : OUT STD_LOGIC;
      mii_phy_crs : OUT STD_LOGIC;
      mii_phy_col : OUT STD_LOGIC;
      rgmii_phy_txc : OUT STD_LOGIC;
      rgmii_phy_txd : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
      rgmii_phy_tx_ctl : OUT STD_LOGIC;
      rgmii_phy_rxc : IN STD_LOGIC;
      rgmii_phy_rxd : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
      rgmii_phy_rx_ctl : IN STD_LOGIC
    );
  END COMPONENT mii2rgmii;
  ATTRIBUTE X_CORE_INFO : STRING;
  ATTRIBUTE X_CORE_INFO OF design_1_mii2rgmii_0_0_arch: ARCHITECTURE IS "mii2rgmii,Vivado 2019.2";
  ATTRIBUTE CHECK_LICENSE_TYPE : STRING;
  ATTRIBUTE CHECK_LICENSE_TYPE OF design_1_mii2rgmii_0_0_arch : ARCHITECTURE IS "design_1_mii2rgmii_0_0,mii2rgmii,{}";
  ATTRIBUTE CORE_GENERATION_INFO : STRING;
  ATTRIBUTE CORE_GENERATION_INFO OF design_1_mii2rgmii_0_0_arch: ARCHITECTURE IS "design_1_mii2rgmii_0_0,mii2rgmii,{x_ipProduct=Vivado 2019.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=mii2rgmii,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VHDL,x_ipSimLanguage=MIXED}";
  ATTRIBUTE IP_DEFINITION_SOURCE : STRING;
  ATTRIBUTE IP_DEFINITION_SOURCE OF design_1_mii2rgmii_0_0_arch: ARCHITECTURE IS "module_ref";
  ATTRIBUTE X_INTERFACE_INFO : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
  ATTRIBUTE X_INTERFACE_PARAMETER OF rgmii_clk: SIGNAL IS "XIL_INTERFACENAME rgmii_clk, FREQ_HZ 25000000, PHASE 0.0, CLK_DOMAIN /clk_wiz_1_clk_out1, INSERT_VIP 0";
  ATTRIBUTE X_INTERFACE_INFO OF rgmii_clk: SIGNAL IS "xilinx.com:signal:clock:1.0 rgmii_clk CLK";
BEGIN
  U0 : mii2rgmii
    PORT MAP (
      rgmii_clk => rgmii_clk,
      mii_phy_tx_data => mii_phy_tx_data,
      mii_phy_tx_en => mii_phy_tx_en,
      mii_phy_tx_er => mii_phy_tx_er,
      mii_phy_rx_data => mii_phy_rx_data,
      mii_phy_dv => mii_phy_dv,
      mii_phy_rx_er => mii_phy_rx_er,
      mii_phy_crs => mii_phy_crs,
      mii_phy_col => mii_phy_col,
      rgmii_phy_txc => rgmii_phy_txc,
      rgmii_phy_txd => rgmii_phy_txd,
      rgmii_phy_tx_ctl => rgmii_phy_tx_ctl,
      rgmii_phy_rxc => rgmii_phy_rxc,
      rgmii_phy_rxd => rgmii_phy_rxd,
      rgmii_phy_rx_ctl => rgmii_phy_rx_ctl
    );
END design_1_mii2rgmii_0_0_arch;

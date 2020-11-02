----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/29/2020 05:09:02 PM
-- Design Name: 
-- Module Name: mii2rgmii - Behavioral


-- Revision:A
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity mii2rgmii is
  Port (
    rgmii_clk       : in std_logic;
    --MII
    --mii_phy_tx_clk	: out std_logic; 
    mii_phy_tx_data	: in std_logic_vector(3 downto 0); 
    mii_phy_tx_en	: in std_logic; 
    mii_phy_tx_er	: in std_logic;
    --mii_phy_rx_clk	: out std_logic;
    mii_phy_rx_data	: out std_logic_vector(3 downto 0); 
    mii_phy_dv : out std_logic;	
    mii_phy_rx_er : out std_logic;	
    mii_phy_crs	  : out std_logic; 
    mii_phy_col	  : out std_logic;
    
    --RGMII
    rgmii_phy_txc	: out std_logic; 
    rgmii_phy_txd : out std_logic_vector(3 downto 0);	
    rgmii_phy_tx_ctl : out std_logic;	
    rgmii_phy_rxc	: in std_logic; 
    rgmii_phy_rxd: in std_logic_vector(3 downto 0);	
    rgmii_phy_rx_ctl	: in std_logic 
        
          );
end mii2rgmii;

architecture Behavioral of mii2rgmii is

signal rx_data_valid : std_logic;
begin
--connect clocks

--connect rx/tx bits
rgmii_phy_txd <= mii_phy_tx_data;
mii_phy_rx_data <= rgmii_phy_rxd;

--crs not used in rgmii
mii_phy_crs <= '0';
mii_phy_col <= '0';
--Tx enable and error to ctl signals
process(rgmii_clk)
begin
    if rising_edge(rgmii_clk) then
        rgmii_phy_tx_ctl <= mii_phy_tx_en;
        mii_phy_dv <= rgmii_phy_rx_ctl;
    end if;
    if falling_edge(rgmii_clk) then
        rgmii_phy_tx_ctl <= mii_phy_tx_en xor mii_phy_tx_er;
        if rgmii_phy_rx_ctl = '0' then
            -- either no send or error
            mii_phy_dv <= '0';
        else
            mii_phy_dv <= rgmii_phy_rx_ctl; 
        end if;
    end if;
end process;


end Behavioral;

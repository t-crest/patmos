--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
--         Rasmus Bo Soerensen (rasmus@rbscloud.dk)
--         Wolfgang Puffitsch (rasmus@rbscloud.dk)
--         Torur Strom (torur.strom@gmail.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Xilinx ml605 board with on-chip memory and ethernet core
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
--      oLedsPins_led : out std_logic_vector(8 downto 0);
        oUartPins_txd : out std_logic;
        iUartPins_rxd : in  std_logic;
		
		-- 200MHz clock input from board
        clk_in_p             : in  std_logic;
		clk_in_n             : in  std_logic;
			
		-- asynchronous reset
        glbl_rst           : in  std_logic;
			
		-- reset for physical interface
		phy_resetn             : out  std_logic;
			
		-- GMII Interface
		-----------------
		gmii_txd        : out std_logic_vector(7 downto 0);
		gmii_tx_en             : out  std_logic;
		gmii_tx_er             : out  std_logic;
		gmii_tx_clk             : out  std_logic;
		gmii_rxd        : in std_logic_vector(7 downto 0);
		gmii_rx_dv             : in  std_logic;
		gmii_rx_er             : in  std_logic;
		gmii_rx_clk     : in  std_logic;
		gmii_col             : in  std_logic;
		gmii_crs             : in  std_logic;
		mii_tx_clk             : in  std_logic
    );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
            clock           : in  std_logic;
            reset           : in  std_logic;

            io_comConf_M_Cmd        : out std_logic_vector(2 downto 0);
            io_comConf_M_Addr       : out std_logic_vector(31 downto 0);
            io_comConf_M_Data       : out std_logic_vector(31 downto 0);
            io_comConf_M_ByteEn     : out std_logic_vector(3 downto 0);
            io_comConf_M_RespAccept : out std_logic;
            io_comConf_S_Resp       : in std_logic_vector(1 downto 0);
            io_comConf_S_Data       : in std_logic_vector(31 downto 0);
            io_comConf_S_CmdAccept  : in std_logic;

            io_comSpm_M_Cmd         : out std_logic_vector(2 downto 0);
            io_comSpm_M_Addr        : out std_logic_vector(31 downto 0);
            io_comSpm_M_Data        : out std_logic_vector(31 downto 0);
            io_comSpm_M_ByteEn      : out std_logic_vector(3 downto 0);
            io_comSpm_S_Resp        : in std_logic_vector(1 downto 0);
            io_comSpm_S_Data        : in std_logic_vector(31 downto 0);

            io_cpuInfoPins_id   : in  std_logic_vector(31 downto 0);
            io_ledsPins_led : out std_logic_vector(8 downto 0);
            io_uartPins_tx  : out std_logic;
            io_uartPins_rx  : in  std_logic;
	
			io_eMACPins_glbl_rst  : in std_logic;
			
			io_eMACPins_gtx_clk_bufg  : in std_logic;
			io_eMACPins_cpu_clk  : in std_logic;
			io_eMACPins_refclk_bufg  : in std_logic;
			io_eMACPins_dcm_lck  : in std_logic;
			
			
			io_eMACPins_phy_resetn  : out std_logic;
			
			
			io_eMACPins_gmii_txd   : out  std_logic_vector(7 downto 0);
			io_eMACPins_gmii_tx_en  : out std_logic;
			io_eMACPins_gmii_tx_er  : out std_logic;
			io_eMACPins_gmii_tx_clk  : out std_logic;
			
			io_eMACPins_gmii_rxd   : in  std_logic_vector(7 downto 0);
			io_eMACPins_gmii_rx_dv  : in std_logic;
			io_eMACPins_gmii_rx_er  : in std_logic;
			io_eMACPins_gmii_rx_clk  : in std_logic;
			
			io_eMACPins_gmii_col  : in std_logic;
			io_eMACPins_gmii_crs  : in std_logic;
			io_eMACPins_mii_tx_clk  : in std_logic

        );
    end component;

	signal gtx_clk_bufg : std_logic;
    signal cpu_clk : std_logic;
	signal refclk_bufg : std_logic;
	signal dcm_lck : std_logic;

    -- for generation of internal reset
    signal int_res            : std_logic;
    signal res_reg1, res_reg2 : std_logic;
    signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    attribute xilinx_attribute : string;
    attribute xilinx_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin

    --
    --  internal reset generation
    --  should include the PLL lock signal
    --
    process(cpu_clk)
    begin
        if rising_edge(cpu_clk) then
            if (res_cnt /= "111") then
                res_cnt <= res_cnt + 1;
            end if;
            res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
            res_reg2 <= res_reg1;
            int_res  <= res_reg2;
        end if;
    end process;

    comp : Patmos port map(cpu_clk, int_res,
           open, open, open, open, open,
           (others => '0'), (others => '0'), '0',
           open, open, open, open,
           (others => '0'), (others => '0'),
           X"00000000",
           open, -- oLedsPins_led,
           oUartPins_txd, iUartPins_rxd,
		   glbl_rst,
		   gtx_clk_bufg,
		   cpu_clk,
		   refclk_bufg,
		   dcm_lck,
		   phy_resetn,
		   gmii_txd,
		   gmii_tx_en,
		   gmii_tx_er,
		   gmii_tx_clk,
		   gmii_rxd,
		   gmii_rx_dv,
		   gmii_rx_er,
		   gmii_rx_clk,
		   gmii_col,
		   gmii_crs,
		   mii_tx_clk
		   );	   
		   
	clock_generator : entity work.clk_wiz_v2_1 port map(
			clk_in_p, 
			clk_in_n,
			gtx_clk_bufg, 
			cpu_clk, 
			refclk_bufg,
			glbl_rst, 
			dcm_lck
			);

end architecture rtl;

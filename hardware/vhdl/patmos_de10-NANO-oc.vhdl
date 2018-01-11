--
-- Copyright: 2017, Technical University of Denmark, DTU Compute
-- Author: Oktay Baris (okba@dtu.com)

-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Altera DE10-NANO board with on-chip memory
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
        clk : in  std_logic;

	oLedsPins_led : out std_logic_vector(7 downto 0);
	iKeysPins_key : in std_logic_vector(1 downto 0);

        --oADC_CONVST : out std_logic;
        --oADC_SCK : out std_logic;
	--oADC_SDI : out std_logic;
	--iADC_SDO : in std_logic;

        osDHostCtrlPins_sdClk  : out std_logic;
        osDHostCtrlPins_sdCs : out std_logic;
        isDHostCtrlPins_sdDatOut : in std_logic;
        osDHostCtrlPins_sdDatIn : out std_logic;

        oUartPins_txd : out std_logic;
        iUartPins_rxd : in  std_logic
    );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
            clk             : in  std_logic;
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

	    io_LedsPins_led : out std_logic_vector(7 downto 0);
	    io_KeysPins_key : in  std_logic_vector(1 downto 0);

       	    io_sDHostCtrlPins_sdClk  : out std_logic;
            io_sDHostCtrlPins_sdCs : out std_logic;
            io_sDHostCtrlPins_sdDatOut : in std_logic;
            io_sDHostCtrlPins_sdDatIn : out std_logic;

            io_uartPins_tx  : out std_logic;
            io_uartPins_rx  : in  std_logic

        );
    end component;

    -- ML605: 66 MHz clock => 66 MHz
    constant pll_mult : natural := 15;
    constant pll_div  : natural := 15;

    signal clk_int : std_logic;

    -- for generation of internal reset
    signal int_res            : std_logic;
    signal res_reg1, res_reg2 : std_logic;
    signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    attribute xilinx_attribute : string;
    attribute xilinx_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
    pll_inst : entity work.pll generic map(
            multiply_by => pll_mult,
            divide_by   => pll_div
        )
        port map(
	    inclk0 => clk,
	    c0 => clk_int
            --clk_in1 => clk,
            --clk_out1 => clk_int
        );
    -- we use a PLL
    -- clk_int <= clk;

    --
    --  internal reset generation
    --  should include the PLL lock signal
    --
    process(clk_int)
    begin
        if rising_edge(clk_int) then
            if (res_cnt /= "111") then
                res_cnt <= res_cnt + 1;
            end if;
            res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
            res_reg2 <= res_reg1;
            int_res  <= res_reg2;
        end if;
    end process;

    comp : Patmos port map(clk_int, int_res,
           open, open, open, open, open,
           (others => '0'), (others => '0'), '0',
           open, open, open, open,
           (others => '0'), (others => '0'),
	   oLedsPins_led,
	   iKeysPins_key,
           osDHostCtrlPins_sdClk, osDHostCtrlPins_sdCs, isDHostCtrlPins_sdDatOut, osDHostCtrlPins_sdDatIn, 
           oUartPins_txd, iUartPins_rxd);

end architecture rtl;

--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
--         Rasmus Bo Soerensen (rasmus@rbscloud.dk)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Altera de2-115 board
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
	port(
		clk : in  std_logic;
		oLedsPins_led : out std_logic_vector(8 downto 0);
		iKeysPins_key : in std_logic_vector(3 downto 0);
		oUartPins_txd : out std_logic;
		iUartPins_rxd : in  std_logic;
		oVgaPins_r       : out std_logic_vector(7 downto 0);
		oVgaPins_g       : out std_logic_vector(7 downto 0);
		oVgaPins_b       : out std_logic_vector(7 downto 0);
		oVgaPins_clk     : out std_logic;
		oVgaPins_syncn   : out std_logic;
		oVgaPins_blankn  : out std_logic;
		oVgaPins_vs      : out std_logic;
		oVgaPins_hs      : out std_logic;
    oSRAM_A : out std_logic_vector(19 downto 0);
    SRAM_DQ : inout std_logic_vector(15 downto 0);
    oSRAM_CE_N : out std_logic;
    oSRAM_OE_N : out std_logic;
    oSRAM_WE_N : out std_logic;
    oSRAM_LB_N : out std_logic;
    oSRAM_UB_N : out std_logic
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

			io_cpuInfoPins_id   : in  std_logic_vector(31 downto 0);
			io_cpuInfoPins_cnt  : in  std_logic_vector(31 downto 0);
			io_ledsPins_led : out std_logic_vector(8 downto 0);
			io_keysPins_key : in  std_logic_vector(3 downto 0);
			io_uartPins_tx  : out std_logic;
			io_uartPins_rx  : in  std_logic;
		  io_vga_r       : out std_logic_vector(7 downto 0);
		  io_vga_g       : out std_logic_vector(7 downto 0);
		  io_vga_b       : out std_logic_vector(7 downto 0);
		  io_vga_clk     : out std_logic;
		  io_vga_sync_n  : out std_logic;
		  io_vga_blank_n : out std_logic;
		  io_vga_vs      : out std_logic;
		  io_vga_hs      : out std_logic;

      io_sramCtrlPins_ramOut_addr : out std_logic_vector(19 downto 0);
      io_sramCtrlPins_ramOut_doutEna : out std_logic;
      io_sramCtrlPins_ramIn_din : in std_logic_vector(15 downto 0);
      io_sramCtrlPins_ramOut_dout : out std_logic_vector(15 downto 0);
      io_sramCtrlPins_ramOut_nce : out std_logic;
      io_sramCtrlPins_ramOut_noe : out std_logic;
      io_sramCtrlPins_ramOut_nwe : out std_logic;
      io_sramCtrlPins_ramOut_nlb : out std_logic;
      io_sramCtrlPins_ramOut_nub : out std_logic

		);
	end component;

	-- DE2-70: 50 MHz clock => 80 MHz
	-- BeMicro: 16 MHz clock => 25.6 MHz
	constant pll_infreq : real    := 50.0;
	constant pll_mult   : natural := 8;
	constant pll_div    : natural := 5;

	signal clk_int : std_logic;

	-- for generation of internal reset
	signal int_res            : std_logic;
	signal res_reg1, res_reg2 : std_logic;
	signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    -- sram signals for tristate inout
    signal sram_out_dout_ena : std_logic;
    signal sram_out_dout : std_logic_vector(15 downto 0);

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
	pll_inst : entity work.pll generic map(
			input_freq  => pll_infreq,
			multiply_by => pll_mult,
			divide_by   => pll_div
		)
		port map(
			inclk0 => clk,
			c0     => clk_int
		);
	-- we use a PLL
	-- clk_int <= clk;

	--
	--	internal reset generation
	--	should include the PLL lock signal
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


    -- tristate output to ssram
    process(sram_out_dout_ena, sram_out_dout)
    begin
      if sram_out_dout_ena='1' then
        SRAM_DQ <= sram_out_dout;
      else
        SRAM_DQ <= (others => 'Z');
      end if;
    end process;

    comp : Patmos port map(clk_int, int_res,
           open, open, open, open, open,
           (others => '0'), (others => '0'), '0',
           open, open, open, open,
           (others => '0'), (others => '0'),
           X"00000000", X"00000001",
           oLedsPins_led,
           iKeysPins_key,
           oUartPins_txd, iUartPins_rxd,
					 oVgaPins_r, oVgaPins_g, oVgaPins_b, oVgaPins_clk, oVgaPins_syncn, oVgaPins_blankn, oVgaPins_vs, oVgaPins_hs,      
           oSRAM_A, sram_out_dout_ena, SRAM_DQ, sram_out_dout, oSRAM_CE_N, oSRAM_OE_N, oSRAM_WE_N, oSRAM_LB_N, oSRAM_UB_N);

end architecture rtl;

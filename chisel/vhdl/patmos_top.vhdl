--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
	port(
		clk : in  std_logic;
		led : out std_logic_vector(8 downto 0);
		key : in  std_logic_vector(3 downto 0);
		txd : out std_logic;
		rxd : in  std_logic;
                oSRAM_A : out std_logic_vector(18 downto 0);
                oSRAM_ADSC_N : out std_logic;
                oSRAM_OE_N : out std_logic;
                oSRAM_WE_N : out std_logic;
                oSRAM_BE_N : out std_logic_vector(3 downto 0);
                oSRAM_GW_N : out std_logic;
                oSRAM_CE1_N : out std_logic;
                oSRAM_CE2 : out std_logic;
                oSRAM_CE3_N : out std_logic;
                oSRAM_ADSP_N : out std_logic;
                oSRAM_ADV_N : out std_logic;
                SRAM_DQ : inout std_logic_vector(31 downto 0);
                oSRAM_CLK : out std_logic
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
			io_ledsPins_led : out std_logic_vector(8 downto 0);
			io_keysPins_key : out std_logic_vector(3 downto 0);
			io_uartPins_tx  : out std_logic;
			io_uartPins_rx  : in  std_logic;

                        io_sramPins_ram_out_addr : out std_logic_vector(18 downto 0);
                        io_sramPins_ram_out_dout_ena : out std_logic;
                        io_sramPins_ram_out_nadsc : out std_logic;
                        io_sramPins_ram_out_noe : out std_logic;
                        io_sramPins_ram_out_nbwe : out std_logic;
                        io_sramPins_ram_out_nbw : out std_logic_vector(3 downto 0);
                        io_sramPins_ram_out_ngw : out std_logic;
                        io_sramPins_ram_out_nce1 : out std_logic;
                        io_sramPins_ram_out_ce2 : out std_logic;
                        io_sramPins_ram_out_nce3 : out std_logic;
                        io_sramPins_ram_out_nadsp : out std_logic;
                        io_sramPins_ram_out_nadv : out std_logic;
                        io_sramPins_ram_out_dout : out std_logic_vector(31 downto 0);
                        io_sramPins_ram_in_din : inout std_logic_vector(31 downto 0)
		);
	end component;

	-- DE2-70: 50 MHz clock => 80 MHz
	-- BeMicro: 16 MHz clock => 25.6 MHz
	constant pll_mult : natural := 8;
	constant pll_div  : natural := 5;

	signal clk_int : std_logic;

	-- for generation of internal reset
	signal int_res            : std_logic;
	signal res_reg1, res_reg2 : std_logic;
	signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

        -- sram signals for tristate inout
        signal sram_out_dout_ena : std_logic;
        signal sram_out_dout : std_logic_vector(31 downto 0);
        signal sram_in_din : std_logic_vector(31 downto 0);
        signal sram_in_din_reg : std_logic_vector(31 downto 0);
        
	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
	pll_inst : entity work.pll generic map(
			multiply_by => pll_mult,
			divide_by   => pll_div
		)
		port map(
			inclk0 => clk,
			c0     => clk_int,
			c1     => oSRAM_CLK
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

        -- capture input from ssram on falling clk edge
        process(clk_int, int_res)
        begin
          if int_res='1' then
            sram_in_din_reg <= (others => '0');
          elsif falling_edge(clk_int) then
            sram_in_din_reg <= SRAM_DQ;
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

        -- input of tristate on positive clk edge
        process(clk_int)
	  begin
	    if rising_edge(clk_int) then
              sram_in_din <= sram_in_din_reg;
            end if;
	end process;

        comp : Patmos port map(clk_int, int_res,
                           open, open, open, open, open,
                           (others => '0'), (others => '0'), '0',
                           open, open, open, open,
                           (others => '0'), (others => '0'),
                           X"00000000",
                           led,
                           key,
                           txd, rxd,
                           oSRAM_A, sram_out_dout_ena, oSRAM_ADSC_N, oSRAM_OE_N, oSRAM_WE_N, oSRAM_BE_N, oSRAM_GW_N, oSRAM_CE1_N, oSRAM_CE2, oSRAM_CE3_N, oSRAM_ADSP_N, oSRAM_ADV_N, sram_out_dout, sram_in_din);

end architecture rtl;

--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Eleftherios Kyriakakis (elky@dtu.dk)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on an Arrow MAX1000 board
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
		iKeysPins_key : in std_logic_vector(0 downto 0)
	);
end entity patmos_top;

architecture rtl of patmos_top is

	component Patmos is
        port(
            clk                                   : in  std_logic;
            reset                                 : in  std_logic;

            io_comConf_M_Cmd                      : out std_logic_vector(2 downto 0);
            io_comConf_M_Addr                     : out std_logic_vector(31 downto 0);
            io_comConf_M_Data                     : out std_logic_vector(31 downto 0);
            io_comConf_M_ByteEn                   : out std_logic_vector(3 downto 0);
            io_comConf_M_RespAccept               : out std_logic;
            io_comConf_S_Resp                     : in  std_logic_vector(1 downto 0);
            io_comConf_S_Data                     : in  std_logic_vector(31 downto 0);
            io_comConf_S_CmdAccept                : in  std_logic;

            io_comSpm_M_Cmd                       : out std_logic_vector(2 downto 0);
            io_comSpm_M_Addr                      : out std_logic_vector(31 downto 0);
            io_comSpm_M_Data                      : out std_logic_vector(31 downto 0);
            io_comSpm_M_ByteEn                    : out std_logic_vector(3 downto 0);
            io_comSpm_S_Resp                      : in  std_logic_vector(1 downto 0);
            io_comSpm_S_Data                      : in  std_logic_vector(31 downto 0);

            io_ledsPins_led                       : out std_logic_vector(7 downto 0);
            io_keysPins_key                       : in  std_logic_vector(0 downto 0)
            -- io_uartPins_tx                        : out std_logic;
            -- io_uartPins_rx                        : in  std_logic
            );
    end component;

	-- max1000: 12 MHz clock => 40 MHz
	constant pll_infreq : real    := 12.0;
	constant pll_mult   : natural := 12;
	constant pll_div    : natural := 3;

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
			device_fam => "MAX 10",
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

	patmos_inst : Patmos port map(
        clk => clk_int,
        reset => '0',
        io_comConf_M_Cmd => open,
        io_comConf_M_Addr => open,
        io_comConf_M_Data => open,
        io_comConf_M_ByteEn => open,
        io_comConf_M_RespAccept => open,
        io_comConf_S_Resp => (others => '0'),
        io_comConf_S_Data => (others => '0'),
        io_comConf_S_CmdAccept => '0',
        io_comSpm_M_Cmd => open,
        io_comSpm_M_Addr => open,
        io_comSpm_M_Data => open,
        io_comSpm_M_ByteEn => open,
        io_comSpm_S_Resp => (others => '0'),
		io_comSpm_S_Data => (others => '0'),
        io_ledsPins_led => oLedsPins_led,
        io_keysPins_key => iKeysPins_key
    );

end architecture rtl;

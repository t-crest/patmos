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
		--		rst : in std_logic;
		led : out std_logic_vector(7 downto 0)
	);
end entity patmos_top;

architecture rtl of patmos_top is
	component FsmContainer is
		port(
			clk    : in  std_logic;
			reset  : in  std_logic;
			io_led : out std_logic_vector(7 downto 0)
		);
	end component;

	signal clk_int : std_logic;

	-- for generation of internal reset
	signal int_res : std_logic;
	signal res_cnt : unsigned(2 downto 0) := "000"; -- for the simulation

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin

	-- we could use a PLL in future designs
	clk_int <= clk;
	
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
			int_res <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
		end if;
	end process;

	comp : FsmContainer port map(
			clk, int_res, led
		);

end architecture rtl;

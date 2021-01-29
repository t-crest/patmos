--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--
-- True dual-port single-clock BRAM definition
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tdp_sc_bram is
	generic(
		DATA_WIDTH : natural;
		ADDR_WIDTH : natural
	);
	port(
		clk      : in  std_logic;
		addr_a   : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		addr_b   : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		data_a_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
		data_b_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
		we_a     : in  std_logic;
		we_b     : in  std_logic;
		data_a_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
		data_b_o : out std_logic_vector((DATA_WIDTH - 1) downto 0)
	);
end tdp_sc_bram;

architecture rtl of tdp_sc_bram is
	-- Build a 2-D array type for the RAM
	type memory_t is array ((2 ** ADDR_WIDTH - 1) downto 0) of std_logic_vector((DATA_WIDTH - 1) downto 0);
	-- Declare the RAM variable.
	shared variable ram : memory_t;

begin
	process(clk)
	begin
		if (rising_edge(clk)) then      -- Port A
			if (we_a = '1') then
				ram(to_integer(unsigned(addr_a))) := data_a_i;
				-- Read-during-write on the same port returns NEW data
				data_a_o                          <= data_a_i;
			else
				-- Read-during-write on the mixed port returns OLD data
				data_a_o <= ram(to_integer(unsigned(addr_a)));
			end if;
		end if;
	end process;

	process(clk)
	begin
		if (rising_edge(clk)) then      -- Port B
			if (we_b = '1') then
				ram(to_integer(unsigned(addr_b))) := data_b_i;
				-- Read-during-write on the same port returns NEW data
				data_b_o                          <= data_b_i;
			else
				-- Read-during-write on the mixed port returns OLD data
				data_b_o <= ram(to_integer(unsigned(addr_b)));
			end if;
		end if;
	end process;
end rtl;

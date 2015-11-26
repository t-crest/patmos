--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--
-- OCP to BRAM bridge
--

library ieee;
use ieee.std_logic_1164.all;

entity ocp_to_bram is
	generic(
		DATA_WIDTH : natural;
		ADDR_WIDTH : natural
	);
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;

		-- OCP IN (slave)
		MCmd       : in  std_logic_vector(2 downto 0);
		MAddr      : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		MData      : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
		MByteEn    : in  std_logic_vector(3 downto 0);
		SResp      : out std_logic_vector(1 downto 0);
		SData      : out std_logic_vector((DATA_WIDTH - 1) downto 0);

		-- Ram OUT (byte based)
		ram_addr   : out std_logic_vector((ADDR_WIDTH - 1) downto 0);
		ram_data_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
		ram_we     : out std_logic_vector(3 downto 0);
		ram_data_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0)
	);
end ocp_to_bram;

architecture rtl of ocp_to_bram is
	signal next_SResp : std_logic_vector(1 downto 0);

begin
	ram_addr <= MAddr;
	ram_data_o <= MData;
	SData <= ram_data_i;

	--Control mux
	process(MCmd, MByteEn)
	begin
		case MCmd is
			when "001" =>               -- write
				ram_we     <= MByteEn;
				next_SResp <= "01";
			when "010" =>               -- read
				ram_we     <= "0000";
				next_SResp <= "01";
			when others =>              -- idle
				ram_we     <= "0000";
				next_SResp <= "00";
		end case;
	end process;

	--Register
	process(clk, rst)
	begin
		if rst = '1' then
			SResp <= (others => '0');
		elsif rising_edge(clk) then
			SResp <= next_SResp;
		end if;
	end process;

end rtl;

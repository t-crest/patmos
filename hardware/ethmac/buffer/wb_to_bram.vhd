--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--
-- Wishbone to BRAM bridge
--

library ieee;
use ieee.std_logic_1164.all;

entity wb_to_bram is
	generic(
		DATA_WIDTH : natural;
		ADDR_WIDTH : natural
	);
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;

		-- Wishbone IN
		wb_addr_i  : in  std_logic_vector((ADDR_WIDTH - 1) downto 0);
		wb_sel_i   : in  std_logic_vector(3 downto 0);
		wb_we_i    : in  std_logic;
		wb_data_o  : out std_logic_vector((DATA_WIDTH - 1) downto 0);
		wb_data_i  : in  std_logic_vector((DATA_WIDTH - 1) downto 0);
		wb_cyc_i   : in  std_logic;
		wb_stb_i   : in  std_logic;
		wb_ack_o   : out std_logic;
		wb_err_o   : out std_logic;

		-- Ram OUT
		ram_addr   : out std_logic_vector((ADDR_WIDTH - 1) downto 0);
		ram_data_o : out std_logic_vector((DATA_WIDTH - 1) downto 0);
		ram_we     : out std_logic;
		ram_data_i : in  std_logic_vector((DATA_WIDTH - 1) downto 0)
	);
end wb_to_bram;

architecture rtl of wb_to_bram is
	type state_type is (IDLE, WRITE, READ, ACK);
	signal state, next_state : state_type;

begin
	ram_addr   <= wb_addr_i;
	ram_data_o <= wb_data_i;
	wb_data_o  <= ram_data_i;
	wb_err_o   <= '0';

	process(state, wb_stb_i, wb_we_i, wb_cyc_i)
	begin
		ram_we     <= '0';
		wb_ack_o   <= '0';
		next_state <= state;
		case state is
			when IDLE =>
				if (wb_stb_i = '1') and (wb_cyc_i = '1') then
					if wb_we_i = '0' then
						next_state <= READ;
					else
						next_state <= WRITE;
					end if;
				else
					next_state <= IDLE;
				end if;
			when WRITE =>
				ram_we     <= '1';
				next_state <= ACK;
			when READ =>
				next_state <= ACK;
			when ACK =>
				wb_ack_o   <= '1';
				next_state <= IDLE;
		end case;
	end process;

	process(clk, rst)
	begin
		if rst = '1' then
			state <= IDLE;
		elsif rising_edge(clk) then
			state <= next_state;
		end if;
	end process;

end rtl;

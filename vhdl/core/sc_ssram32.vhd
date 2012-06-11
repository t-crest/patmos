--
--
--  This file is a part of JOP, the Java Optimized Processor
--
--  Copyright (C) 2001-20010, Martin Schoeberl (martin@jopdesign.com)
--
--  This program is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program.  If not, see <http://www.gnu.org/licenses/>.
--


--
--	sc_ssram32.vhd
--
--	SimpCon compliant external memory interface
--	for 32-bit SSRAM (e.g. Altera DE2-70 board)
--
--	Connection between mem_sc and the external memory bus
--
--	memory mapping
--	
--		000000-x7ffff	external SRAM (w mirror)	max. 512 kW (4*4 MBit)
--
--	RAM: 32 bit word
--
--
--	2005-11-22	first version
--	2007-03-17	changed SimpCon to records
--	2008-05-29	nwe on pos edge, additional wait state for write
--	2010-06-25  adapted from sc_sram32.vhd for synchronous SRAM
--

Library IEEE;
use IEEE.std_logic_1164.all;
use ieee.numeric_std.all;

--use work.jop_types.all;
use work.sc_pack.all;

entity sc_mem_if is
generic (ram_ws : integer; addr_bits : integer);

port (

	clk, reset	: in std_logic;
	clk2		: in std_logic;	-- an inverted clock

--
--	SimpCon memory interface
--
	sc_mem_out		: in sc_out_type;
	sc_mem_in		: out sc_in_type;

-- memory interface

	ram_addr	: out std_logic_vector(addr_bits-1 downto 0);
	ram_dout	: out std_logic_vector(31 downto 0);
	ram_din		: in std_logic_vector(31 downto 0);
	ram_dout_en	: out std_logic;
	ram_clk		: out std_logic;
	ram_nsc		: out std_logic;
	ram_ncs		: out std_logic;
	ram_noe		: out std_logic;
	ram_nwe		: out std_logic

);
end sc_mem_if;

architecture rtl of sc_mem_if is

--
--	signals for mem interface
--
	type state_type		is (
							idl, rd1, rd2,
							wr1, wr2
						);
	signal state 		: state_type;
	signal next_state	: state_type;

	signal wait_state	: unsigned(3 downto 0);
	signal cnt			: unsigned(1 downto 0);

	signal dout_ena		: std_logic;
	signal rd_data_ena	: std_logic;
	
	signal ram_ws_wr	: integer;

	signal ram_din_reg : std_logic_vector(31 downto 0);
	
begin
	
	assert SC_ADDR_SIZE>=addr_bits report "Not enough address bits";

	ram_ws_wr <= ram_ws; -- no additional wait state for SSRAM

	ram_dout_en <= dout_ena;
	
	sc_mem_in.rdy_cnt <= cnt;

--
--	Register memory address, write data and read data
--
process(clk, reset)
begin
	if reset='1' then

		ram_addr <= (others => '0');
		ram_dout <= (others => '0');
		sc_mem_in.rd_data <= (others => '0');

	elsif rising_edge(clk) then

		if sc_mem_out.rd='1' or sc_mem_out.wr='1' then
			ram_addr <= sc_mem_out.address(addr_bits-1 downto 0);
		end if;
		if sc_mem_out.wr='1' then
			ram_dout <= sc_mem_out.wr_data;
		end if;
		if rd_data_ena='1' then
			sc_mem_in.rd_data <= ram_din_reg;
		end if;

	end if;
end process;

-- inverted clock to have some setup time
-- ram_clk <= not clk;

ram_clk <= clk2;

process(clk2, reset)
begin
	-- latch input data on negative edge to help hold time (WP)
	-- MS: at least use the generated PLL clock that is 180 phase shifted
	-- MS: I'm still unsure if this is the best way to do it
	if reset='1' then
		ram_din_reg <= (others => '0');
--	elsif falling_edge(clk) then
	elsif rising_edge(clk2) then
		ram_din_reg <= ram_din;
	end if;
end process;

--
--	next state logic
--
process(state, sc_mem_out.rd, sc_mem_out.wr, wait_state)

begin

	next_state <= state;

	case state is

		when idl =>
			if sc_mem_out.rd='1' then
				if ram_ws=0 then
					-- then we omit state rd1!
					next_state <= rd2;
				else
					next_state <= rd1;
				end if;
			elsif sc_mem_out.wr='1' then
				next_state <= wr1;
			end if;

		-- the read address state
		when rd1 =>
			next_state <= rd2;

		-- the read fetch state
		when rd2 =>
			if wait_state=1 then
				next_state <= idl;
			end if;
			-- This should do to give us a pipeline
			-- level of 2 for read
			if sc_mem_out.rd='1' then
				if ram_ws=0 then
					-- then we omit state rd1!
					next_state <= rd2;
				else
					next_state <= rd1;
				end if;
			elsif sc_mem_out.wr='1' then
				next_state <= wr1;
			end if;
			
		-- the write address/data state
		when wr1 =>
			next_state <= wr2;
		
		-- the write enable state
		when wr2 =>
			if wait_state=1 then
				next_state <= idl;
			end if;
			-- This should do to give us a pipeline
			-- level of 2 for write
			if sc_mem_out.rd='1' then
				if ram_ws=0 then
					-- then we omit state rd1!
					next_state <= rd2;
				else
					next_state <= rd1;
				end if;
			elsif sc_mem_out.wr='1' then
				next_state <= wr1;
			end if;

	end case;
				
end process;

--
--	state machine register
--	output register
--
process(clk, reset)

begin
	if (reset='1') then
		state <= idl;
		dout_ena <= '0';
		ram_nsc <= '1';
		ram_ncs <= '1';
		ram_noe <= '1';
		rd_data_ena <= '0';
		ram_nwe <= '1';
		
	elsif rising_edge(clk) then

		state <= next_state;
		dout_ena <= '0';
		ram_nsc <= '1';
		ram_ncs <= '1';
		ram_noe <= '1';
		rd_data_ena <= '0';
		ram_nwe <= '1';
		
		case next_state is

			when idl =>

			-- the wait state
			when rd1 =>
				ram_nsc <= '0';
				ram_ncs <= '0';
				ram_noe <= '0';

			-- last read state
			when rd2 =>
				ram_noe <= '0';
				if wait_state=2 then
					rd_data_ena <= '1';					
				end if;
				
			-- write address/data state
			when wr1 =>
				ram_nsc <= '0';
				ram_ncs <= '0';
				ram_nwe <= '0';
				dout_ena <= '1';
			
			-- write enable state	
			when wr2 =>
				dout_ena <= '1';

		end case;
					
	end if;
end process;

--
-- wait_state processing
-- cs delay, dout enable
--
process(clk, reset)
begin
	if (reset='1') then
		wait_state <= (others => '1');
		cnt <= "00";
	elsif rising_edge(clk) then

		wait_state <= wait_state-1;

		cnt <= "11";
		if next_state=idl then
			cnt <= "00";
		-- if wait_state<4 then
		elsif wait_state(3 downto 2)="00" then
			cnt <= wait_state(1 downto 0)-1;
		end if;

		if sc_mem_out.rd='1' then
			wait_state <= to_unsigned(ram_ws+1, 4);
			if ram_ws<3 then
				cnt <= to_unsigned(ram_ws+1, 2);
			else
				cnt <= "11";
			end if;
		end if;
		
		if sc_mem_out.wr='1' then
			wait_state <= to_unsigned(ram_ws_wr+1, 4);
			if ram_ws_wr<3 then
				cnt <= to_unsigned(ram_ws_wr+1, 2);
			else
				cnt <= "11";
			end if;
		end if;

	end if;
end process;

end rtl;

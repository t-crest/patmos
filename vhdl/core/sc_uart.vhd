-- 
-- Copyright (C) 2001-2008, Martin Schoeberl (martin@jopdesign.com)
-- This file is part of JOP and the time-predictable VLIW Patmos.
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
-- 
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
-- 
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
-- 
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
-- NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-- 
-- The views and conclusions contained in the software and documentation are
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holder.
-- 


--------------------------------------------------------------------------------
-- Short descripton.
--
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------

--
--	sc_uart.vhd
--
--	8-N/E/O-1 serial interface
--	
--	wr, rd should be one cycle long => trde, rdrf goes 0 one cycle later
--
--	Author: Martin Schoeberl	martin@jopdesign.com
--
--
--	resources on ACEX1K30-3
--
--		100 LCs, max 90 MHz
--
--	resetting rts with fifo_full-1 works with C program on pc
--	but not with javax.comm: sends some more bytes after deassert
--	of rts (16 byte blocks regardless of rts).
--	Try to stop with half full fifo.
--
--	todo:
--
--
--	2000-12-02	first working version
--	2002-01-06	changed tdr and rdr to fifos.
--	2002-05-15	changed clkdiv calculation
--	2002-11-01	don't wait if read fifo is full, just drop the byte
--	2002-11-03	use threshold in fifo to reset rts 
--				don't send if cts is '0'
--	2002-11-08	rx fifo to 20 characters and stop after 4
--	2003-07-05	new IO standard, change cts/rts to neg logic
--	2003-09-19	sync ncts in!
--	2004-03-23	two stop bits
--	2005-11-30	change interface to SimpCon
--	2006-08-07	rxd input register with clk to avoid Quartus tsu violation
--	2006-08-13	use 3 FFs for the rxd input at clk
--  2009-03-03  incorporate changes for parity bits


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sc_uart is

	generic (addr_bits : integer;
			 clk_freq : integer;
			 baud_rate : integer;
			 txf_depth : integer; txf_thres : integer;
			 rxf_depth : integer; rxf_thres : integer);
	port (
		clk		: in std_logic;
		reset	: in std_logic;

-- SimpCon interface

		address		: in std_logic_vector(addr_bits-1 downto 0);
		wr_data		: in std_logic_vector(31 downto 0);
		rd, wr		: in std_logic;
		rd_data		: out std_logic_vector(31 downto 0);
		rdy_cnt		: out unsigned(1 downto 0);

		txd		: out std_logic;
		rxd		: in std_logic;
		ncts	: in std_logic;
		nrts	: out std_logic;

-- Signals to aid CMP front-end
		tf_ready : out std_logic;
		rf_avail : out std_logic;
		pa_error : out std_logic
		);
end sc_uart;

architecture rtl of sc_uart is

	component fifo is

		generic (width : integer; depth : integer; thres : integer);
		port (
			clk		: in std_logic;
			reset	: in std_logic;

			din		: in std_logic_vector(width-1 downto 0);
			dout	: out std_logic_vector(width-1 downto 0);

			rd		: in std_logic;
			wr		: in std_logic;

			empty	: out std_logic;
			full	: out std_logic;
			half	: out std_logic
			);
	end component;

--
--	signals for uart connection
--
	signal ua_dout			: std_logic_vector(7 downto 0);
	signal ua_wr, tdre		: std_logic;
	signal ua_rd, rdrf		: std_logic;

	type uart_tx_state_type		is (s0, s1);
	signal uart_tx_state 		: uart_tx_state_type;

	signal tf_dout		: std_logic_vector(7 downto 0); -- fifo out
	signal tf_rd		: std_logic;
	signal tf_empty		: std_logic;
	signal tf_full		: std_logic;
	signal tf_half		: std_logic;

	signal ncts_buf		: std_logic_vector(2 downto 0);	-- sync in

	signal tsr			: std_logic_vector(10 downto 0); -- tx shift register

	signal tx_clk		: std_logic;


	type uart_rx_state_type		is (s0, s1, s2);
	signal uart_rx_state 		: uart_rx_state_type;

	signal rf_wr		: std_logic;
	signal rf_empty		: std_logic;
	signal rf_full		: std_logic;
	signal rf_half		: std_logic;

	signal rxd_reg		: std_logic_vector(2 downto 0);
	signal rx_buf		: std_logic_vector(2 downto 0);	-- sync in, filter
	signal rx_d			: std_logic;					-- rx serial data
	
	signal rsr			: std_logic_vector(10 downto 0); -- rx shift register

	signal rx_clk		: std_logic;
	signal rx_clk_ena	: std_logic;
	
	constant PARITY_NONE : std_logic_vector(1 downto 0) := "00";
	constant PARITY_ODD : std_logic_vector(1 downto 0) := "01";
	constant PARITY_EVEN : std_logic_vector(1 downto 0) := "11";

	signal parity_mode  : std_logic_vector(1 downto 0) := PARITY_NONE;
	signal parity_error  : std_logic;

	constant clk16_cnt	: integer := (clk_freq/baud_rate+8)/16-1;
	

begin

	rdy_cnt <= "00";	-- no wait states
	rd_data(31 downto 8) <= std_logic_vector(to_unsigned(0, 24));
	pa_error <= '0';
--
--	The registered MUX is all we need for a SimpCon read.
--	The read data is stored in registered rd_data.
--
	process(clk, reset)
	begin

		if (reset='1') then
			rd_data(7 downto 0) <= (others => '0');
			parity_mode <= PARITY_NONE;
		elsif rising_edge(clk) then

			ua_rd <= '0';
			if rd='1' then
				-- that's our very simple address decoder
				if address(0)='0' then
					rd_data(7 downto 0) <= "00000" & parity_error & rdrf & tdre;
				else
					rd_data(7 downto 0) <= ua_dout;
					ua_rd <= rd;
				end if;
			end if;
			if wr = '1' and address(0) = '0' then
				parity_mode(1 downto 0) <= wr_data(1 downto 0);
			end if;
		end if;

	end process;
	
	-- write is on address offset 1	
	ua_wr <= wr and address(0);

	tf_ready <= tdre;
	rf_avail <= rdrf;
	
--
--	serial clock
--
	process(clk, reset)

		variable clk16		: integer range 0 to clk16_cnt;
		variable clktx		: unsigned(3 downto 0);
		variable clkrx		: unsigned(3 downto 0);

	begin
		if (reset='1') then
			clk16 := 0;
			clktx := "0000";
			clkrx := "0000";
			tx_clk <= '0';
			rx_clk <= '0';
			rx_buf <= "111";

		elsif rising_edge(clk) then

			rxd_reg(0) <= rxd;			-- to avoid setup timing error in Quartus
			rxd_reg(1) <= rxd_reg(0);
			rxd_reg(2) <= rxd_reg(1);

			if (clk16=clk16_cnt) then		-- 16 x serial clock
				clk16 := 0;
--
--	tx clock
--
				clktx := clktx + 1;
				if (clktx="0000") then
					tx_clk <= '1';
				else
					tx_clk <= '0';
				end if;
--
--	rx clock
--
				if (rx_clk_ena='1') then
					clkrx := clkrx + 1;
					if (clkrx="1000") then
						rx_clk <= '1';
					else
						rx_clk <= '0';
					end if;
				else
					clkrx := "0000";
				end if;
--
--	sync in filter buffer
--
				rx_buf(0) <= rxd_reg(2);
				rx_buf(2 downto 1) <= rx_buf(1 downto 0);
			else
				clk16 := clk16 + 1;
				tx_clk <= '0';
				rx_clk <= '0';
			end if;


		end if;

	end process;


--
--	transmit fifo
--
	tf: fifo generic map (8, txf_depth, txf_thres)
		port map (clk, reset, wr_data(7 downto 0), tf_dout, tf_rd, ua_wr, tf_empty, tf_full, tf_half);

--
--	state machine for actual shift out
--
	process(clk, reset)

		variable i : integer range 0 to 12;
		variable parity_tx  : std_logic;

	begin
		

		if (reset='1') then
			uart_tx_state <= s0;
			tsr <= "11111111111";
			tf_rd <= '0';
			ncts_buf <= "111";

		elsif rising_edge(clk) then

			ncts_buf(0) <= ncts;
			ncts_buf(2 downto 1) <= ncts_buf(1 downto 0);

			case uart_tx_state is

				when s0 =>

					-- even parity
					parity_tx := ( (tf_dout(7) xor tf_dout(6)) xor (tf_dout(5) xor tf_dout(4)) ) xor ( (tf_dout(3) xor tf_dout(2)) xor (tf_dout(1) xor tf_dout(0)) );

					if (parity_mode = PARITY_ODD) then   -- odd parity
						parity_tx := not parity_tx;
					elsif (parity_mode = PARITY_NONE) then  -- no parity, stop bit
						parity_tx := '1';
					end if;
					
					i := 0;
					if (tf_empty='0' and ncts_buf(2)='0') then
						uart_tx_state <= s1;
						tsr <= parity_tx & tf_dout & '0' & '1';
						tf_rd <= '1';
					end if; 

				when s1 =>
					tf_rd <= '0';
					if (tx_clk='1') then
						tsr(10) <= '1';
						tsr(9 downto 0) <= tsr(10 downto 1);
						i := i+1;
						if (i=12) or (i=11 and parity_mode=PARITY_NONE) then -- two stop bits
							uart_tx_state <= s0;
						end if;
						
					end if;
					
			end case;
			
		end if;

	end process;

	txd <= tsr(0);
	tdre <= not tf_full;

--
--	receive fifo
--
	rf: fifo generic map (8, rxf_depth, rxf_thres)
		port map (clk, reset, rsr(8 downto 1), ua_dout, ua_rd, rf_wr, rf_empty, rf_full, rf_half);

	rdrf <= not rf_empty;
	nrts <= rf_half;			-- glitches even on empty fifo!

--
--	filter rxd
--
	with rx_buf select
		rx_d <=	'0' when "000",
		'0' when "001",
		'0' when "010",
		'1' when "011",
		'0' when "100",
		'1' when "101",
		'1' when "110",
		'1' when "111",
		'X' when others;

--
--	state machine for actual shift in
--
	process(clk, reset)

		variable i : integer range 0 to 11;
		variable parity_rx : std_logic;

	begin

		if (reset='1') then
			uart_rx_state <= s0;
			rsr <= "00000000000";
			rf_wr <= '0';
			rx_clk_ena <= '0';
			parity_error <= '0';

		elsif rising_edge(clk) then

			case uart_rx_state is


				when s0 =>
					i := 0;
					rf_wr <= '0';
					if (rx_d='0') then
						rx_clk_ena <= '1';
						uart_rx_state <= s1;
					else
						rx_clk_ena <= '0';
					end if;

				when s1 =>
					if (rx_clk='1') then
						rsr(10) <= rx_d;
						rsr(9 downto 0) <= rsr(10 downto 1);
						i := i+1;

						if i=11 then
							uart_rx_state <= s2;
						end if;

						if i=10 and parity_mode=PARITY_NONE then
							rsr(10) <= rx_d;
							rsr(9) <= rx_d;
							rsr(8 downto 0) <= rsr(10 downto 2);
							uart_rx_state <= s2;
						end if;
					end if;
					
				when s2 =>
					rx_clk_ena <= '0';
					
					if rsr(0)='0' and rsr(10)='1' then
						
						parity_rx := ( (rsr(8) xor rsr(7)) xor (rsr(6) xor rsr(5)) ) xor ( (rsr(4) xor rsr(3)) xor (rsr(2) xor rsr(1)) );

						if (rsr(9) = parity_rx) then -- ok for even parity
							parity_error <= '0';
							if (parity_mode = PARITY_ODD) then  -- odd parity
								parity_error <= '1';								
							end if;
						else -- ok for odd parity
							parity_error <= '0';
							if (parity_mode = PARITY_EVEN) then  -- even parity
								parity_error <= '1';
							end if;
						end if;
						
						if rf_full='0' then				-- if full just drop it
							rf_wr <= '1';
						end if;
					end if;
					
					uart_rx_state <= s0;

			end case;
		end if;

	end process;

end rtl;

--
--  Copyright 2011 Martin Schoeberl <masca@imm.dtu.dk>,
--                 Technical University of Denmark, DTU Informatics. 
--  All rights reserved.
--
--  License: TBD, BSD style requested, decision pending.
--
--	leroscyc12.vhd
--
--	top level for cycore board with EP1C12
--
--	2011-02-20	creation
--
--


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.patmos_types.all;
-- use work.sc_pack.all;


entity leros_top is

generic (
	ram_cnt		: integer := 2		-- clock cycles for external ram
);

port (
	clk		: in std_logic;
--
--	serial interface
--
	ser_txd			: out std_logic;
	ser_rxd			: in std_logic;
	ser_ncts		: in std_logic;
	ser_nrts		: out std_logic;

--
--	watchdog
--
	wd		: out std_logic;
	freeio	: out std_logic;

--
--	two ram banks
--
-- 	rama_a		: out std_logic_vector(17 downto 0);
-- 	rama_d		: inout std_logic_vector(15 downto 0);
-- 	rama_ncs	: out std_logic;
-- 	rama_noe	: out std_logic;
-- 	rama_nlb	: out std_logic;
-- 	rama_nub	: out std_logic;
-- 	rama_nwe	: out std_logic;
-- 	ramb_a		: out std_logic_vector(17 downto 0);
-- 	ramb_d		: inout std_logic_vector(15 downto 0);
-- 	ramb_ncs	: out std_logic;
-- 	ramb_noe	: out std_logic;
-- 	ramb_nlb	: out std_logic;
-- 	ramb_nub	: out std_logic;
-- 	ramb_nwe	: out std_logic;

--
--	I/O pins of board
--
	io_b	: inout std_logic_vector(10 downto 1);
	io_l	: inout std_logic_vector(20 downto 1);
	io_r	: inout std_logic_vector(20 downto 1);
	io_t	: inout std_logic_vector(6 downto 1)
);
end leros_top;

architecture rtl of leros_top is


	signal clk_int, clk2x_int : std_logic;

	-- for generation of internal reset
	signal int_res			: std_logic;
	signal res_cnt			: unsigned(2 downto 0) := "000";	-- for the simulation

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

	signal wd_out			: std_logic;
	
	signal ioout : io_out_type;
	signal ioin : io_in_type;

	signal outp 			: std_logic_vector(15 downto 0);
	
begin

	-- let's go for 200 MHz ;-)
	-- but for now 100 MHz is enough
	pll_inst : entity work.pll generic map(
		multiply_by => 5,
		divide_by => 1
	)
	port map (
		inclk0	 => clk,
		c0	 => clk_int,
		c1 => clk2x_int,
		locked => open
	);

--
--	internal reset generation
--	should include the PLL lock signal
--

process(clk_int)
begin
	if rising_edge(clk_int) then
		if (res_cnt/="111") then
			res_cnt <= res_cnt+1;
		end if;

		int_res <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
	end if;
end process;

	wd <= wd_out;

	cpu: entity work.patmos
		port map(clk_int, clk2x_int, int_res, ioout, ioin);

	ioin.rddata <= (others => '0');
			
process(clk_int)
begin

	if rising_edge(clk_int) then
		if ioout.wr='1' then
			outp <= ioout.wrdata;
		end if;
		wd_out <= outp(0);
	end if;
end process;

end rtl;

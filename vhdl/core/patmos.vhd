--
--  Copyright 2011 Martin Schoeberl <masca@imm.dtu.dk>,
--                 Technical University of Denmark, DTU Informatics. 
--  All rights reserved.
--
--  License: TBD, BSD style requested, decision pending.
--
-- top level of the Leros CPU
-- That should be instanziated in a FPGA specific top level


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.patmos_types.all;

entity patmos is
	port  (
		clk : in std_logic;
		clk2x : in std_logic;
		reset : in std_logic;
		ioout : out io_out_type;
		ioin : in io_in_type
	);
end patmos;

architecture rtl of patmos is

	signal fdin : fedec_in_type;
	signal fdout : fedec_out_type;
	
	signal exout : ex_out_type;


begin

	fdin.accu <= exout.accu;
	ioout.addr <= fdout.imm(7 downto 0);
	ioout.rd <= fdout.dec.inp;
	ioout.wr <= fdout.dec.outp;
	ioout.wrdata <= exout.accu;
	
	
-- 	fd: entity work.leros_fedec port map (
-- 		clk, reset, fdin, fdout
-- 	);
-- 	ex: entity work.leros_ex port map(
-- 		clk, reset, fdout, ioin, exout
-- 	);
	
end rtl;
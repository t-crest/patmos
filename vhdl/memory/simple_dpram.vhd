-- 
-- Copyright Technical University of Denmark. All rights reserved.
-- This file is part of the time-predictable VLIW Patmos.
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
-- Simple dual port ram with read and write port.
--
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------

--
--	dpram.vhd
--
--	Simple dual port ram with read and write port
--	Read and write address, write data is registered. Output is not
--	registered.
--
--	Read during write at the same address is old value according to VHDL
--  code. Might be undefined on some FPGAs.
--  When forwarding is set to true it is implemented in LC on a Cyclone-II.
-- 
-- The 0 initialization only works at simulation.
-- Xilinx and Altera FPGAs zero the memory by default,
-- Actel FPGAs and ASICs not.
--
--
--	Author: Martin Schoeberl (martin@jopdesign.com)
--
--	2006-08-03	adapted from simulation only version
--	2008-03-02	added read enable
--  2010-08-13	simplified for Patmos
--	2012-07-17	add forwarding possibility
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dpram is
generic (width : integer := 32; addr_width : integer := 7; forwarding : boolean := false);
port (
	clk			: in std_logic;
	
	wraddress	: in std_logic_vector(addr_width-1 downto 0);
	din		: in std_logic_vector(width-1 downto 0);
	wren		: in std_logic;

	rdaddress	: in std_logic_vector(addr_width-1 downto 0);
	dout		: out std_logic_vector(width-1 downto 0)
);
end dpram ;

architecture rtl of dpram is

	subtype word is std_logic_vector(width-1 downto 0);
	constant nwords : integer := 2 ** addr_width;
	type ram_type is array(0 to nwords-1) of word;

-- The 0 initialization works only in simulation and some FPGAs
--	signal ram : ram_type := (others => (others => '0'));
	signal ram : ram_type;

begin

process (clk)
begin
	if rising_edge(clk) then
		if wren='1' then
			ram(to_integer(unsigned(wraddress))) <= din;
		end if;
		-- does not really work on an FPGA
		if forwarding=true then
			if rdaddress = wraddress then
				dout <= din;
			else
				dout <= ram(to_integer(unsigned(rdaddress)));
			end if;
		else
			dout <= ram(to_integer(unsigned(rdaddress)));
		end if;
	end if;
end process;

end rtl;

-- 
-- Copyright 2010 Martin Schoeberl, martin@jopdesign.com. All rights reserved.
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
-- Entity: fmax
--------------------------------------------------------------------------------
-- Copyright ... 2010
-- Filename          : fmax.vhd
-- Creation date     : 2010-08-15
-- Author(s)         : martin
-- Version           : 1.00
-- Description       : <short description>
--------------------------------------------------------------------------------
-- File History:
-- Date         Version  Author   Comment
-- 2010-08-15   1.00     martin     Creation of File
--------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.patmos_types.all;

entity fmax is
	port  (
		clk : in std_logic;
		din : in std_logic;
		out_sel : in std_logic;
		dout : out std_logic
	);
end fmax;

architecture arch of fmax is

	constant IN_LENGTH : integer := 100;
	constant OUT_LENGTH : integer := 100;

	signal in_val : std_logic_vector(IN_LENGTH-1 downto 0);
	signal out_sel_shf : std_logic_vector(2 downto 0);
	signal out_val, out_reg : std_logic_vector(OUT_LENGTH-1 downto 0);
	signal out_sig : std_logic;
	
begin

process(clk)
begin
	if rising_edge(clk) then
		in_val(0) <= din;
		dout <= out_val(OUT_LENGTH-1);
		in_val(IN_LENGTH-1 downto 1) <= in_val(IN_LENGTH-2 downto 0);
		out_sel_shf(0) <= out_sel;
		out_sel_shf(2 downto 1) <= out_sel_shf(1 downto 0);
		out_reg(OUT_LENGTH-1 downto 1) <= in_val(OUT_LENGTH-1 downto 1);
		out_reg(0) <= '0';
		if out_sel_shf(2)='1' then
			out_val <= out_reg;
		else
			out_val(OUT_LENGTH-1 downto 1) <= out_val(IN_LENGTH-2 downto 0);
		end if;
	end if;
end process;

end arch;


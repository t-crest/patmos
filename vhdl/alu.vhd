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
-- Entity: alu
--------------------------------------------------------------------------------
-- Copyright ... 2010
-- Filename          : alu.vhd
-- Creation date     : 2010-08-10
-- Author(s)         : martin
-- Version           : 1.00
-- Description       : <short description>
--------------------------------------------------------------------------------
-- File History:
-- Date         Version  Author   Comment
-- 2010-08-10   1.00     martin     Creation of File
--------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.patmos_types.all;

entity alu is
	port  (
		clk : in std_logic;
		reset : in std_logic;
		alu_in : in alu_in_type;
		alu_out : out alu_out_type
	);
end alu;

architecture rtl of alu is

	signal result, result_reg : alu_out_type;
	

begin

process(alu_in)
begin
	if alu_in.add='1' then
		result.result <= std_logic_vector(signed(alu_in.opa) + signed(alu_in.opb));
	else
		result.result <= std_logic_vector(signed(alu_in.opa) - signed(alu_in.opb));
	end if;		
end process;


process(clk, reset)
begin
	if reset='1' then
		result_reg.result <= (others => '0');
	elsif rising_edge(clk) then
		result_reg <= result;
	end if;
end process;

end rtl;


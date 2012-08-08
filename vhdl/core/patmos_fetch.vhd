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
-- Instruction fetch and PC manipulation.
--
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- Author: Sahar Abbaspour
--------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_fetch is
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;
		decout     : in  decode_out_type;
		exout      : in  execution_out_type;
		reg1, reg2 : out std_logic_vector(4 downto 0);
		dout       : out fetch_out_type
	);
end entity patmos_fetch;

architecture arch of patmos_fetch is
	signal pc, pc_next : std_logic_vector(pc_length - 1 downto 0);
	signal addr        : std_logic_vector(pc_length - 1 downto 0);
	signal feout       : fetch_out_type;
	signal tmp         : std_logic_vector(31 downto 0);

begin
	process(pc, decout, exout)
	begin
		-- this is effective branch in the EX stage with
		-- two branch delay slots
		if decout.inst_type_out = BC and exout.predicate(to_integer(unsigned(decout.predicate_condition))) = '1' then -- decout.predicate_bit_out then
			-- no addition? no relative branch???
			pc_next <= decout.imm;
		else
			pc_next <= std_logic_vector(unsigned(pc) + 1);
		end if;
	end process;

	rom : entity work.patmos_rom
		port map(
			address => addr(7 downto 0),
			-- instruction shall not be unsigned
			q       => tmp
		);

	feout.instruction <= tmp;
	-- register addresses unregistered to make the register file code easier
	reg1 <= tmp(16 downto 12);
	reg2 <= tmp(11 downto 7);
	process(clk, rst)
	begin
		if (rst = '1') then
			pc               <= (others => '0');
			dout.pc          <= (others => '0');
			dout.instruction <= (others => '0');
		elsif (rising_edge(clk) and rst = '0') then
			pc               <= pc_next;
			addr             <= pc_next;
			dout.instruction <= feout.instruction;
			-- MS: the next pc? PC calculation is REALLY an independent pipe stage!
			dout.pc <= pc;
		end if;
	end process;

end arch;

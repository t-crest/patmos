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
	-- we should have global constants for memory sizes
	signal pc, pc_next                               : std_logic_vector(pc_length - 1 downto 0);
	signal pc_add	: unsigned(1 downto 0);
	signal evn_next, addr_evn, addr_odd              : std_logic_vector(7 downto 0);
	signal feout                                     : fetch_out_type;
	signal data_evn, data_odd, instr_a, instr_b, tmp : std_logic_vector(31 downto 0);

begin
	process(pc, instr_a, pc_add, decout, exout)
	begin
		if instr_a(31) = '1' then
			pc_add <= "10";
		else
			pc_add <= "01";
		end if;
		pc_next <= std_logic_vector(unsigned(pc) + pc_add);
		
		feout.b_valid <= instr_a(31);
		
		-- this is effective branch in the EX stage with
		-- two branch delay slots
		if decout.inst_type_out = BC and exout.predicate(to_integer(unsigned(decout.predicate_condition))) = '1' then -- decout.predicate_bit_out then
			-- no addition? no relative branch???
			pc_next <= decout.imm;
		end if;
	end process;

	-- Even RAM address needs an increment when PC is odd
	process(pc_next)
	begin
		evn_next <= pc_next(7 downto 1) & "0";
		if pc_next(0) = '1' then
			evn_next <= std_logic_vector(unsigned(pc_next(7 downto 1)) + 1) & "0";
		end if;
	end process;

	-- Reusing a single ROM with constant '0' and '1' at address(0)
	-- Assuming that synthsize will optimize it
	rom_evn : entity work.patmos_rom
		port map(
			address => addr_evn,
			-- instruction shall not be unsigned
			q       => data_evn
		);

	rom_odd : entity work.patmos_rom
		port map(
			address => addr_odd,
			-- instruction shall not be unsigned
			q       => data_odd
		);

	-- MUX the two outputs data
	process(pc, data_evn, data_odd)
	begin
		if (pc(0) = '0') then
			instr_a <= data_evn;
			instr_b <= data_odd;
		else
			instr_a <= data_odd;
			instr_b <= data_evn;
		end if;
	end process;

	feout.instruction <= instr_a;
	feout.instr_b     <= instr_b;
	feout.pc          <= pc;
	-- register addresses unregistered to make the register file code easier
	-- shall be a type of unregistered (or combinational) output
	reg1 <= instr_a(16 downto 12);
	reg2 <= instr_a(11 downto 7);
	process(clk, rst)
	begin
		if (rst = '1') then
			-- Let's start with -1, so the first instruction at 0 gets executed
			pc               <= (others => '1');
			dout.pc          <= (others => '0');
			dout.instruction <= (others => '0');
		elsif (rising_edge(clk) and rst = '0') then
			pc       <= pc_next;
			addr_evn <= evn_next;
			addr_odd <= pc_next(7 downto 1) & "1";
			-- MS: the next pc? PC calculation is REALLY an independent pipe stage!
			dout <= feout;
		end if;
	end process;

end arch;

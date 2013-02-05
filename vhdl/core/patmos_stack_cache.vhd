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
-- Short descripton.
--
-- Author: Sahar Abbaspour
--------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;
use work.sc_pack.all;

entity patmos_stack_cache is
  port
  (
    	clk       	         				: in std_logic;
    	rst									: in std_logic;
       	cpu_out								: in cpu_out_type;
		cpu_in								: out sc_in_type;

		mem_out								: out sc_out_type;
		mem_in								: in sc_in_type
       	
  );    
end entity patmos_stack_cache;
architecture arch of patmos_stack_cache is
	
	signal sc_write_data					: std_logic_vector(31 downto 0);		
	signal sc_write_add						: std_logic_vector(sc_length - 1 downto 0);
	signal sc_read_data						: std_logic_vector(31 downto 0);
	signal sc_read_add						: std_logic_vector(sc_length - 1 downto 0);
	signal sc_en_fill						: std_logic_vector(3 downto 0);
	signal exout_reg_adr_shft				: std_logic_vector(31 downto 0);
	
begin

	sc0: entity work.patmos_data_memory(arch)
		generic map(8, sc_length)
		port map(clk,
			     sc_write_add,
			     sc_write_data(7 downto 0),
			     sc_en_fill(0),
			     sc_read_add,
			     sc_read_data(7 downto 0));
 
	sc1: entity work.patmos_data_memory(arch)
		generic map(8, sc_length)
		port map(clk,
			     sc_write_add,
			     sc_write_data(15 downto 8),
			     sc_en_fill(1),
			     sc_read_add,
			     sc_read_data(15 downto 8));
			     
	sc2: entity work.patmos_data_memory(arch)
		generic map(8, sc_length)
		port map(clk,
			     sc_write_add,
			     sc_write_data(23 downto 16),
			     sc_en_fill(2),
			     sc_read_add,
			     sc_read_data(23 downto 16));
			     
	sc3: entity work.patmos_data_memory(arch)
		generic map(8, sc_length)
		port map(clk,
			     sc_write_add,
			     sc_write_data(31 downto 24),
			     sc_en_fill(3),
			     sc_read_add,
			     sc_read_data(31 downto 24));	
			     
	-------------------------------------------------------------------------		 
	
	process(cpu_out.address) -- shift the address. . . 
	begin
		exout_reg_adr_shft <= "00" & cpu_out.address(31 downto 2); 
	end process;    
	
  	process(cpu_out,
		sc_read_data, exout_reg_adr_shft
	) --SA: Main memory read/write address, normal load/store or fill/spill
	begin
		--cpu_in.rd_add <= exout_reg_adr_shft(9 downto 0);
		--cpu_in.wr_add <= exout_reg_adr_shft(9 downto 0);
		--gm_en_spill <= gm_en;
		--cpu_in.wr_data <= mem_write_data_stall;
		cpu_in.rd_data 	<= sc_read_data;
		sc_read_add 	<= exout_reg_adr_shft(sc_length - 1 downto 0);
		sc_write_add 	<= exout_reg_adr_shft(sc_length - 1 downto 0);
		sc_en_fill 		<= cpu_out.sc_en;
		sc_write_data 	<= cpu_out.wr_data;
--		if (spill = '1' or fill = '1') then	
--			cpu_in.rd_add <= mem_top(9 downto 0);
--			sc_read_add <= mem_top(sc_length - 1 downto 0) and SC_MASK;
--			gm_en_spill <= gm_spill; -- this is for spilling ( writing to main memory)
--			cpu_in.rd_data <= sc_read_data;
--			sc_en_fill <= sc_fill; -- this is for filling!
--			sc_write_data <= cpu_out.wr_data;
--			--sc_write_add <= ; -- spill
--		end if;
	end process;
     
end arch;



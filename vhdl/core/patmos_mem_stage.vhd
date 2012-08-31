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


entity patmos_mem_stage is 
  port
  (
    clk                          : in std_logic;
    rst       	 	                : in std_logic;
    din             	            : in mem_in_type;
    dout                         : out mem_out_type 
  );
end entity patmos_mem_stage;


architecture arch of patmos_mem_stage is
		
	signal en0, en1, en2, en3	: std_logic;
	signal dout0, dout1, dout2, dout3 : std_logic_vector(7 downto 0);
	signal mem_write_data0, mem_write_data1, mem_write_data2, mem_write_data3 :std_logic_vector(7 downto 0);
begin


  
  mem_wb: process(clk)
  begin
    if (rising_edge(clk)) then
      dout.data_out <= din.data_in;
      -- forwarding
      dout.reg_write_out <= din.reg_write_in;
      dout.write_back_reg_out <= din.write_back_reg_in;
      dout.mem_write_data_out <= din.mem_write_data_in;
    end if;
  end process mem_wb;

--	memory: entity work.patmos_data_memory(arch)
--	port map(clk, din.alu_result,
--	din.mem_write_data_in, din.mem_write, din.alu_result, dout.data_mem_data_out);
	
	memory0: entity work.patmos_data_memory(arch)
	generic map(8, 10)
	port map(clk, din.alu_result(9 downto 0),
	mem_write_data0, en0, din.alu_result(9 downto 0), dout0);
	
	memory1: entity work.patmos_data_memory(arch)
	generic map(8, 10)
	port map(clk, din.alu_result(9 downto 0),
	mem_write_data1, en1, din.alu_result(9 downto 0), dout1);
	
	memory2: entity work.patmos_data_memory(arch)
	generic map(8, 10)
	port map(clk, din.alu_result(9 downto 0),
	mem_write_data2, en2, din.alu_result(9 downto 0), dout2);
	
	memory3: entity work.patmos_data_memory(arch)
	generic map(8, 10)
	port map(clk, din.alu_result(9 downto 0),
	mem_write_data3, en3, din.alu_result(9 downto 0), dout3);
	
	
	
	
	ld_type: process(dout0, dout1, dout2, dout3)
	begin
		case din.LDT_instruction_type_out is
			when LWL=> 
				dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;
			when LWC =>
				dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;
			when LWM => 	
				dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;
				
			when LHL=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed( dout0 & dout1), 32));
			when LHC=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed( dout0 & dout1), 32));
			when LHM=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed( dout0 & dout1), 32));
				
			when LBL=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed(dout0), 32));
			when LBM=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed(dout0), 32));
			when LBC=>
				dout.data_mem_data_out <= std_logic_vector(resize(signed(dout0), 32));	
				
			when LHUL=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned( dout0 & dout1), 32));
			when LHUC=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned( dout0 & dout1), 32));
			when LHUM=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned( dout0 & dout1), 32));
					
			when LBUL=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout0), 32));
			when LBUC=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout0), 32));
			when LBUM=>
				dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout0), 32));
				
			when others => 
			 	dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;
		end case;
	end process;
	
	
	st_type: process(din)
	begin
		case din.STT_instruction_type_out is 
			when SWL =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(31 downto 24);
				mem_write_data1 <= din.mem_write_data_in(23 downto 16);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when SHL =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(15 downto 8);
				mem_write_data1 <= din.mem_write_data_in(7 downto 0);
				mem_write_data2 <= (others => '0');
				mem_write_data3 <= (others => '0');
		    when SBL =>
		    	en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(7 downto 0);
			when SWM =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(31 downto 24);
				mem_write_data1 <= din.mem_write_data_in(23 downto 16);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when SHM =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(15 downto 8);
				mem_write_data1 <= din.mem_write_data_in(7 downto 0);
				mem_write_data2 <= (others => '0');
				mem_write_data3 <= (others => '0');
		    when SBM =>
		    	en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(7 downto 0);
				mem_write_data1 <= (others => '0');
				mem_write_data2 <= (others => '0');
				mem_write_data3 <= (others => '0');						
			when SWC =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(31 downto 24);
				mem_write_data1 <= din.mem_write_data_in(23 downto 16);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when SHC =>
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(15 downto 8);
				mem_write_data1 <= din.mem_write_data_in(7 downto 0);
				mem_write_data2 <= (others => '0');
				mem_write_data3 <= (others => '0');
		    when SBC =>
		    	en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(7 downto 0);
				mem_write_data1 <= (others => '0');
				mem_write_data2 <= (others => '0');
				mem_write_data3 <= (others => '0');			
		    when others => 
				en0 <= din.mem_write; 
				en1 <= din.mem_write;
				en2 <= din.mem_write;
				en3 <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(31 downto 24);
				mem_write_data1 <= din.mem_write_data_in(23 downto 16);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
		end case;
	end process st_type;
	
	
	



end arch;
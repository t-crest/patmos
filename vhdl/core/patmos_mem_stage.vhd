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
	signal memory_din		: std_logic_vector(31 downto 0);
	signal memory_dout		: std_logic_vector(31 downto 0);
begin

-- MS: this should be merged with memory stage
  
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

	memory: entity work.patmos_data_memory(arch)
	port map(clk, din.alu_result,
	din.data_in, din.mem_write, din.alu_result, dout.data_mem_data_out);
	
--	st_type: process(din)
--	begin
--		case din.STT_instruction_type_out is 
--			when SWL =>
--				memory_din <= din.data_in;
--			when SHL =>
--				memory_din <= din.data_in;--(16 downto 0);
--		    when SBL =>
--		    	memory_din <= din.data_in;--(16 downto 0);
--		end case;
--	end process st_type;

end arch;
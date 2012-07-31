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


--TO DO: 
-- replcae pd with predicate_reg(pd) (number of predicate register that should be written with 0 or 1)


 -----------------------------------
 -- execution
 -----------------------------------

library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use work.patmos_type_package.all;


entity patmos_execute is
  port
  (
    clk                             : in std_logic;
    rst                             : in std_logic;
    din                       	     : in execution_in_type;
    dout                            : out execution_out_type
  );

end entity patmos_execute;

architecture arch of patmos_execute is
--signal test : std_logic := '0';
begin

  process(clk)
  begin
    if rising_edge(clk) then
      if (din.predicate_data_in(to_integer(din.predicate_condition)) /= din.predicate_bit_in ) then
      	dout.mem_read_out <= din.mem_read_in;
      	dout.mem_write_out <= din.mem_write_in;
      	dout.reg_write_out <= din.reg_write_in;
      	dout.ps_reg_write_out <= din.ps_reg_write_in;
      --	test <= '1';
      else
      	dout.mem_read_out <= '0';
      	dout.mem_write_out <= '0';
     	dout.reg_write_out <= '0';
     	dout.ps_reg_write_out <= '0';
      end if;
      
      dout.mem_to_reg_out <= din.mem_to_reg_in;
      dout.alu_result_out <= din.alu_result_in;
      dout.mem_write_data_out <= din.mem_write_data_in;
      dout.write_back_reg_out <= din.write_back_reg_in;
      dout.ps_write_back_reg_out <= din.ps_write_back_reg_in;
      dout.head_out <= din.head_in;
      dout.tail_out <= din.tail_in;
      dout.STT_instruction_type_out <= din.STT_instruction_type_in;
      dout.LDT_instruction_type_out <= din.LDT_instruction_type_in;
      dout.alu_result_predicate_out <= din.alu_result_predicate_in;
    end if; 
  end process;
end arch;




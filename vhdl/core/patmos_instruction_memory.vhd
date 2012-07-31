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

entity patmos_instruction_memory is
  port(
  		clk					  : in std_logic;
  	 	rst                   : in std_logic;
        address               : in unsigned(31 downto 0);
        inst_in               : in unsigned(31 downto 0); -- store
        inst_out              : out unsigned(31 downto 0); -- load
        read_enable           : in std_logic;
        write_enable          : in std_logic
  );
end entity patmos_instruction_memory;


architecture arch of patmos_instruction_memory is


  type instruction_memory is array (0 to 1024) of unsigned(31 downto 0);
  signal inst_mem : instruction_memory;

begin
  read: process(address, read_enable)
  begin
  	--if (read_enable = '1') then
  		inst_out <= inst_mem(to_integer(unsigned(address)));
  --	end if;
  end process;
  
  mem : process(clk, rst)
  begin
    if(rst = '1') then   
    inst_mem(7) <= "00000000000000100000000000000010";
    inst_mem(8) <= "00000010100101000101000110000000";
    inst_mem(9) <= "00000000000000000000000000000001";
    inst_mem(10) <= "00000010000101101010000010000111";
    inst_mem(11) <= "00000111110000001011000010000100";
    inst_mem(12) <= "00000000000000000000000000000001";
    inst_mem(13) <= "00000010100111100101000110000001";
    inst_mem(14) <= "00000010100111100101000110000001";
    inst_mem(15) <= "00000000000001110011000000000001";
    inst_mem(16) <= "00000000000001000000000001010000";
    inst_mem(17) <= "00000010100101000101000110000000";
    inst_mem(18) <= "00000000000000000000000000000001";
    inst_mem(19) <= "00000010000101100011010100000111";
    inst_mem(20) <= "00000111110000001011000110000100";
    inst_mem(21) <= "00000000000000000000000000000001";
    inst_mem(22) <= "00000010110001100111011110000001";
    inst_mem(23) <= "00000000000000000000000000000001";
    inst_mem(24) <= "00000000000001100000000000000010";
    inst_mem(25) <= "00000111110000000000001100000010";
    inst_mem(26) <= "00000000000000000000000000000001";
    elsif (rising_edge(clk)) then
      if(write_enable = '1') then
        inst_mem(to_integer(unsigned(address))) <= inst_in;
      end if;
    end if;
  end process mem;

end arch;  

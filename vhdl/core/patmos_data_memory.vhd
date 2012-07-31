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

entity patmos_data_memory is
  port(
        clk       	           : in std_logic;
        rst                   : in std_logic;
        address               : in unsigned(31 downto 0);
        data_in               : in unsigned(31 downto 0); -- store
        data_out              : out unsigned(31 downto 0); -- load
        read_enable           : in std_logic;
        write_enable          : in std_logic
      );
end entity patmos_data_memory;

architecture arch of patmos_data_memory is
  type data_memory is array (0 to 255) of unsigned(31 downto 0);
  signal data_mem : data_memory;

begin
 -- data_out <= (data_mem(to_integer(unsigned(address)) + 3) & 
 --              data_mem(to_integer(unsigned(address)) + 2) &
 --              data_mem(to_integer(unsigned(address)) + 1) &
 --              data_mem(to_integer(unsigned(address)))) when read_enable = '1';
  data_out <= data_mem(to_integer(unsigned(address)));
  mem : process(clk, rst)
  begin
   -- if(rst = '1') then
    --    for i in 0 to 255 loop -- initialize register file
      --    data_mem(i)<= (others => '0');
       -- end loop;
    --els
  if (rising_edge(clk)) then
      if(write_enable = '1') then
       -- data_mem(to_integer(unsigned(address)) + 3) <= data_in(31 downto 24);
       -- data_mem(to_integer(unsigned(address)) + 2) <= data_in(23 downto 16);
       -- data_mem(to_integer(unsigned(address)) + 1) <= data_in(15 downto 8);
       -- data_mem(to_integer(unsigned(address))) <=     data_in(7 downto 0);
        data_mem(to_integer(unsigned(address))) <= data_in;
      end if;
    end if;
  end process mem;
end arch;

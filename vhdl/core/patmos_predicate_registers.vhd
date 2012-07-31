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


------------------------------------------
-- predicate registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_predicate_registers is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
    read_address  : in unsigned(2 downto 0);
    write_address : in unsigned(2 downto 0);
    read_data     : out std_logic;
    write_data    : in std_logic;
    write_enable  : in std_logic
  );
end entity patmos_predicate_registers;

architecture arch of patmos_predicate_registers is
--signal register_bank : std_logic_vector(7 downto 0);

signal reg_read_address : unsigned(2 downto 0);
begin
  --                                  
  ------ latch read address
  latch_read_address:  process (clk, rst)
  begin
    if(rst = '1') then
        -- initialize register file
          predicate_register_bank <= (others => '0');
    elsif rising_edge(clk) then
   --   if (read_enable) then ?? need read enable?!!
          reg_read_address <= read_address;
   --   end if;
    end if;
   end process latch_read_address;
   
 ------ read process (or should be async?)
  read:  process (reg_read_address)
  begin
    read_data <= predicate_register_bank(to_integer(unsigned(reg_read_address)));
  end process read;
  
 ------ write process
  write:  process (clk)
   begin
   if rising_edge(clk) then
     if (write_enable = '1') then
       predicate_register_bank(to_integer(unsigned(write_address))) <= write_data;
     end if;
   end if;
    end process write;
end arch;

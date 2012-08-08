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
-- A parameterized, inferable, true dual-port, dual-clock block RAM in VHDL.
--
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------



library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity patmos_dual_port_ram is
generic (
    DATA    : integer := 72;
    ADDR    : integer := 10
);
port (
    -- Port A
    a_clk   : in  std_logic;
    a_wr    : in  std_logic;
    a_addr  : in  std_logic_vector(ADDR-1 downto 0);
    a_din   : in  std_logic_vector(DATA-1 downto 0);
    a_dout  : out std_logic_vector(DATA-1 downto 0);
    
    -- Port B
    b_clk   : in  std_logic;
    b_wr    : in  std_logic;
    b_addr  : in  std_logic_vector(ADDR-1 downto 0);
    b_din   : in  std_logic_vector(DATA-1 downto 0);
    b_dout  : out std_logic_vector(DATA-1 downto 0)
);
end patmos_dual_port_ram;

architecture rtl of patmos_dual_port_ram is
    -- Shared memory
    type mem_type is array (0 to (2**ADDR)-1 ) of std_logic_vector(DATA-1 downto 0);
    signal mem : mem_type;
begin

-- Port A
process(a_clk)
begin
    if(rising_edge(a_clk)) then
     --   if(a_wr='1') then
     --       mem(to_integer(a_addr)) <= a_din;
     --   end if;
        a_dout <= mem(to_integer(unsigned(a_addr)));
    end if;
end process;

-- Port B
process(b_clk)
begin
    if(rising_edge(b_clk)) then
        if(b_wr='1') then
            mem(to_integer(unsigned(b_addr))) <= b_din;
        end if;
        b_dout <= mem(to_integer(unsigned(b_addr)));
    end if;
end process;

end rtl;



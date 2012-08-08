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

entity patmos_stack_cache is
  port
  (
    	clk       	         		: in std_logic;
    	rst							: in std_logic;
       	din							: in patmos_stack_cache_in;
       	dout						: out patmos_stack_cache_out
  );    
end entity patmos_stack_cache;
architecture arch of patmos_stack_cache is

component patmos_dual_port_ram is
generic (
    DATA    : integer := 32;
    ADDR    : integer := 5
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

  end component;
  
begin

	stack_cache_ram : patmos_dual_port_ram port map    
	(
	-- Port A, mem
		    a_clk  => clk,
    		a_wr   => din.spill_fill,
   			a_addr => din.head_tail,
   			a_din  => din.din_from_mem,
   			a_dout => dout.dout_to_mem,
    
    -- Port B, CPU
    		b_clk  => clk,
    		b_wr   => din.write_enable,
    		b_addr => din.address,
    		b_din  => din.din_from_cpu,
    		b_dout => dout.dout_to_cpu
	);
	
  
     
end arch;



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

entity patmos_testbench is 
end entity patmos_testbench;

architecture timed of patmos_testbench is 
  signal clk                   :  std_logic := '0';
  signal rst                   :  std_logic := '0';
 -- signal fetch_din             :  fetch_in_type;
  signal    led         	  :  std_logic;
  signal  txd      	          :  std_logic;
  signal  rxd     		:   std_logic;
  signal  oSRAM_A		 :  std_logic_vector(18 downto 0);		-- edit
signal	SRAM_DQ		 :  std_logic_vector(31 downto 0);		-- edit
signal	oSRAM_CE1_N	 :  std_logic;
signal	oSRAM_OE_N	 :  std_logic;
signal	oSRAM_BE_N	 :  std_logic_vector(3 downto 0);
signal	oSRAM_WE_N	 :  std_logic;
signal	oSRAM_GW_N   :  std_logic;
signal	oSRAM_CLK	 :  std_logic;
signal	oSRAM_ADSC_N :  std_logic;
signal	oSRAM_ADSP_N :  std_logic;
signal	oSRAM_ADV_N	 :  std_logic;
signal	oSRAM_CE2	 :  std_logic;
signal	oSRAM_CE3_N  :  std_logic;
signal internal_rst  :std_logic;
begin

	
  core: entity work.patmos_core(arch)
  port map(clk, internal_rst, led, txd, rxd);
--  , oSRAM_A, SRAM_DQ, oSRAM_CE1_N
--  	, oSRAM_OE_N, oSRAM_BE_N, oSRAM_WE_N, oSRAM_GW_N, oSRAM_CLK,
--  	oSRAM_ADSC_N, oSRAM_ADSP_N, oSRAM_ADV_N, oSRAM_CE2, oSRAM_CE3_N
--  );  
	
	
  clk <= not clk after 5 ns;
              --      "xpred00fff4321043210109876543210"
--  fetch_din.instruction <= -- after 5 ns,  -- 
 --                     "00000000000001000000000000000011" after 5 ns, -- r2 <= r0 + 3 add immediate
--                      "00000000000001100010000000000010" after 15 ns, -- r3 <= r2 + 2 add immediate -- alu forward
 --                     "00000000000000000000000010000001" after 25 ns, -- r0 <= r0 + 128 add immediate -- mem forward
 --                     "00000000000010000001000000000001" after 35 ns, -- r4 <= r1 + 1 add immediate
  --                    "00000000000010100010000000000001" after 45 ns, -- r5 <= r2 + 1 add immediate
                     -- "00000000000010100101000000000001" after 55 ns; -- r5 <= r5 + 1 add immediate
 --                     "00000010101101011000000000000000" after 55 ns;  -- load( rd:26 <= mem[ra:24 + imm:0])
                   --   "00000010000001100110011110000000" after 15 ns;--, -- r3 <= r? + r? add register -- 
	
  rst <= '1' after 11 ns;
 internal_rst <= not rst;
  
end architecture timed;

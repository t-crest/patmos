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



--------------------------------------
-- determine the forwarding type
--------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity forward_type_select is
  generic ( width : integer := 5 );    
  port
  (
    read_register                 : in unsigned(width -1 downto 0);
    alu_write_register            : in unsigned(width -1 downto 0);
    mem_write_register            : in unsigned(width -1 downto 0);
    alu_write_enable              : in std_logic;
    mem_write_enable              : in std_logic;
    fw_type                       : out forwarding_type
  );
end entity forward_type_select;

architecture arch of forward_type_select is
begin
  process (read_register, alu_write_register, mem_write_register, alu_write_enable, mem_write_enable)
  begin
   if(alu_write_register = read_register and alu_write_enable = '1' and alu_write_register /= "00000") then
      fw_type <= FWALU;
    elsif (mem_write_register = read_register and mem_write_enable = '1' and mem_write_register /= "00000") then
      fw_type <= FWMEM;
    else 
      fw_type <= FWNOP;
    end if;
  end process;
end arch;

---------------------------------
-- determine the forwarding value
---------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_forward_value is
--generic ( width : integer := 32 );
  port
  (
    fw_alu              : in unsigned(31 downto 0); --rs forwarded from previous alu
    fw_mem              : in unsigned(31 downto 0); --rs forwarded from data memory
    fw_in               : in unsigned(31 downto 0); --rs from register file
    fw_out              : out unsigned(31 downto 0);
    fw_ctrl             : in forwarding_type
  );
end entity patmos_forward_value;

architecture arch of patmos_forward_value is
begin
  
    process(fw_ctrl, fw_in, fw_alu, fw_mem)
	 begin
	   if(fw_ctrl = FWALU)  then
	 			fw_out <= fw_alu;
   	elsif (fw_ctrl = FWMEM) then
             fw_out <= fw_mem; 
		elsif (fw_ctrl = FWNOP) then
             fw_out <= fw_in;
				 else fw_out <= fw_in;
		end if;
	 end process;
end arch;

-- MS: predictaes nee no forward, this shall be deleted some time

--library ieee;
--use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
--use work.patmos_type_package.all;
--
--entity patmos_forward_value_predicate is
----generic ( width : integer := 32 );
--  port
--  (
--    fw_alu              : in std_logic; --rs forwarded from previous alu
--    fw_mem              : in std_logic; --rs forwarded from data memory
--    fw_in               : in std_logic; --rs from register file
--    fw_out              : out std_logic;
--    fw_ctrl             : in forwarding_type
--  );
--end entity patmos_forward_value_predicate;
--
--architecture arch of patmos_forward_value_predicate is
--begin
--  
--    process(fw_ctrl, fw_in, fw_alu, fw_mem)
--	 begin
--	   if(fw_ctrl = FWALU)  then
--	 			fw_out <= fw_alu;
--   	elsif (fw_ctrl = FWMEM) then
--             fw_out <= fw_mem; 
--		elsif (fw_ctrl = FWNOP) then
--             fw_out <= fw_in;
--				 else fw_out <= fw_in;
--		end if;
--	 end process;
--end arch;
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_forward is
generic ( width : integer := 5 );
  port
  (
    rs                     : in unsigned(width -1 downto 0); -- register exec reads
    rt                     : in unsigned(width -1 downto 0); -- register exec reads
    alu_we                 : in std_logic;
    mem_we                 : in std_logic;
    alu_wr_rn              : in unsigned(width -1 downto 0);
    mem_wr_rn              : in unsigned(width -1 downto 0);
    mux_fw_rs              : out forwarding_type;
    mux_fw_rt              : out forwarding_type
  );
end entity patmos_forward;

architecture arch of patmos_forward is
begin
  uut_rs: entity work.forward_type_select(arch)
  	generic map (width)
	port map(rs, alu_wr_rn, mem_wr_rn, alu_we, mem_we, mux_fw_rs);
	
	uut_rt: entity work.forward_type_select(arch)
	generic map (width)
	port map(rt, alu_wr_rn, mem_wr_rn, alu_we, mem_we, mux_fw_rt);
	  
end arch;


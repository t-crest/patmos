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



entity patmos_stack_cache_ctrl is
	port
	(
		clk,rst					: in std_logic;
		din					: in patmos_stack_cache_ctrl_in;
		dout 				: out patmos_stack_cache_ctrl_out
	);
end entity patmos_stack_cache_ctrl;

architecture arch of patmos_stack_cache_ctrl is

signal number_of_bytes_in_stack_cache			: unsigned(4 downto 0) := (others => '0');
signal head										: unsigned(4 downto 0);
signal tail										: unsigned(4 downto 0);
type state_type is (s0,s1); 
signal stack_state: state_type := s0;
signal spill_counter 							: unsigned(4 downto 0):= (others => '0'); 
signal fill_counter 							: unsigned(4 downto 0):= (others => '0');

signal spill									: std_logic;
signal st										: unsigned (31 downto 0);
begin

process(din.instruction, rst)
begin
	if(rst = '1') then
		head <= (others => '0');
	--	tail <= (others => '0');
	--	dout.reg_write_out <= '0';
		number_of_bytes_in_stack_cache <= (others => '0');
	else
	--st <= din.st_in;
	case din.instruction is -- unbounded cache
        when SRES =>
        	head <= (head + din.stc_immediate_in) mod 32;
    		if ((32 - number_of_bytes_in_stack_cache) <  din.stc_immediate_in) then
    			spill <= '1';
    	--		spill_counter <=  din.stc_immediate_in - (32 - number_of_bytes_in_stack_cache);
    		end if;
        when SENS =>
        	
        when SFREE =>
        	head <= head - din.stc_immediate_in;
        when others => null;
    end case;
    end if;
end process;

process(clk)
begin
	if (spill = '1') then
		case stack_state is
        	when s0 =>
        		dout.spill_fill <= '1';
        		dout.stall <= '1';
        		st <= st + 1;
        		tail <= tail + 1;
        		stack_state <= s1;
        		dout.head_tail <= tail + 1;
        		dout.st_out <= st + 1;
        		spill_counter <= spill_counter - 1;
        	when s1 =>
        		if (spill_counter > 0) then
        			stack_state <= s1;
        		else
        			dout.spill_fill <= '0';
        			dout.stall <= '0';
        			dout.reg_write_out <= '1';
        		end if;
        end case;
	end if;
end process;

end arch;


		

		--if (number_of_bytes_in_stack_cache = din.stc_immediate_in) then
         --dout.fill_out <= '0';
         --else
         --dout.fill_out <= '1';
         --if (din.tail_in > din.head_in) then -- not sure if this condition is correct?
         --dout.tail_out <= din.tail_in - din.stc_immediate_in;
         --end if;
         --dout.st_out <= din.st_in - ("000000000000000000000000000" & din.stc_immediate_in);
         --end if;	
         --when SFREE => --free
         --dout.head_out <= din.head_in - din.stc_immediate_in; -- not sure if this is correct?
         --dout.st_out <= din.st_in - ("000000000000000000000000000" & din.stc_immediate_in);
         --number_of_bytes_in_stack_cache <= number_of_bytes_in_stack_cache - din.stc_immediate_in;
         --dout.tail_out <= din.tail_in;

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
		clk,rst						: in std_logic;
		din							: in patmos_stack_cache_ctrl_in;
		dout 						: out patmos_stack_cache_ctrl_out
	);
end entity patmos_stack_cache_ctrl;

architecture arch of patmos_stack_cache_ctrl is

--signal number_of_bytes_in_stack_cache			: unsigned(4 downto 0) := (others => '0');
--signal head										: unsigned(4 downto 0);
--signal tail										: unsigned(4 downto 0);
--type state_type is (s0,s1); 
--signal stack_state: state_type := s0;
--signal spill_counter 							: unsigned(4 downto 0):= (others => '0'); 
--signal fill_counter 							: unsigned(4 downto 0):= (others => '0');
--
--signal spill									: std_logic;
--signal st										: unsigned (31 downto 0);

	signal num_valid_sc_slots		: integer;
--	signal state					: sc_state_type;
--	signal next_state				: sc_state_type;
	signal head, tail				: std_logic_vector(sc_depth - 1 downto 0);
	
begin

--	process(state)
--	begin
--
--	case din.instruction is
--		when reserve 	=>
--			if (std_logic_vector((unsigned(head) + unsigned(din.reserve_size)) mod sc_depth) = tail and  num_valid_sc_slots = (sc_depth -1 ) ** 2) then
--				head <= std_logic_vector((unsigned(head) + unsigned(din.reserve_size)) mod sc_depth);
--				dout.spill <= '1';
--				dout.stall <= '1';
--			end if;
--		when free 		=>
--		when ensure		=>
--
--			
--	end case;	
--	end process;
--
--	process(rst, clk)
--	begin
--		if rst = '1' then
--			head <= (others => '0');
--			tail <= (others => '0');
--			num_valid_sc_slots <= 0;
--		elsif rising_edge(clk) then
--		end if;
--	end process;

end arch;



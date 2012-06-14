library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;



entity patmos_stack_cache_ctrl is
	port
	(
		clk					: in std_logic;
		din					: in patmos_stack_cache_ctrl_in;
		dout 				: out patmos_stack_cache_ctrl_out
	);
end entity patmos_stack_cache_ctrl;

architecture arch of patmos_stack_cache_ctrl is

signal number_of_bytes_in_stack_cache			: unsigned(4 downto 0) := (others => '0');
signal head										: unsigned(4 downto 0) := (others => '0');
signal tail										: unsigned(4 downto 0) := (others => '0');
type state_type is (s0,s1); 
signal stack_state: state_type := s0;
signal spill_counter 							: unsigned(4 downto 0):= (others => '0'); 
signal fill_counter 							: unsigned(4 downto 0):= (others => '0');
begin

process(din.instruction)
begin
	case din.instruction is -- unbounded cache
        when SRES =>
        	head <= head + din.stc_immediate_in;
        			--if (number_of_bytes_in_stack_cache = 0 and din.head_in = din.tail_in) then--stack empty
        			--	    dout.spill_out <= '0';
        			--		dout.tail_out <= din.tail_in;
        			--		dout.head_out <= (din.head_in + din.stc_immediate_in) mod 32;
        			--		number_of_bytes_in_stack_cache <= number_of_bytes_in_stack_cache + din.stc_immediate_in;
        						
        			--elsif ((32 - number_of_bytes_in_stack_cache) <  din.stc_immediate_in) then -- needs to spill
        			--	head <= (head + din.stc_immediate_in) mod 32;
        			--	stall <= '1';
        			--	spill_counter <=  "000000000000000000000000000" & din.stc_immediate_in - (32 - number_of_bytes_in_stack_cache);-- how much to spill?
        			--	case stack_state is
        			--		when s0 =>
        			--			fill_spill <= '1'; 
        			--			tail <= tail + 1;
        			--			head_tail <= tail;
        			--			if(tail = ) then
        			--				stack_state <= s1;
        			--			dout.tail_out <= (din.tail_in + din.stc_immediate_in - (32 - number_of_bytes_in_stack_cache)) mod 32;
        						
        			--			dout.st_out <= din.st_in + ("000000000000000000000000000" & din.stc_immediate_in);
        			--		when s1 =>
        			--		when others => null;
        			--	end case;
        			--	else 
        			--		number_of_bytes_in_stack_cache <= number_of_bytes_in_stack_cache + din.stc_immediate_in;
        			--		dout.spill_out <= '0';
        			--		dout.tail_out <= din.tail_in;
        			--	end if;	
        when SENS =>
        	
        when SFREE =>
        	head <= head - din.stc_immediate_in;
        when others => null;
    end case;
end process;

end arch;
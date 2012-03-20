library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_testbench is 
end entity patmos_testbench;

architecture timed of patmos_testbench is 
  signal clk                   :  std_logic := '0';
  signal rst                   :  std_logic := '1';
  signal fetch_din             :  fetch_in_type;

begin

  core: entity work.patmos_core(arch)
  port map(clk, rst, fetch_din);  
	
	
  clk <= not clk after 5 ns;
              --      "xpred00fff4321043210109876543210"
  fetch_din.instruction_word <= -- after 5 ns,  -- 
                      "00000000000001000000000000000011" after 5 ns, -- r2 <= r0 + 3 add immediate
                      "00000000000001100010000000000010" after 15 ns, -- r3 <= r2 + 2 add immediate -- alu forward
                      "00000000000000000000000010000001" after 25 ns, -- r0 <= r0 + 128 add immediate -- mem forward
                      "00000000000010000001000000000001" after 35 ns, -- r4 <= r1 + 1 add immediate
                      "00000000000010100010000000000001" after 45 ns; -- r5 <= r2 + 1 add immediate
                     -- "00000000000010100101000000000001" after 55 ns; -- r5 <= r5 + 1 add immediate
                      
                   --   "00000010000001100110011110000000" after 15 ns;--, -- r3 <= r? + r? add register -- 
                 --    
                 --    
                    
  -- pc <= (others => '0');
  rst <= '0' after 4 ns;

end architecture timed;

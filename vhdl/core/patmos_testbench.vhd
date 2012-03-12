library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_testbench is 
end entity patmos_testbench;

architecture timed of patmos_testbench is 
  signal clk                   :  std_logic := '0';
  signal rst                   :  std_logic := '1';
  signal instruction_word      :  unsigned(31 downto 0);

begin

  core: entity work.patmos_core(arch)
  port map(clk, rst, instruction_word);  
	
	
  clk <= not clk after 5 ns;
              --      "xpred00fff4321043210109876543210"
  instruction_word <= "00000000000000100000000000000001" after 5 ns,  -- r1 <= r0 + 1 add immediate
                      "00000000000001000001000000000010" after 15 ns;--, -- r2 <= r1 + 2 add immediate
                    --  "00000010000010010000000000010000" after 35 ns; -- r4 < int8_t(r16) unary
                  --  "00000010000001100010000010000000" after 25 ns;--, -- r3 <= r1 + r2 add register -- read after write
                 --   "00000000001000000000000010000001" after 35 ns, -- r16 <= r1 + 128 add immediate 
                 --    
                    
  -- pc <= (others => '0');
  rst <= '0' after 4 ns;

end architecture timed;

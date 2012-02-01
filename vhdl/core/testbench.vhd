library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


entity pc_tester is 
end entity;

architecture timed of pc_tester is 
signal clk : std_logic := '0';--   
signal rst : std_logic := '0';--   
signal instruction_word             : std_logic_vector(63 downto 0);
signal operation1                   : std_logic_vector(31 downto 0);
signal operation2                   : std_logic_vector(31 downto 0);
signal pc : std_logic_vector(31 downto 0);
signal pc_next :  std_logic_vector(31 downto 0);

--------------------------------------------
begin

   
 
uut1: entity work.fetch(arch)
port map(clk, rst, instruction_word, pc, operation1, operation2);

uut2: entity work.decode(arch)
port map(clk, rst, operation1, operation2);

clk <= not clk after 5 ns;
instruction_word <= "1111111111111111111111111111111100000000000000000000000000000000";
-- pc <= (others => '0');
rst <= '1' after 1 ns, '0' after 2 ns;

end architecture timed;

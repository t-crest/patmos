library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity test is 
end entity;

architecture timed of test is 
  signal       clk                        : std_logic := '0';
  signal       rst                        : std_logic := '0';
  signal       pc                         : unsigned(32 - 1 downto 0):= (others => '0');
  signal       instruction_word           : unsigned(32 - 1 downto 0) := "00000000000000000000100000100000" ;
  signal       operation1                 : unsigned(32 - 1 downto 0);
-- signal       immediate                  : std_logic_vector(21 downto 0);-- branch
--  signal       predicate                  : std_logic;

begin
-- check PC for normal and immediate
 --	uut1: entity work.pc_generator(arch)
--	port map(clk, rst, pc, PCBranch, "0000000000000000001000", '1');

  uut1: entity work.fetch(arch)
	port map(clk, rst, instruction_word, pc, PCNext, operation1, '1', "0000000000000000001000");
  clk <= not clk after 5 ns;

  rst <= '1' after 1 ns, '0' after 2 ns;
  
  
end architecture timed;
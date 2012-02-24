library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


entity pc_tester is 
end entity;

architecture timed of pc_tester is 
signal clk : std_logic := '0';--   
signal rst : std_logic := '0';--   
signal instruction_word             : std_logic_vector(63 downto 0);
signal pc : std_logic_vector(31 downto 0);
signal pc_next :  std_logic_vector(31 downto 0);
signal rs                              : std_logic_vector (4 downto 0);
signal rt                              : std_logic_vector (4 downto 0);
signal rd                              : std_logic_vector (4 downto 0);
signal func                            : std_logic_vector (2 downto 0);
signal read_data1                      : std_logic_vector(31 downto 0);
signal read_data2                      : std_logic_vector(31 downto 0);
signal write_data                      : std_logic_vector(31 downto 0);
signal operation1                      : std_logic_vector(31 downto 0);
signal operation2                      : std_logic_vector(31 downto 0);
signal write_enable                    : std_logic := '0';

--------------------------------------------
begin

 	uut1: entity work.fetch(arch)
	port map(clk, rst, instruction_word, pc, operation1, operation2);

	uut2: entity work.decode(arch)
	port map(clk, rst, operation1, operation2, rs, rt, rd, func);

	uut3: entity work.clock_input(arch)
	port map(clk,  , write_enable);

	uut4: entity work.register_file(arch)
	port map(clk, rst, rs, rt, rd, read_data1, read_data2, write_data, write_enable);

  uut5: entity work.execute(arch)
	port map(clk, read_data1, read_data2, write_data, func);

clk <= not clk after 5 ns;
instruction_word <= "1111111111111111111111111111111100000000000000000000100000100000";
-- pc <= (others => '0');
rst <= '1' after 1 ns, '0' after 2 ns;

end architecture timed;

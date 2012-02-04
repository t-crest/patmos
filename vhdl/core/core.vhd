library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


entity core is
  port
  (
    clk : in std_logic;
    rst : in std_logic;
    instruction_word : in std_logic_vector(63 downto 0)
  );
end entity core;


architecture arch of core is
signal rs                              : std_logic_vector (4 downto 0);
signal rt                              : std_logic_vector (4 downto 0);
signal rd                              : std_logic_vector (4 downto 0);
signal func                            : std_logic_vector (2 downto 0);
signal read_data1                      : std_logic_vector(31 downto 0);
signal read_data2                      : std_logic_vector(31 downto 0);
signal write_data                      : std_logic_vector(31 downto 0);
signal operation1                      : std_logic_vector(31 downto 0);
signal operation2                      : std_logic_vector(31 downto 0);
signal pc                              : std_logic_vector(31 downto 0);
signal pc_next                         : std_logic_vector(31 downto 0);

begin
 	uut1: entity work.fetch(arch)
	port map(clk, rst, instruction_word, pc, operation1, operation2);

	uut2: entity work.decode(arch)
	port map(clk, rst, operation1, operation2, rs, rt, rd, func);

	--uut3: entity work.clock_input(arch)
	--port map(clk,  , );

	uut4: entity work.register_file(arch)
	port map(clk, rs, rt, rd, read_data1, read_data2, write_data);

  uut5: entity work.execute(arch)
	port map(clk, read_data1, read_data2, write_data, func);

end arch;



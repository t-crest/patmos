
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity patmos_fetch is
    generic
    (
        pc_length                   : integer := 32;
        instruction_word_length     : integer := 32
    
    );
    port
    (
        clk                         : in std_logic;
        rst                         : in std_logic;
        instruction_word            : in unsigned(instruction_word_length - 1 downto 0);  
        pc                          : out unsigned(pc_length - 1 downto 0);
        pc_ctrl                     : in pc_type;
        operation1                  : out unsigned(31 downto 0);
        predicate                   : in std_logic;
        immediate                   : in unsigned(21 downto 0)
  --      operation2                  : out std_logic_vector(31 downto 0) 
    );
end entity patmos_fetch;

architecture arch of patmos_fetch is
--  signal pc_next                    : std_logic_vector(pc_length - 1 downto 0) := (others => '0');

begin
    uut1: entity work.pc_generator(arch)
	  port map(clk, rst, pc, pc_ctrl, immediate, predicate);

    fetch: process(clk)
    begin
        if (rising_edge(clk) and rst = '0') then
            operation1 <= instruction_word(31 downto 0);
         --   operation2 <= instruction_word(63 downto 32);
        end if;
    end process fetch;
end arch;
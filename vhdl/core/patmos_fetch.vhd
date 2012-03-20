
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_fetch is
    port
    (
        clk                         : in std_logic;
        rst                         : in std_logic;
        din                         : in fetch_in_type;
        dout                        : out fetch_out_type
    );
end entity patmos_fetch;

architecture arch of patmos_fetch is
--  signal pc_next                    : std_logic_vector(pc_length - 1 downto 0) := (others => '0');

begin
    uut1: entity work.patmos_pc_generator(arch)
	  port map(clk, rst, dout.pc, din.pc_ctrl, din.immediate, din.predicate);

    fetch: process(clk)
    begin
        if (rising_edge(clk) and rst = '0') then
            dout.operation1 <= din.instruction_word(31 downto 0);
         --   operation2 <= instruction_word(63 downto 32);
        end if;
    end process fetch;
end arch;
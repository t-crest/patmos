
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

begin

    fetch: process(clk, rst)
    begin
        if (rst = '1') then
          dout.pc <= (others => '0');
        elsif (rising_edge(clk) and rst = '0') then
            dout.instruction <= din.instruction;
            dout.pc <= din.pc;
        end if;
    end process fetch;
end arch;


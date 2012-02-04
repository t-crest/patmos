library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;


entity clock_input is
  generic 
  (
    input_length : integer := 32
  );
  port
  (
    clk : in std_logic;
    input : in std_logic_vector(input_length downto 0);
    output : out std_logic_vector(input_length downto 0)
  );
end entity clock_input;


architecture arch of clock_input is

begin 
  clock: process (clk)
  begin
    if rising_edge(clk) then
      output <= input;
    end if;
  end process clock;
end arch;

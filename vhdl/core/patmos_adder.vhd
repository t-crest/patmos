library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_adder is
  port
  (
    data_in1                : in unsigned(31 downto 0);
    data_in2                : in unsigned(31 downto 0);
    data_out                : out unsigned(31 downto 0)
  );    
end entity patmos_adder;
architecture arch of patmos_adder is
begin
     add: process (data_in1, data_in2)
          begin
            data_out <= data_in1 + data_in2;
          end process add;
end arch;



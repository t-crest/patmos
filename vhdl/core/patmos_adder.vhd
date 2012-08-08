library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_adder is
  port
  (
    data_in1                : in std_logic_vector(31 downto 0);
    data_in2                : in std_logic_vector(31 downto 0);
    data_out                : out std_logic_vector(31 downto 0)
  );    
end entity patmos_adder;
architecture arch of patmos_adder is
begin
     add: process (data_in1, data_in2)
          begin
            data_out <= std_logic_vector(unsigned(data_in1) + unsigned(data_in2));
          end process add;
end arch;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
entity patmos_adder2 is
  port
  (
    data_in1                : in std_logic_vector(31 downto 0);
    data_in2                : in std_logic_vector(31 downto 0);
    data_out                : out std_logic_vector(31 downto 0)
  );    
end entity patmos_adder2;
architecture arch of patmos_adder2 is
begin
     add: process (data_in1, data_in2)
          begin
            data_out <= std_logic_vector(unsigned(data_in1) - unsigned(data_in2));
          end process add;
end arch;

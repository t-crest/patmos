library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_mux_3x32 is
  port
  (
    data_in1                : in unsigned(31 downto 0);
    data_in2                : in unsigned(31 downto 0);
    data_in3                : in unsigned(31 downto 0);
    sel                     : in unsigned(1 downto 0);
    data_out                : out unsigned(31 downto 0)
  );    
end entity patmos_mux_3x32;

architecture arch of patmos_mux_3x32 is
begin
    data_out <= data_in1 when (sel = "00") else data_in2 when (sel = "01") else data_in3 when (sel = "10");
end arch;

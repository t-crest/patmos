library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_mux_32 is
  port
  (
    data_in1                : in unsigned(31 downto 0);
    data_in2                : in unsigned(31 downto 0);
    sel                     : in std_logic;
    data_out                : out unsigned(31 downto 0)
  );    
end entity patmos_mux_32;
architecture arch of patmos_mux_32 is
begin
   process(data_in1, data_in2, sel)
	begin
		if(sel = '0') then
			data_out <= data_in1;
		elsif(sel = '1') then
			data_out <= data_in2;
			else data_out <= data_in1;
			end if;
	end process;
end arch;




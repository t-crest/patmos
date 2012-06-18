library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;


entity patmos_equal_check is
    port
    (
        din1                         : in unsigned(31 downto 0);
        din2                         : in unsigned(31 downto 0);
        dout                         : out std_logic
    );
end entity patmos_equal_check;

architecture arch of patmos_equal_check is

begin
	process (din1, din2)
	begin
		if (din1 = din2) then
			dout <= '0';
		else
			dout <= '1';
		end if;
	end process;
  
end arch;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_mem_stage is 
  port
  (
    clk                          : in std_logic;
    write_register_in            : in unsigned(4 downto 0);
    write_data_in                : in unsigned(31 downto 0);
    write_enable_in              : in std_logic;
    write_register_out           : out unsigned(4 downto 0);
    write_data_out               : out unsigned(31 downto 0);
    write_enable_out             : out std_logic
  );
end entity patmos_mem_stage;


architecture arch of patmos_mem_stage is

begin
  --mem/wb register
  mem_wb: process(clk)
  begin
    if (rising_edge(clk)) then
      write_data_out <= write_data_in;
      write_register_out <= write_register_in;
      write_enable_out <= write_enable_in;
    end if;
  end process mem_wb;

end arch;
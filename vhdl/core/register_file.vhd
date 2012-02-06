library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity register_file is
  port
  (
    clk           : in std_logic;
    read_address1 : in std_logic_vector(4 downto 0);
    read_address2 : in std_logic_vector(4 downto 0);
    write_address : in std_logic_vector(4 downto 0);
    read_data1    : out std_logic_vector(31 downto 0);
    read_data2    : out std_logic_vector(31 downto 0);
    write_data    : in std_logic_vector(31 downto 0);
    write_enable  : in std_logic
  );
end entity register_file;

architecture arch of register_file is
type register_bank is array (0 to 31) of std_logic_vector(31 downto 0);
signal reg_bank : register_bank;
begin
  --                                  
  reg_bank (0) <= "00000000000000000000000000000001";
  reg_bank (1) <= "00000000000000000000000000000011";
  reg_bank (2) <= "00000000000000000000000000000000";
  read_data1 <= reg_bank(to_integer(unsigned(read_address1)));
  read_data2 <= reg_bank(to_integer(unsigned(read_address2)));
  read:  process (clk)
  begin
    if rising_edge(clk) then
      
    end if;
 end process read;
  
  write:  process (clk)
   begin
   if rising_edge(clk) then
     if (write_enable = '1') then
     --   reg_bank(to_integer(unsigned(write_address))) <= write_data;
     end if;
   end if;
    end process write;
end arch;

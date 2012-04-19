------------------------------------------
--special purpose registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity patmos_special_register_file is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
    read_address1 : in unsigned(4 downto 0);
    read_address2 : in unsigned(4 downto 0);
    write_address : in unsigned(4 downto 0);
    read_data1    : out unsigned(31 downto 0);
    read_data2    : out unsigned(31 downto 0);
    write_data    : in unsigned(31 downto 0);
    write_enable  : in std_logic
  );
end entity patmos_special_register_file;

architecture arch of patmos_special_register_file is
type register_bank is array (0 to 15) of unsigned(31 downto 0);
signal reg_bank : register_bank;
signal reg_read_address1, reg_read_address2 : unsigned(4 downto 0);
begin
  --                                  
  ------ latch read address
  latch_read_address:  process (clk, rst)
  begin
    if(rst = '1') then
        for i in 0 to 31 loop -- initialize register file
          reg_bank(i)<= (others => '0');
        end loop;
    elsif rising_edge(clk) then
   --   if (read_enable) then
          reg_read_address1 <= read_address1;
          reg_read_address2 <= read_address2;
   --   end if;
    end if;
   end process latch_read_address;
   
 ------ read process (or should be async?)
  read:  process (reg_read_address1, reg_read_address2)
  begin
    read_data1 <= reg_bank(to_integer(unsigned(reg_read_address1)));
    read_data2 <= reg_bank(to_integer(unsigned(reg_read_address2)));
  end process read;
  
 ------ write process
  write:  process (clk)
   begin
   if rising_edge(clk) then
     if (write_enable = '1') then
       reg_bank(to_integer(unsigned(write_address))) <= write_data;
     end if;
   end if;
    end process write;
end arch;


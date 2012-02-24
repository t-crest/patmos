--TO DO: add hazard detection and forwarding

------------------------------------------
--general purpose registers
------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;


entity register_file is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
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
signal reg_read_address1, reg_read_address2 : std_logic_vector(4 downto 0);
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

------------------------------------------
--special purpose registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;


entity s_register_file is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
    read_address1 : in std_logic_vector(4 downto 0);
    read_address2 : in std_logic_vector(4 downto 0);
    write_address : in std_logic_vector(4 downto 0);
    read_data1    : out std_logic_vector(31 downto 0);
    read_data2    : out std_logic_vector(31 downto 0);
    write_data    : in std_logic_vector(31 downto 0);
    write_enable  : in std_logic
  );
end entity s_register_file;

architecture arch of s_register_file is
type register_bank is array (0 to 15) of std_logic_vector(31 downto 0);
signal reg_bank : register_bank;
signal reg_read_address1, reg_read_address2 : std_logic_vector(4 downto 0);
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

------------------------------------------
-- predicate registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity predicate_register is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
    read_address  : in std_logic_vector(2 downto 0);
    write_address : in std_logic_vector(2 downto 0);
    read_data     : out std_logic;
    write_data    : in std_logic;
    write_enable  : in std_logic
  );
end entity predicate_register;

architecture arch of predicate_register is
--signal register_bank : std_logic_vector(7 downto 0);

signal reg_read_address : std_logic_vector(2 downto 0);
begin
  --                                  
  ------ latch read address
  latch_read_address:  process (clk, rst)
  begin
    if(rst = '1') then
        -- initialize register file
          predicate_register_bank <= (others => '0');
    elsif rising_edge(clk) then
   --   if (read_enable) then ?? need read enable?!!
          reg_read_address <= read_address;
   --   end if;
    end if;
   end process latch_read_address;
   
 ------ read process (or should be async?)
  read:  process (reg_read_address)
  begin
    read_data <= predicate_register_bank(to_integer(unsigned(reg_read_address)));
  end process read;
  
 ------ write process
  write:  process (clk)
   begin
   if rising_edge(clk) then
     if (write_enable = '1') then
       predicate_register_bank(to_integer(unsigned(write_address))) <= write_data;
     end if;
   end if;
    end process write;
end arch;

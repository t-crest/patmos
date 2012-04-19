------------------------------------------
-- predicate registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_predicate_registers is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
    read_address  : in unsigned(2 downto 0);
    write_address : in unsigned(2 downto 0);
    read_data     : out std_logic;
    write_data    : in std_logic;
    write_enable  : in std_logic
  );
end entity patmos_predicate_registers;

architecture arch of patmos_predicate_registers is
--signal register_bank : std_logic_vector(7 downto 0);

signal reg_read_address : unsigned(2 downto 0);
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

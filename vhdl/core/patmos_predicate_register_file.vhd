------------------------------------------
-- predicate registers
------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_predicate_register_file is --general purpose registers
  port
  (
    clk           : in std_logic;
    rst           : in std_logic;
   -- read_address1 : in unsigned(2 downto 0);
   -- read_address2 : in unsigned(2 downto 0);
    write_address : in unsigned(2 downto 0);
    read_data    : out unsigned(7 downto 0);
    --read_data2    : out unsigned(31 downto 0);
    write_data    : in std_logic;
 --   read_enable	  : in std_logic;
    write_enable  : in std_logic
  );
end entity patmos_predicate_register_file;

architecture arch of patmos_predicate_register_file is
--type predicate_register_bank is array (0 to 31) of unsigned(31 downto 0);
signal predicate_reg_bank : unsigned(7 downto 0);
--signal reg_read_address1, reg_read_address2 : unsigned(4 downto 0);
--signal reg_write_enable : std_logic;

begin
  --                                  
  ------ latch read address
  latch_read_address:  process (clk, rst)
  begin
    if(rst = '1') then
        for i in 1 to 7 loop -- initialize register file
          predicate_reg_bank(i) <= '0';
        end loop;
        predicate_reg_bank(0) <= '1';
    elsif rising_edge(clk) then
   --   if (read_enable) then
      --    reg_read_address1 <= read_address1;
      --    reg_read_address2 <= read_address2;
          if (write_enable = '1') then
             predicate_reg_bank(to_integer(unsigned(write_address))) <= write_data;
           end if;
   --   end if;
    end if;
   end process latch_read_address;
   
 ------ read process (or should be async?)
 -- read:  process (read_enable)
  --begin
   -- if ((read_address1 = write_address) and write_enable = '1' )then
  --   read_data1 <= write_data;
  -- else 
      read_data <= predicate_reg_bank;
 --   end if;
    
  -- if (read_address2 = write_address) and write_enable = '1' then
    --  read_data2 <= write_data;
  -- else   
    --  read_data2 <= reg_bank(to_integer(unsigned(read_address2)));
  --  end if;
 -- end process read;
end arch;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_instruction_memory is
  port(
  		clk					  : in std_logic;
  	 	rst                   : in std_logic;
        address               : in unsigned(31 downto 0);
        inst_in               : in unsigned(31 downto 0); -- store
        inst_out              : out unsigned(31 downto 0); -- load
        read_enable           : in std_logic;
        write_enable          : in std_logic
  );
end entity patmos_instruction_memory;


architecture arch of patmos_instruction_memory is


  type instruction_memory is array (0 to 1024) of unsigned(31 downto 0);
  signal inst_mem : instruction_memory;

begin
  read: process(address, read_enable)
  begin
  	--if (read_enable = '1') then
  		inst_out <= inst_mem(to_integer(unsigned(address)));
  --	end if;
  end process;
  
  mem : process(clk, rst)
  begin
    if(rst = '1') then   
    inst_mem(7) <= "00000000000000100000000000000010";
    inst_mem(8) <= "00000010100101000101000110000000";
    inst_mem(9) <= "00000000000000000000000000000001";
    inst_mem(10) <= "00000010000101101010000010000111";
    inst_mem(11) <= "00000111110000001011000010000100";
    inst_mem(12) <= "00000000000000000000000000000001";
    inst_mem(13) <= "00000010100111100101000110000001";
    inst_mem(14) <= "00000010100111100101000110000001";
    inst_mem(15) <= "00000000000001110011000000000001";
    inst_mem(16) <= "00000000000001000000000001010000";
    inst_mem(17) <= "00000010100101000101000110000000";
    inst_mem(18) <= "00000000000000000000000000000001";
    inst_mem(19) <= "00000010000101100011010100000111";
    inst_mem(20) <= "00000111110000001011000110000100";
    inst_mem(21) <= "00000000000000000000000000000001";
    inst_mem(22) <= "00000010110001100111011110000001";
    inst_mem(23) <= "00000000000000000000000000000001";
    inst_mem(24) <= "00000000000001100000000000000010";
    inst_mem(25) <= "00000111110000000000001100000010";
    inst_mem(26) <= "00000000000000000000000000000001";
    elsif (rising_edge(clk)) then
      if(write_enable = '1') then
        inst_mem(to_integer(unsigned(address))) <= inst_in;
      end if;
    end if;
  end process mem;

end arch;  

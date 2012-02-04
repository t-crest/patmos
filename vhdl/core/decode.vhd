library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity decode is 
    port (
        clk, rst                        : in std_logic;
        operation1                      : in std_logic_vector (31 downto 0);
        operation2                      : in std_logic_vector (31 downto 0);
        rs                              : out std_logic_vector (4 downto 0);
        rt                              : out std_logic_vector (4 downto 0);
        rd                              : out std_logic_vector (4 downto 0);
        func                            : out std_logic_vector (2 downto 0)
    );
end entity decode;

architecture arch of decode is 
signal operation1_out : std_logic_vector (31 downto 0);
signal operation2_out : std_logic_vector (31 downto 0);
begin
  decode: process(clk)
  begin
    if rising_edge(clk) then
      operation1_out <= operation1;
      operation2_out <= operation2;
      rs <= operation1_out(4 downto 0);
      rt <= operation1_out(9 downto 5);
      rd <= operation1_out(14 downto 10);
      func <= operation1_out(17 downto 15);
    end if;
  end process decode; 
  
 -- read_registers: process(clk)
 -- begin
  --   if rising_edge(clk) then
       
   --  end if;
  -- end process read_registers;
end arch;


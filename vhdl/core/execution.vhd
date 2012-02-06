library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;


entity execute is
  port
  (
    clk               : in std_logic;
    rs                : in std_logic_vector(31 downto 0);
    rt                : in std_logic_vector(31 downto 0);
    rd                : out std_logic_vector(31 downto 0);
    func              : in std_logic_vector(2 downto 0)-- which function of alu?
  );
  
end entity execute;

architecture arch of execute is
begin
  alu: process(clk)
  begin
    if rising_edge(clk) then
    case func is
      when "000" => rd <= rs + rt;
      when "001" => rd <= rs - rt;
      when "010" => rd <= rt - rs;
   --   when "011" =>;
   --   when "100" =>;
   --   when "101" =>;
   --   when "110" =>;
   --   when "111" =>;
      when others => rd <= rs + rt;
    end case;
  end if;
  end process alu;
end arch;

 
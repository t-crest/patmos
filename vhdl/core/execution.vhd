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
    func              : in std_logic_vector(2 downto 0);-- which function of alu?
    write_enable      : out std_logic
    -- for vliw this should be extended to 5 bits and the three lower bits are used in ALUi instructions
  );
  
end entity execute;

architecture arch of execute is
signal test : unsigned(31 downto 0);

--shift left logical
function shift_left_logical (rs, rt : std_logic_vector(31 downto 0))
                 return std_logic_vector is
  variable shift_out  : std_logic_vector(31 downto 0):= (others => '0');
  variable shift_value  : std_logic_vector(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out := rs(31 downto 1) & '0'; 
      when "00010" => shift_out := rs(31 downto 2) & "00";   
      when "00011" => shift_out := rs(31 downto 3) & "000"; 
      when "00100" => shift_out := rs(31 downto 4) & "0000"; 
      when "00101" => shift_out := rs(31 downto 5) & "00000"; 
      when "00110" => shift_out := rs(31 downto 6) & "000000"; 
      when "00111" => shift_out := rs(31 downto 7) & "0000000"; 
      when "01000" => shift_out := rs(31 downto 8) & "00000000"; 
      when "01001" => shift_out := rs(31 downto 9) & "000000000"; 
      when "01010" => shift_out := rs(31 downto 10) & "0000000000"; 
      when "01011" => shift_out := rs(31 downto 11) & "00000000000"; 
      when "01100" => shift_out := rs(31 downto 12) & "000000000000"; 
      when "01101" => shift_out := rs(31 downto 13) & "0000000000000"; 
      when "01110" => shift_out := rs(31 downto 14) & "00000000000000"; 
      when "01111" => shift_out := rs(31 downto 15) & "000000000000000"; 
      when "10000" => shift_out := rs(31 downto 16) & "0000000000000000"; 
      when "10001" => shift_out := rs(31 downto 17) & "00000000000000000"; 
      when "10010" => shift_out := rs(31 downto 18) & "000000000000000000";   
      when "10011" => shift_out := rs(31 downto 19) & "0000000000000000000"; 
      when "10100" => shift_out := rs(31 downto 20) & "00000000000000000000"; 
      when "10101" => shift_out := rs(31 downto 21) & "000000000000000000000"; 
      when "10110" => shift_out := rs(31 downto 22) & "0000000000000000000000"; 
      when "10111" => shift_out := rs(31 downto 23) & "00000000000000000000000"; 
      when "11000" => shift_out := rs(31 downto 24) & "000000000000000000000000"; 
      when "11001" => shift_out := rs(31 downto 25) & "0000000000000000000000000"; 
      when "11010" => shift_out := rs(31 downto 26) & "00000000000000000000000000"; 
      when "11011" => shift_out := rs(31 downto 27) & "000000000000000000000000000"; 
      when "11100" => shift_out := rs(31 downto 28) & "0000000000000000000000000000"; 
      when "11101" => shift_out := rs(31 downto 29) & "00000000000000000000000000000"; 
      when "11110" => shift_out := rs(31 downto 30) & "000000000000000000000000000000"; 
      when "11111" => shift_out := rs(31 downto 31) & "0000000000000000000000000000000"; 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_left_logical;

function shift_right_logical (rs, rt : std_logic_vector(31 downto 0))
                 return std_logic_vector is
  variable shift_out  : std_logic_vector(31 downto 0):= (others => '0');
  variable shift_value  : std_logic_vector(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out :=  '0' & rs(31 downto 1); 
      when "00010" => shift_out :=  "00" & rs(31 downto 2);   
      when "00011" => shift_out :=  "000" & rs(31 downto 3); 
      when "00100" => shift_out :=  "0000" & rs(31 downto 4); 
      when "00101" => shift_out :=  "00000" & rs(31 downto 5); 
      when "00110" => shift_out :=  "000000" & rs(31 downto 6); 
      when "00111" => shift_out :=  "0000000" & rs(31 downto 7); 
      when "01000" => shift_out :=  "00000000" & rs(31 downto 8); 
      when "01001" => shift_out :=  "000000000" & rs(31 downto 9) ; 
      when "01010" => shift_out :=  "0000000000" & rs(31 downto 10); 
      when "01011" => shift_out :=  "00000000000" & rs(31 downto 11); 
      when "01100" => shift_out :=  "000000000000" & rs(31 downto 12); 
      when "01101" => shift_out :=  "0000000000000" & rs(31 downto 13); 
      when "01110" => shift_out :=  "00000000000000" & rs(31 downto 14); 
      when "01111" => shift_out :=  "000000000000000" & rs(31 downto 15); 
      when "10000" => shift_out :=  "0000000000000000" & rs(31 downto 16); 
      when "10001" => shift_out :=  "00000000000000000" & rs(31 downto 17); 
      when "10010" => shift_out :=  "000000000000000000" & rs(31 downto 18);   
      when "10011" => shift_out :=  "0000000000000000000" & rs(31 downto 19); 
      when "10100" => shift_out :=  "00000000000000000000" & rs(31 downto 20); 
      when "10101" => shift_out :=  "000000000000000000000" & rs(31 downto 21); 
      when "10110" => shift_out :=  "0000000000000000000000" & rs(31 downto 22); 
      when "10111" => shift_out :=  "00000000000000000000000" & rs(31 downto 23); 
      when "11000" => shift_out :=  "000000000000000000000000" & rs(31 downto 24); 
      when "11001" => shift_out :=  "0000000000000000000000000" & rs(31 downto 25); 
      when "11010" => shift_out :=  "00000000000000000000000000" & rs(31 downto 26); 
      when "11011" => shift_out :=  "000000000000000000000000000" & rs(31 downto 27); 
      when "11100" => shift_out :=  "0000000000000000000000000000" & rs(31 downto 28); 
      when "11101" => shift_out :=  "00000000000000000000000000000" & rs(31 downto 29); 
      when "11110" => shift_out :=  "000000000000000000000000000000" & rs(31 downto 30); 
      when "11111" => shift_out :=  "0000000000000000000000000000000" & rs(31 downto 31); 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_right_logical;

function shift_right_arith (rs, rt : std_logic_vector(31 downto 0))
                 return std_logic_vector is
  variable shift_out  : std_logic_vector(31 downto 0):= (others => '0');
  variable shift_value  : std_logic_vector(4 downto 0):= (others => '0');
begin
  shift_value(4 downto 0 ) := rt(4 downto 0);
  case (shift_value) is
      when "00000" => shift_out := rs;
      when "00001" => shift_out :=  rs(31) & rs(31 downto 1); 
      when "00010" => shift_out :=  rs(31 downto 30) & rs(31 downto 2);   
      when "00011" => shift_out :=  rs(31 downto 29) & rs(31 downto 3); 
      when "00100" => shift_out :=  rs(31 downto 28) & rs(31 downto 4); 
      when "00101" => shift_out :=  rs(31 downto 27) & rs(31 downto 5); 
      when "00110" => shift_out :=  rs(31 downto 26) & rs(31 downto 6); 
      when "00111" => shift_out :=  rs(31 downto 25) & rs(31 downto 7); 
      when "01000" => shift_out :=  rs(31 downto 24) & rs(31 downto 8); 
      when "01001" => shift_out :=  rs(31 downto 23) & rs(31 downto 9) ; 
      when "01010" => shift_out :=  rs(31 downto 22) & rs(31 downto 10); 
      when "01011" => shift_out :=  rs(31 downto 21) & rs(31 downto 11); 
      when "01100" => shift_out :=  rs(31 downto 20) & rs(31 downto 12); 
      when "01101" => shift_out :=  rs(31 downto 19) & rs(31 downto 13); 
      when "01110" => shift_out :=  rs(31 downto 18) & rs(31 downto 14); 
      when "01111" => shift_out :=  rs(31 downto 17) & rs(31 downto 15); 
      when "10000" => shift_out :=  rs(31 downto 16) & rs(31 downto 16); 
      when "10001" => shift_out :=  rs(31 downto 15) & rs(31 downto 17); 
      when "10010" => shift_out :=  rs(31 downto 14) & rs(31 downto 18);   
      when "10011" => shift_out :=  rs(31 downto 13) & rs(31 downto 19); 
      when "10100" => shift_out :=  rs(31 downto 12) & rs(31 downto 20); 
      when "10101" => shift_out :=  rs(31 downto 11) & rs(31 downto 21); 
      when "10110" => shift_out :=  rs(31 downto 10) & rs(31 downto 22); 
      when "10111" => shift_out :=  rs(31 downto 9) & rs(31 downto 23); 
      when "11000" => shift_out :=  rs(31 downto 8) & rs(31 downto 24); 
      when "11001" => shift_out :=  rs(31 downto 7) & rs(31 downto 25); 
      when "11010" => shift_out :=  rs(31 downto 6) & rs(31 downto 26); 
      when "11011" => shift_out :=  rs(31 downto 5) & rs(31 downto 27); 
      when "11100" => shift_out :=  rs(31 downto 4) & rs(31 downto 28); 
      when "11101" => shift_out :=  rs(31 downto 3) & rs(31 downto 29); 
      when "11110" => shift_out :=  rs(31 downto 2) & rs(31 downto 30); 
      when "11111" => shift_out :=  rs(31 downto 1) & rs(31 downto 31); 
      when  others => shift_out := rs;
   end case;
  return shift_out ;
end shift_right_arith;

begin
  alu: process(clk)
  begin
    if rising_edge(clk) then
--if ALUi
    write_enable <= '1';
    case func is
      when "000" => rd <= rs + rt;
      when "001" => rd <= rs - rt;
      when "010" => rd <= rt - rs;
      when "011" => rd <= shift_left_logical(rs, rt);
      when "100" => rd <= shift_right_logical(rs, rt);
   --   when "101" =>;
      when "110" => rd <= rs or rt ;
      when "111" => rd <= rs and rt ;
      when others => rd <= rs + rt;
    end case;
  end if;
--end if ALUi
  end process alu;
end arch;

 
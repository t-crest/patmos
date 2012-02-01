
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity pc_generator is 
    generic
    (
        pc_length                   : integer := 32;
        instruction_word_length     : integer := 64
    );
    
    port
    (
         clk                        : in std_logic;
         rst                        : in std_logic;
         pc                         : in std_logic_vector(pc_length - 1 downto 0);
         pc_next                    : out std_logic_vector(pc_length - 1 downto 0)
       --  ctrl                       : in std_logic_vector(3 - 1 downto 0); -- number of states pc can take
       --  imm                        : in std_logic_vector(11 downto 0)--; jump
    );
end entity pc_generator;

architecture arch of pc_generator is
  signal ctrl                         : std_logic_vector(2 downto 0) := "111";
begin
     pc_gen: process (clk)
    begin
      --if (rst = '1') then
        --   pc_next <= (others => '0');  
       if (clk = '1') then
         case ctrl is
          -- when "000" => pc_next <= ret_address; --function return
         --  when "001" => pc_next <= ; -- jump
         --  when "010" => pc_next <= ; --branch
           when "111" => pc_next <= pc + 4;
           when others => pc_next <= pc + 4;
          end case;   
       end if;
    end process pc_gen ;        

end arch;
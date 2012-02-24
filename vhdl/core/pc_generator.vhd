---------------------------------
-- pc generation
---------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.type_package.all;

entity pc_generator is 
    generic
    (
        pc_length                   : integer := 32;
        instruction_word_length     : integer := 64--;
       
    );
    
    port
    (
         clk                        : in std_logic;
         rst                        : in std_logic;
         pc                         : out std_logic_vector(pc_length - 1 downto 0);-- (others => '0');
     --    pc_next                    : out std_logic_vector(pc_length - 1 downto 0);
         pc_ctrl                    : in pc_type; -- pc states
         immediate                  : in std_logic_vector(21 downto 0);-- branch
         predicate                  : in std_logic 
    );
end entity pc_generator;

architecture arch of pc_generator is
  signal ctrl                                : std_logic_vector(2 downto 0) := "111";
  signal sign_extended_immediate             : std_logic_vector(31 downto 0);
  signal pc_next                             : std_logic_vector(pc_length - 1 downto 0) := (others => '0');
begin
    uut1: entity work.sign_extension(arch)
	  port map(immediate, sign_extended_immediate);
	   
    pc_gen: process (clk, rst)
    begin
      if (rst = '1') then
           pc_next <= (others => '0');  
           pc <= (others => '0');  
      elsif (clk = '1') then
         if predicate = '1' then -- investigate predicate register
            case pc_ctrl is
               when PCNext => pc <= pc_next + 4; -- next instruction
              -- when  => pc_next <= pc; -- function call
               when PCBranch => pc <= (pc_next + 4 + sign_extended_immediate); --branch
              -- when  => pc_next <= pc + 4;
               when others => pc <= pc_next + 4;
            end case;   
         elsif predicate = '0' then -- control signals
           pc_next <= pc_next + 4;
         end if;
         pc_next <= pc_next + 4;
      end if;
      
    end process pc_gen ;        

end arch;

-------------------------------------
-- sign extension for branch
-------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity sign_extension is
  port 
  (
    immediate                       : in std_logic_vector(21 downto 0);
    sign_extended_immediate         : out std_logic_vector(31 downto 0)
  );
end entity sign_extension;

architecture arch of sign_extension is
signal sign_bit : std_logic;
begin
  sign_bit <= immediate(21);
  sign_extended_immediate <=  (sign_bit & sign_bit & sign_bit & sign_bit & sign_bit & sign_bit & sign_bit & sign_bit &immediate) & "00";
end arch;


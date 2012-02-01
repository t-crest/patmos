
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity fetch is
    generic
    (
        pc_length                   : integer := 32;
        instruction_word_length     : integer := 64
    );
    port
    (
        clk                         : in std_logic;
        rst                         : in std_logic;
        instruction_word            : in std_logic_vector(instruction_word_length - 1 downto 0);  
        pc                          : out std_logic_vector(pc_length - 1 downto 0);
      --  pc_next                     : in std_logic_vector(pc_length - 1 downto 0);
        operation1                  : out std_logic_vector(31 downto 0);
        operation2                  : out std_logic_vector(31 downto 0) 
    );
end entity fetch;

architecture arch of fetch is
  signal pc_next                    : std_logic_vector(pc_length - 1 downto 0) := (others => '0');
--component pc_generator is
  --  port 
  --  (
  --      clk                         : in std_logic;
  --      rst                         : in std_logic;
  --      pc                          : in std_logic_vector(pc_length - 1 downto 0);
   --     pc_next                     : out std_logic_vector(pc_length - 1 downto 0)
       -- ctrl                        : in std_logic_vector(pc_length - 1 downto 0);
       -- imm                         : in std_logic_vector(11 downto 0)--; jump
            
   --  );
--end component;
 
begin
   -- pc_gen: pc_generator port map
    --(
      -- clk, rst, pc_next, pc
    --);
     
    new_pc: process(clk, rst)
    begin 
        if (rst = '1') then
            pc <= (others => '0');
            
        elsif rising_edge(clk) then
          --case ctrl, jump, branch, ...
            pc <= pc_next + 4;  
            pc_next <= pc_next + 4; 
            
        end if;
    end process new_pc;
    fetch: process(clk)
    begin
        if (rising_edge(clk) and rst = '0') then
            operation1 <= instruction_word(31 downto 0);
            operation2 <= instruction_word(63 downto 32);
        end if;
    end process fetch;
end arch;
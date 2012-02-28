library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity clock_in is
  generic 
  (
    input_length : integer := 32
  );
  port
  (
    clk : in std_logic;
 --   rst : in std_logic;
    input : in unsigned(input_length - 1 downto 0);
    output : out unsigned(input_length - 1 downto 0)
  );
end entity clock_in;


architecture arch of clock_in is

begin 
  clock: process (clk)
  begin
    if rising_edge(clk) then
      output <= input;
    end if;
  end process clock;
end arch;

-----------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity clock_inst_type is
 
  port
  (
    clk : in std_logic;
    input : in instruction_type;
    output : out instruction_type
  );
end entity clock_inst_type;


architecture arch of clock_inst_type is

begin 
  clock: process (clk)
  begin
    if rising_edge(clk) then
      output <= input;
    end if;
  end process clock;
end arch;

-----------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity function_type is
 
  port
  (
    clk : in std_logic;
    input : in instruction_type;
    output : out instruction_type
  );
end entity function_type;


architecture arch of function_type is

begin 
  clock: process (clk)
  begin
    if rising_edge(clk) then
      output <= input;
    end if;
  end process clock;
end arch;

----------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity clock_ALU_instruction_type is
 
  port
  (
    clk : in std_logic;
    input : in ALU_inst_type;
    output : out ALU_inst_type
  );
end entity clock_ALU_instruction_type;


architecture arch of clock_ALU_instruction_type is

begin 
  clock: process (clk)
  begin
    if rising_edge(clk) then
      output <= input;
    end if;
  end process clock;
end arch;


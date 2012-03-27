library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_rom is
  port
  (
    addr                      : in unsigned(31 downto 0);
    rom_out                   : out unsigned(31 downto 0)
  );
end entity patmos_rom;

architecture arch of patmos_rom is
  type rom is array (0 to 255) of unsigned(31 downto 0);
  signal rom_unit :rom;
begin
  rom_unit(0) <= "00000000000001000000000000000011";
  rom_unit(4) <= "00000000000001100010000000000010";
  rom_unit(8) <= "00000000000000000000000010000001";
  rom_out <= rom_unit(to_integer(addr));
end arch;


--library ieee;
--use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;

--entity patmos_instruction_memory is
--  port
--  (
   -- clk                           : in std_logic;
 --   pc                            : in unsigned(32 - 1 downto 0); 
 --   inst                          : out unsigned(31 downto 0)
 -- );
--end entity patmos_instruction_memory;

--architecture arch of patmos_instruction_memory is
--  signal inst0, inst1, inst2, inst3   : unsigned(7 downto 0);
 -- signal pc1, pc2, pc3                : unsigned(31 downto 0);
  
--begin
--  pc1 <= pc + 1;
--  pc2 <= pc + 2;
--  pc3 <= pc + 3;
--  rom1: entity work.patmos_rom(arch) 
--  port map(pc, inst0(7 downto 0));
--  rom2: entity work.patmos_rom(arch) 
--  port map(pc1 , inst1(7 downto 0));
--  rom3: entity work.patmos_rom(arch) 
--  port map(pc2 , inst2(7 downto 0));
--  rom4: entity work.patmos_rom(arch) 
--  port map(pc3 , inst3(7 downto 0));
--end arch;


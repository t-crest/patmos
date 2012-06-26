library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_ram is
  port
  (
    addr                      : in unsigned(31 downto 0);
    rom_out                   : out unsigned(31 downto 0)
  );
end entity patmos_ram;

architecture arch of patmos_ram is
  type rom is array (0 to 255) of unsigned(31 downto 0);
  signal rom_unit :rom;
begin
-- CPU pipe check
 -- rom_unit(0) <= "00000000000001000000000000000011";
 -- rom_unit(4) <= "00000000000001100010000000000010"; --check fw_alu
 -- rom_unit(8) <= "00000000000000000010000010000001";  --check fw_me
 -- rom_unit(12) <= "00000000000011000000000000000001"; -- a random instruction!
 -- rom_unit(16) <= "00000000000011100000000000000001"; -- a random instruction!
 -- rom_unit(20) <= "00000000001011000000000000000001"; -- another random instruction!
 -- rom_unit(24) <= "00000000001001000000000000000010"; -- another random instruction!
 -- rom_unit(28) <= "00000010110000000000000100000111"; -- store
 -- rom_unit(32) <= "00000010100001000000000000000000"; -- load
 -- rom_unit(36) <= "00000111110000000111001100001100";--beq
 -- rom_unit(40) <= "00000000001111000000000000000101";-- instruction after branch, delayed branch!!
 -- rom_unit(52) <= "00000000000110000101000000000001"; -- branch is taken!
 -- rom_unit(56) <= "00000010000110000011000000000011";  -- shift

-- UART check
--  rom_unit(0) <= "00000000000001000000000000000011";
--  rom_unit(4) <= "00000000000001100010000000000010"; --check fw_alu
--  rom_unit(8) <= "00000000000000000010000010000001";  --check fw_me
--  rom_unit(12) <= "00000000000011000000000000000001"; -- a random instruction!
--  rom_unit(16) <= "00000000000011100000000000000001"; -- a random instruction!
--  rom_unit(20) <= "00000000001011000000000000000001"; -- another random instruction!
--  rom_unit(24) <= "00000000001001000000000000000010"; -- another random instruction!
--  rom_unit(28) <= "00000010110000000000000100000111"; -- store
--  rom_unit(32) <= "00000010100001000000000000000000"; -- load
--  rom_unit(36) <= "00000010110000000110000101111111"; -- store/write to uart
--  rom_unit(40) <= "00000111110000000111001100011100";--beq
--  rom_unit(44) <= "00000000001111000000000000000101";-- instruction after branch, delayed branch!!
--  rom_unit(52) <= "00000000000110000101000000000001"; -- branch is taken!
--  rom_unit(56) <= "00000010000110000101000000000000"; -- branch is taken!

-- stack cache check
--	rom_unit(0) <=  "00000011000000000000000000000100"; -- reserve 4 words
--	rom_unit(4) <= "00000010110000000000111110000111"; -- sws : sc[] <= r31
--  	rom_unit(8) <= "00000000000011000000000000000001"; -- 
--    rom_unit(12) <= "00000000000011100000000000000001"; -- 
--    rom_unit(16) <= "00000000000011100000000000000001"; -- 
--    rom_unit(20) <= "00000010100001000011000000000111"; -- lws: 1 <= sc[]
--    rom_unit(24) <= "00000011100000000000000000000100";
  
  --------------------------------------------
 -- 	rom_unit(0) <= "00000000000000100000000000000001";
--  	rom_unit(4) <= "00000000000001000000000001010000";
 -- 	rom_unit(8) <= "00000010100101000000000000000000"; -- load uart status
 -- 	rom_unit(12) <= "00000000001111000000000000000001";-- nop (we dont have nop yet so we just use an add instead)
 --   rom_unit(16) <= "00000010000101100001010100000111"; -- and
 --   rom_unit(20) <= "00000111110000001011000010010000"; -- benq 
 -- 	rom_unit(24) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	rom_unit(28) <= "00000010111111101110000100000001"; -- write in uart

  
 
 --  rom_unit(32) <= "00000000000110000101000001000001";
--   rom_unit(36) <= "00000010100101000000000000000000"; -- load uart status
  --	rom_unit(40) <= "00000000001111000000000000000001"; -- nop (we dont have nop yet so we just use an add instead)
  -- rom_unit(44) <= "00000010000101100001010100000111"; -- and
  -- rom_unit(48) <= "00000111110000001011000010010000"; -- benq 
 -- 	rom_unit(52) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	rom_unit(56) <= "00000010111111101110011000000001"; -- write in uart
	
	
--	rom_unit(60) <= "00000000000110000101000001010100";
 --  rom_unit(64) <= "00000010100101000000000000000000"; -- load uart status
  --	rom_unit(68) <= "00000000001111000000000000000001"; -- nop (we dont have nop yet so we just use an add instead)
 --  rom_unit(72) <= "00000010000101100001010100000111"; -- and
 --  rom_unit(76) <= "00000111110000001011000010010000"; -- benq 
 -- 	rom_unit(80) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	rom_unit(84) <= "00000010111111101110011000000001"; -- write in uart
	
--	rom_unit(88) <= "00000000000110000101000001001101";
--   rom_unit(92) <= "00000010100101000000000000000000"; -- load uart status
--  	rom_unit(96) <= "00000000001111000000000000000001"; -- nop (we dont have nop yet so we just use an add instead)
--   rom_unit(100) <= "00000010000101100001010100000111"; -- and
 --  rom_unit(104) <= "00000111110000001011000010010000"; -- benq 
  --	rom_unit(108) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	rom_unit(112) <= "00000010111111101110011000000001"; -- write in uart
	
	
--	rom_unit(116) <= "00000000000110000101000001001111";
 --  rom_unit(120) <= "00000010100101000000000000000000"; -- load uart status
  --	rom_unit(124) <= "00000000001111000000000000000001"; -- nop (we dont have nop yet so we just use an add instead)
  -- rom_unit(128) <= "00000010000101100001010100000111"; -- and
 --  rom_unit(132) <= "00000111110000001011000010010000"; -- benq 
  --	rom_unit(136) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	-rom_unit(140) <= "00000010111111101110011000000001"; -- write in uart
	
	
--	rom_unit(144) <= "00000000000110000101000001010011";
 --  rom_unit(148) <= "00000010100101000000000000000000"; -- load uart status
  --	rom_unit(152) <= "00000000001111000000000000000001"; -- nop (we dont have nop yet so we just use an add instead)
  -- rom_unit(156) <= "00000010000101100001010100000111"; -- and
 --  rom_unit(160) <= "00000111110000001011000010010000"; -- benq 
  --	rom_unit(164) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too 
  --	rom_unit(168) <= "00000010111111101110011000000001"; -- write in uart
	
  
 --	rom_unit(172) <= "00000000000000000000000000000001";
  --	rom_unit(176) <= "00000000000001100000000000000010";
  --	rom_unit(180) <= "00000111110000000000001100001000"; --
 
-------------------------------------------------- read from uart and echo

	rom_unit(0) <= "00000000000000100000000000000010"; -- add r1, ".00010"
	rom_unit(1) <= "00000010100101000101111110000000"; -- load
    rom_unit(2) <= "00000000000000000000000000000001"; --nop
    rom_unit(3) <= "00000010000101101010000010000111"; -- and
    rom_unit(4) <= "00000111110000001011000010000100"; --benq
    rom_unit(5) <= "00000000000000000000000000000001"; -- nop after branch
    rom_unit(6) <= "00000010100111100101111110000001"; -- load data from uart to r15
    rom_unit(7) <= "00000010100111100101111110000001"; -- load data from uart to r15
    
    --echo
     rom_unit(8) <= "00000000000001110011000000000001"; -- add r3 <= r19, ".00001"
   rom_unit(9) <= "00000000000001000000000001010000"; -- store whatever
   rom_unit(10) <= "00000010100101000101111110000000"; -- load uart status
   rom_unit(11) <= "00000000000000000000000000000001";-- nop (we dont have nop yet so we just use an add instead)
   rom_unit(12) <= "00000010000101101010000110000111"; -- and r11 <=r3, r10
   rom_unit(13) <= "00000111110000001011000110000100"; -- benq
   rom_unit(14) <= "00000000000000000000000000000001"; --instruction after branch -- this should be nop too
   rom_unit(15) <= "00000010111111101110011110000001"; -- write r15 in uart
    
   
  
 
  
  rom_unit(16) <= "00000000000000000000000000000001";
   rom_unit(17) <= "00000000000001100000000000000010";
   rom_unit(18) <= "00000111110000000000001100000010"; --
   rom_unit(19) <= "00000000000000000000000000000001";-- instruction after branch . . .



  
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




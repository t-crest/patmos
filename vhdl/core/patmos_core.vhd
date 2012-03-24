library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_core is 
  port
  (
    clk                   : in std_logic;
    rst                   : in std_logic
 --   fetch_din             : in fetch_in_type
  );
end entity patmos_core;

architecture arch of patmos_core is 

signal pc                              : unsigned(32 - 1 downto 0);
signal pc_next                         : unsigned(32 - 1 downto 0);
signal pc_offset                       : unsigned(32 - 1 downto 0);
signal mux_branch                      : unsigned(32 - 1 downto 0);
signal branch                          : std_logic := '0';
signal fetch_din                       : fetch_in_type;
signal fetch_dout                      : fetch_out_type;
signal decode_din                      : decode_in_type;
signal decode_dout                     : decode_out_type;

--------------------------------------------
begin
  fetch_din.pc <= pc_next;
  
  fet: entity work.patmos_fetch(arch)
	port map(clk, rst, fetch_din, fetch_dout);

  pc_adder: entity work.patmos_adder(arch)
  port map(pc, "00000000000000000000000000000100", pc_next);
  
  mux_pc: entity work.patmos_mux_32(arch)
  port map(pc_next, pc_offset, branch, mux_branch);
  
  pc_gen: entity work.patmos_pc_generator(arch)
  port map(clk, rst, mux_branch, pc);

  inst_mem: entity work.patmos_rom(arch)
  port map(pc, fetch_din.instruction);
--------------------------------------------------------
--	dec: entity work.patmos_decode(arch)
--	port map(clk, rst, decode_din, decode_dout);


  
end architecture arch;



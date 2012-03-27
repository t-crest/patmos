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
signal alu_din                         : alu_in_type;
signal alu_dout                        : alu_out_type;
signal mux_mem_reg                  	  : unsigned(31 downto 0); 
signal write_enable                    : std_logic;
signal mux_alu_src                     : unsigned(31 downto 0); 
signal alu_src1                        : unsigned(31 downto 0);
signal alu_src2                        : unsigned(31 downto 0);
signal fw_alu                          : unsigned(31 downto 0);
signal fw_mem                          : unsigned(31 downto 0);
signal fw_ctrl_rs1                     : forwarding_type;
signal fw_ctrl_rs2                     : forwarding_type;
signal alu_we                          : std_logic;
signal mem_we                          : std_logic;
signal alu_rd                          : unsigned(4 downto 0); 
signal mem_rd                          : unsigned(4 downto 0); 
------------------------------------------------------- fetch
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
-------------------------------------------------------- decode
--  pc_offset_adder: entity work.patmos_adder(arch) -- for branch instruction
--  port map(fetch_dout.pc, decode_dout.immediate, pc_offset);

  reg_file: entity work.patmos_register_file(arch)
	port map(clk, rst, fetch_dout.instruction(16 downto 12), fetch_dout.instruction(11 downto 7),
	         fetch_dout.instruction(21 downto 17), decode_din.rs1_data_in, decode_din.rs2_data_in,
	          mux_mem_reg, decode_dout.reg_write_out);
	 
  decode_din.operation <= fetch_dout.instruction;
	dec: entity work.patmos_decode(arch)
	port map(clk, rst, decode_din, decode_dout);


  ------------------------------------------------------ execute
  mux_imm: entity work.patmos_mux_32(arch) -- immediate or rt
  port map(decode_dout.rs2_data_out, decode_dout.ALUi_immediate_out, 
           decode_dout.alu_src_out, mux_alu_src);
  
  mux_rs1: entity work.patmos_forward_value(arch)
  port map(fw_alu, fw_mem, decode_dout.rs1_data_out, alu_src1, fw_ctrl_rs1);
  
  mux_rs2: entity work.patmos_forward_value(arch)
  port map(fw_alu, fw_mem, mux_alu_src, alu_src2, fw_ctrl_rs2);
  
  forward: entity work.patmos_forward(arch)
  port map(decode_dout.rs1_out, decode_dout.rs2_out, alu_we, mem_we, 
           alu_rd, mem_rd, fw_ctrl_rs1, fw_ctrl_rs2);
  
  alu_din.rs1 <= alu_src1;
  alu_din.rs2 <= alu_src2;
  alu_din.inst_type <= decode_dout.inst_type_out;
  alu_din.ALU_function_type <= decode_dout.ALU_function_type_out;
  
  
  alu: entity work.patmos_alu(arch)
  port map(clk, rst, alu_din, alu_dout);
  
  
  --execute: entity work.patmos_execute(arch)
  --port map();

end architecture arch;



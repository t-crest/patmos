library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_core is 
  port
  (
    clk                   : in std_logic;
    rst                   : in std_logic;
    fetch_din             : in fetch_in_type
  );
end entity patmos_core;

architecture arch of patmos_core is 


signal pc                              : unsigned(31 downto 0);
signal pc_next                         :  unsigned(31 downto 0);
signal rs                              : unsigned (4 downto 0);
signal rt                              : unsigned (4 downto 0);
signal rd                              : unsigned (4 downto 0);
signal func                            : unsigned (2 downto 0);
signal read_data1                      : unsigned(31 downto 0);
signal read_data2                      : unsigned(31 downto 0);
signal write_data                      : unsigned(31 downto 0);
signal write_data_mem                  : unsigned(31 downto 0);
signal write_data_in             : unsigned(31 downto 0);
signal operation1                      : unsigned(31 downto 0);
signal operation2                      : unsigned(31 downto 0);
signal write_enable                    : std_logic := '0';
signal inst_type                       : instruction_type;
signal ALUi_function_type              : unsigned(2 downto 0);
signal ALU_function_type               : unsigned(3 downto 0);
signal ALU_instruction_type            : ALU_inst_type;
signal ALUi_immediate                  : unsigned(11 downto 0);
signal inst_type_reg                   : instruction_type;
signal ALUi_function_type_reg          : unsigned(2 downto 0);
signal ALU_function_type_reg           : unsigned(3 downto 0);
signal ALU_instruction_type_reg        : ALU_inst_type;
signal ALUi_immediate_reg              : unsigned(11 downto 0);
signal pc_ctrl_gen                     : pc_type;
signal multiplexer_a_ctrl              : std_logic;
signal multiplexer_b_ctrl              : std_logic;
signal alu_we                          : std_logic;
signal wb_we                           : std_logic;
signal rs1                             : unsigned (4 downto 0);
signal rs2                             : unsigned (4 downto 0);
signal rs1_fw                             : unsigned (4 downto 0);
signal rs2_fw                             : unsigned (4 downto 0);
signal predicate_bit                   : std_logic;
signal pd                              : unsigned (2 downto 0);
signal wb_we_out_exec                  : std_logic;
signal rd_ex                           : unsigned (4 downto 0);
signal rd_ex_in               	        : unsigned (4 downto 0); 
signal write_address_reg_file          : unsigned (4 downto 0);
signal write_address_mem               : unsigned (4 downto 0);
signal predicate                       : unsigned(2 downto 0);
signal mem_we                          : std_logic;
signal mem_wr_rn                       : unsigned(4 downto 0); 
signal mux_fw_rs                       : forwarding_type;
signal mux_fw_rt                       : forwarding_type;
signal write_enable_mem_stage          : std_logic;
signal fw_mem                          : unsigned(31 downto 0);
signal wb_we_exec                      : std_logic;
signal wb_we_exec_out                  : std_logic;
-- signal fetch_din                       : fetch_in_type;
signal fetch_dout                      : fetch_out_type;
signal decode_din                      : decode_in_type;
signal decode_dout                     : decode_out_type;
signal ld_type                         : load_type;
signal load_immediate                  : unsigned(6 downto 0);
signal ld_function_type                : unsigned(1 downto 0);
signal load_address                    : unsigned(31 downto 0); 
--------------------------------------------
begin
  decode_din.operation1 <= fetch_dout.operation1;
  fet: entity work.patmos_fetch(arch)
	port map(clk, rst, fetch_din, fetch_dout);
  

	dec: entity work.patmos_decode(arch)
	port map(clk, rst, decode_din, decode_dout);


----------------------------------------------------

	reg_file: entity work.patmos_register_file(arch)
	port map(clk, rst, decode_dout.rs1, decode_dout.rs2, write_address_reg_file, read_data1, read_data2, write_data, write_enable);
	
	uut_rs1: entity work.patmos_clock_input(arch)
	generic map(5)
	port map(clk, decode_dout.rs1, rs1_fw);
  uut_rs2: entity work.patmos_clock_input(arch)
	generic map(5)
	port map(clk, decode_dout.rs2, rs2_fw);
  

  exec: entity work.patmos_execute(arch)
	port map(clk, inst_type_reg, ALU_function_type_reg, 
	         ALU_instruction_type_reg, ALUi_immediate_reg, 
	         read_data1, read_data2, predicate, write_data_in,
	         decode_dout.wb_we, wb_we_exec, write_data_in, write_data, 
	         mux_fw_rs, mux_fw_rt, ld_type, load_immediate, ld_function_type, load_address);

  forward: entity work.patmos_forward(arch)
 	port map(rs1_fw, rs2_fw, wb_we_exec_out, write_enable, rd_ex, write_address_reg_file, mux_fw_rs, mux_fw_rt);

   -- rs                     : in unsigned(4 downto 0); -- register exec reads
   -- rt                     : in unsigned(4 downto 0); -- register exec reads
   -- alu_we                 : in std_logic;
   -- mem_we                 : in std_logic;
   -- alu_wr_rn              : in unsigned(4 downto 0);
   -- mem_wr_rn              : in unsigned(4 downto 0);
   -- mux_fw_rs              : out forwarding_type;
   -- mux_fw_rt              : out forwarding_type
----------------------------------------------------
-- clock decode stage to exec stage
	uut_inst_type: entity work.clock_inst_type(arch)
	port map(clk, decode_dout.inst_type, inst_type_reg);
	
  uut_function_type: entity work.patmos_clock_input(arch)
  generic map(4)
	port map(clk, decode_dout.ALU_function_type, ALU_function_type_reg);
	
	uut_ALU_instruction_type: entity work.clock_ALU_instruction_type(arch)
	port map(clk, decode_dout.ALU_instruction_type, ALU_instruction_type_reg);
	
	uut_ALUi_immediate: entity work.patmos_clock_input(arch)
  generic map(12)
	port map(clk, decode_dout.ALUi_immediate, ALUi_immediate_reg);
	
	------------------------------
	uut_write_address_exec1: entity work.clock_input(arch)
	generic map(5)
	port map(clk, decode_dout.rd, rd_ex_in);
	uut_write_address_exec2: entity work.clock_input(arch)
	generic map(5)
	port map(clk, rd_ex_in, rd_ex);
	
	uut_we: entity work.clock_we(arch)
	port map(clk, wb_we_exec, wb_we_exec_out);
	
	uut_ex_mem: entity work.patmos_mem_stage(arch)
	port map(clk, rd_ex, write_data_in, wb_we_exec_out, write_address_reg_file, write_data, write_enable);
  --clk, write_register_in, write_data_in, write_enable_in, write_register_out, write_data_out, write_enable_out       
  -------------------------------
  
end architecture arch;



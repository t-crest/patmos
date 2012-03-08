library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_testbench is 
end entity patmos_testbench;

architecture timed of patmos_testbench is 
signal clk : std_logic := '0';--   
signal rst : std_logic := '1';--   
signal instruction_word             : unsigned(31 downto 0);
signal pc : unsigned(31 downto 0);
signal pc_next :  unsigned(31 downto 0);
signal rs                              : unsigned (4 downto 0);
signal rt                              : unsigned (4 downto 0);
signal rd                              : unsigned (4 downto 0);
signal func                            : unsigned (2 downto 0);
signal read_data1                      : unsigned(31 downto 0);
signal read_data2                      : unsigned(31 downto 0);
signal write_data                      : unsigned(31 downto 0);
signal write_data_exec_out             : unsigned(31 downto 0);
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
signal predicate_bit                   : std_logic;
signal pd                              : unsigned (2 downto 0);
signal wb_we_out_exec                  : std_logic;
signal wa1                             : unsigned (4 downto 0);
signal wa2                             : unsigned (4 downto 0);
signal write_address_reg_file          : unsigned (4 downto 0);
signal predicate                       : unsigned(2 downto 0);
signal mem_we                          : std_logic;
signal mem_wr_rn                       : unsigned(4 downto 0); 
signal mux_fw_rs                       : forwarding_type;
signal mux_fw_rt                       : forwarding_type;
signal write_enable_mem_stage          : std_logic;
signal fw_mem                          : unsigned(31 downto 0);
--------------------------------------------
begin

  fet: entity work.patmos_fetch(arch)
	port map(clk, rst, instruction_word, pc, pc_ctrl_gen, operation1, '1', "0000000000000000001000");
  clk <= not clk after 5 ns;

	dec: entity work.patmos_decode(arch)
	port map(clk, rst, predicate_bit, inst_type, ALU_function_type, ALU_instruction_type, ALUi_immediate, 
	pc_ctrl_gen, multiplexer_a_ctrl, multiplexer_b_ctrl, alu_we, wb_we, operation1, rs1, rs2, rd, pd);

----------------------------------------------------
-- clock decode stage to exec stage
	uut_inst_type: entity work.clock_inst_type(arch)
	port map(clk, inst_type, inst_type_reg);
	
  uut_function_type: entity work.patmos_clock_input(arch)
  generic map(4)
	port map(clk, ALU_function_type, ALU_function_type_reg);
	
	uut_ALU_instruction_type: entity work.clock_ALU_instruction_type(arch)
	port map(clk, ALU_instruction_type, ALU_instruction_type_reg);
	
	uut_ALUi_immediate: entity work.patmos_clock_input(arch)
  generic map(12)
	port map(clk, ALUi_immediate, ALUi_immediate_reg);
	
	uut_we1: entity work.clock_we(arch)
	port map(clk, wb_we_out_exec, write_enable_mem_stage);
	
	uut_we2: entity work.clock_we(arch)
	port map(clk, write_enable_mem_stage, write_enable);
	
	-- write register address
	uut_wa1: entity work.patmos_clock_input(arch)
	generic map(5)
	port map(clk, rd, wa1);
	
	
	uut_wa2: entity work.patmos_clock_input(arch)
	generic map(5)
	port map(clk, wa1, wa2);
	
  uut_wa3: entity work.patmos_clock_input(arch)
	generic map(5)
	port map(clk, wa2, write_address_reg_file);
	
	-- clock write data through mem stage!
	uut_clock_write_data_to_wb: entity work.patmos_clock_input(arch)
	generic map(32)
	port map(clk, write_data_exec_out, write_data);
	

	
----------------------------------------------------

	reg_file: entity work.patmos_register_file(arch)
	port map(clk, rst, rs1, rs2, write_address_reg_file, read_data1, read_data2, write_data, write_enable);
	

  exec: entity work.patmos_execute(arch)
	port map(clk, inst_type_reg, ALU_function_type_reg, 
	         ALU_instruction_type_reg, ALUi_immediate_reg, 
	         read_data1, read_data2, predicate, write_data_exec_out,
	         wb_we, wb_we_out_exec, write_data_exec_out, fw_mem, mux_fw_rs, mux_fw_rt);
	         
       
	
  forward: entity work.patmos_forward(arch)
 	port map(rs1, rs2, wb_we, mem_we, rd, mem_wr_rn, mux_fw_rs, mux_fw_rt);

	
--	entity patmos_forward is
--  port
 -- (
 --   rs                     : in unsigned(4 downto 0); -- register exec reads
--    rt                     : in unsigned(4 downto 0); -- register exec reads
--    alu_we                 : in std_logic;
--    mem_we                 : in std_logic;
--    alu_wr_rn              : in unsigned(4 downto 0);
--    mem_wr_rn              : in unsigned(4 downto 0);
--    mux_fw_rs              : out forwarding_type;
--    mux_fw_rt              : out forwarding_type
-- );

clk <= not clk after 5 ns;
              --    "xpred00fff4321043210109876543210"
instruction_word <= "00000000000000100000000000000001" after 5 ns,  -- r1 <= r0 + 1 add immediate
                    "00000000000001000000000000000010" after 15 ns, -- r2 <= r0 + 2 add immediate
                    "00000010000001100010000010000000" after 25 ns;--, -- r3 <= r1 + r2 add register -- read after write
                 --   "00000000001000000000000010000001" after 35 ns, -- r16 <= r1 + 128 add immediate 
                 --   "00000010000010010000000000010000" after 45 ns; -- r4 < int8_t(r16) unary 
                    
-- pc <= (others => '0');
rst <= '0' after 4 ns;

end architecture timed;

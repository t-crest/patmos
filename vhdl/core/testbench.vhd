library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.type_package.all;

entity pc_tester is 
end entity;

architecture timed of pc_tester is 
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
signal operation1                      : unsigned(31 downto 0);
signal operation2                      : unsigned(31 downto 0);
signal write_enable                    : std_logic := '0';
signal inst_type                       : instruction_type;
signal ALUi_function_type              : unsigned(2 downto 0);
signal ALU_function_type               : unsigned(3 downto 0);
signal ALU_instruction_type            : ALU_inst_type;
signal ALUi_immediate                  : unsigned(11 downto 0);
signal inst_type_reg                       : instruction_type;
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
signal pd                              :unsigned (2 downto 0);
--------------------------------------------
begin

  fet: entity work.patmos_fetch(arch)
	port map(clk, rst, instruction_word, pc, PCNext, operation1, '1', "0000000000000000001000");
  clk <= not clk after 5 ns;

	dec: entity work.patmos_decode(arch)
	port map(clk, rst, predicate_bit, inst_type, ALU_function_type, ALU_instruction_type, ALUi_immediate, 
	pc_ctrl_gen, multiplexer_a_ctrl, multiplexer_b_ctrl, alu_we, wb_we, operation1, rs1, rs2, rd, pd);

----------------------------------------------------
-- clock decode stage to exec stage
	uut_inst_type: entity work.clock_inst_type(arch)
	port map(clk, inst_type, inst_type_reg);
	
  uut_function_type: entity work.clock_in(arch)
  generic map(4)
	port map(clk, ALU_function_type, ALU_function_type_reg);
	
	uut_ALU_instruction_type: entity work.clock_ALU_instruction_type(arch)
	port map(clk, ALU_instruction_type, ALU_instruction_type_reg);
	
	uut_ALUi_immediate: entity work.clock_in(arch)
  generic map(12)
	port map(clk, ALUi_immediate, ALUi_immediate_reg);
	
----------------------------------------------------

	reg_file: entity work.patmos_register_file(arch)
	port map(clk, rst, rs1, rs2, rd, read_data1, read_data2, write_data, write_enable);
	

  exec: entity work.patmos_execute(arch)
	port map(clk, inst_type_reg, ALU_function_type_reg, ALU_instruction_type_reg, ALUi_immediate_reg, read_data1, read_data2, write_data);
	
	
	

clk <= not clk after 5 ns;
                
instruction_word <= "00000000000000100000000000000001" after 5 ns;--,-- r1 <= r0 + 1 add immediate
                  --  "00000000010001000000000000000010" after 15 ns,-- r2 <= r0 + 2 add immediate
                   -- "00000010000001100010000010000000" after 25 ns; -- r3 <= r1 + r2 add register
                    
-- pc <= (others => '0');
rst <= '0' after 4 ns;

end architecture timed;

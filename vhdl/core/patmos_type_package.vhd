library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package patmos_type_package is
  
  
  type instruction_type is (ALUi, ALU, NOP, SPC, LDT, STT);
  type pc_type is (PCNext, PCBranch);
  type ALU_inst_type is (ALUr, ALUu, ALUm, ALUc, ALUp);
  type SPC_type is (SPCn, SPCw, SPCt, SPCf);
  signal predicate_register_bank : unsigned(7 downto 0);
  type forwarding_type is (FWNOP, FWMEM, FWALU);
  type load_type is (lw, lh, lb, lhu, lbu, dlwh, dlbh, dlbu);

-------------------------------------------
-- in/out records
-------------------------------------------
constant        pc_length                   : integer := 32;
constant        instruction_word_length     : integer := 32;
-------------------------------------------
-- fetch
-------------------------------------------
type fetch_in_type is record
    instruction_word            :  unsigned(instruction_word_length - 1 downto 0); 
    pc_ctrl                     :  pc_type;
    predicate                   :  std_logic;
    immediate                   :  unsigned(21 downto 0);
end record;
type fetch_out_type is record
		pc                          :  unsigned(pc_length - 1 downto 0);
		operation1                  :  unsigned(31 downto 0);
end record;

--------------------------------------------
-- decode
--------------------------------------------
type decode_in_type is record
    operation1                      : unsigned (31 downto 0);
end record;
type decode_out_type is record
		predicate_bit                   : std_logic;
    inst_type                       : instruction_type;
    ALU_function_type               : unsigned(3 downto 0);
    ALU_instruction_type            : ALU_inst_type;
    ALUi_immediate                  : unsigned(11 downto 0);
    pc_ctrl_gen                     : pc_type;
    multiplexer_a_ctrl              : std_logic;
    multiplexer_b_ctrl              : std_logic;
    alu_we                          : std_logic;
    wb_we                           : std_logic;
    rs1                             : unsigned (4 downto 0);
    rs2                             : unsigned (4 downto 0);
    rd                              : unsigned (4 downto 0);
    pd                              : unsigned (2 downto 0);
    sd                              : unsigned (3 downto 0);
    ss                              : unsigned (3 downto 0);
    ld_type                         : load_type;
    load_immediate                  : unsigned(6 downto 0);
    ld_function_type                : unsigned(1 downto 0);
end record;
       



end patmos_type_package;
     


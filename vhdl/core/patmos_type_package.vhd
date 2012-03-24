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
-- fetch/decode
-------------------------------------------
type fetch_in_type is record
    instruction                   :  unsigned(instruction_word_length - 1 downto 0); 
    pc                            :  unsigned(pc_length - 1 downto 0);
end record;
type fetch_out_type is record
		pc                            :  unsigned(pc_length - 1 downto 0);
		instruction                   :  unsigned(31 downto 0);
end record;

--------------------------------------------
-- decode/exec
--------------------------------------------
type decode_in_type is record
    --operation1                         : unsigned (31 downto 0);
  		predicate_bit_in                   : std_logic;
    inst_type_in                       : instruction_type;
    ALU_function_type_in               : unsigned(3 downto 0);
    ALU_instruction_type_in            : ALU_inst_type;
    ALUi_immediate_in                  : unsigned(11 downto 0);
    pc_ctrl_gen_in                     : pc_type;
    wb_we_in                           : std_logic;
    rs1_in                             : unsigned (4 downto 0);
    rs2_in                             : unsigned (4 downto 0);
    rd_in                              : unsigned (4 downto 0);
    pd_in                              : unsigned (2 downto 0);
    sd_in                              : unsigned (3 downto 0);
    ss_in                              : unsigned (3 downto 0);
    ld_type_in                         : load_type;
    load_immediate_in                  : unsigned(6 downto 0);
    ld_function_type_in                : unsigned(1 downto 0);
end record;
type decode_out_type is record
		predicate_bit_out                   : std_logic;
    inst_type_out                       : instruction_type;
    ALU_function_type_out               : unsigned(3 downto 0);
    ALU_instruction_type_out            : ALU_inst_type;
    ALUi_immediate_out                  : unsigned(11 downto 0);
    pc_ctrl_gen_out                     : pc_type;
    wb_we_out                           : std_logic;
    rs1_out                             : unsigned (4 downto 0);
    rs2_out                             : unsigned (4 downto 0);
    rd_out                              : unsigned (4 downto 0);
    pd_out                              : unsigned (2 downto 0);
    sd_out                              : unsigned (3 downto 0);
    ss_out                              : unsigned (3 downto 0);
    ld_type_out                         : load_type;
    load_immediate_out                  : unsigned(6 downto 0);
    ld_function_type_out                : unsigned(1 downto 0);
end record;
       
-------------------------------------------
-- execution
-------------------------------------------
--type execution_in_type is record
    
--end record;

--type execution_out_type is record
--end record;



end patmos_type_package;
     


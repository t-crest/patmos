library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package patmos_type_package is
  
  
  type instruction_type is (NONE, ALUi, ALU, NOP, SPC, LDT, STT, STC, BC);
  type STC_instruction_type is (NONE, SRES, SENS, SFREE);
  type pc_type is (PCNext, PCBranch);
  type ALU_inst_type is (NONE, ALUr, ALUu, ALUm, ALUc, ALUp);
  type STT_inst_type is (NONE, SWS, SWL, SWC, SWM, SHS, SHL, SHC, SHM, SBS, SBL, SBC, SBM); -- all stores
  type LDT_inst_type is (NONE, LWM, LWS, LHS, LBS, LHUS, LBUS); -- these are just stack cache loads
  type SPC_type is (NONE, SPCn, SPCw, SPCt, SPCf);
  type forwarding_type is (FWNOP, FWMEM, FWALU);
  type load_type is (NONE, lw, lh, lb, lhu, lbu, dlwh, dlbh, dlbu);

-------------------------------------------
-- in/out records
-------------------------------------------
constant        pc_length                   : integer := 32;
constant        instruction_word_length     : integer := 32;
-------------------------------------------
-- fetch/decode
-------------------------------------------
type fetch_in_type is record
    pc                            :  unsigned(pc_length - 1 downto 0);
end record;
type fetch_out_type is record
		pc                            :  unsigned(pc_length - 1 downto 0);
		-- TODO: why is this unsigned?
		instruction                   :  unsigned(31 downto 0);
end record;

--------------------------------------------
-- decode/exec
--------------------------------------------
type decode_in_type is record
    operation                          : unsigned (31 downto 0);
	rs1_data_in                        : unsigned (31 downto 0);
    rs2_data_in                        : unsigned (31 downto 0);
    rs1_in                             : unsigned (4 downto 0);
    rs2_in                             : unsigned (4 downto 0);
 --   reg_write_in                       : std_logic;
    alu_src_in                         : std_logic;  
    rs1_data_in_special				   : unsigned(31 downto 0);   
    rs2_data_in_special				   : unsigned(31 downto 0);
    predicate_bit_in					: std_logic;
  	predicate_data_in					: unsigned (7 downto 0);  
    head_in							   : unsigned(4 downto 0);
    tail_in							   : unsigned(4 downto 0);     
    stack_data_in   					: unsigned (31 downto 0);
end record;
type decode_out_type is record

	imm : std_logic_vector(31 downto 0);
	
	predicate_bit_out                   : std_logic;
	predicate_data_out					: unsigned (7 downto 0);  
	predicate_condition					: unsigned(2 downto 0);
	ps1_out 							:unsigned(3 downto 0);
    ps2_out 							:unsigned(3 downto 0);
 --   pd_out 								:unsigned(3 downto 0);
    inst_type_out                       : instruction_type;
    ALU_function_type_out               : unsigned(3 downto 0);
    ALU_instruction_type_out            : ALU_inst_type;
    ALUi_immediate_out                  : unsigned(31 downto 0);
    pc_ctrl_gen_out                     : pc_type;
  --  wb_we_out                           : std_logic;
    rs1_out                             : unsigned (4 downto 0);
    rs2_out                             : unsigned (4 downto 0);
    rd_out                              : unsigned (4 downto 0);
    rs1_data_out                        : unsigned (31 downto 0);
    rs2_data_out                        : unsigned (31 downto 0);
    pd_out                              : unsigned (3 downto 0);
    sd_out                              : unsigned (3 downto 0);
    ss_out                              : unsigned (3 downto 0);
    ld_type_out                         : load_type;
 --   load_immediate_out                  : unsigned(6 downto 0);
    ld_function_type_out                : unsigned(1 downto 0);
    ps_reg_write_out					: std_logic;
    reg_write_out                       : std_logic;
    predicate_reg_write_out				: std_logic;
    alu_src_out                         : std_logic; -- 0 for ALUi/ 1 for ALU
    mem_to_reg_out                      : std_logic; -- data to register file comes from alu or mem? 0 for alu and 1 for mem
    mem_read_out                        : std_logic;
    mem_write_out                       : std_logic;
    st_out							    : unsigned(3 downto 0);
    stack_data_out   					: unsigned (31 downto 0);
    STC_instruction_type_out			: STC_instruction_type;
    head_out							 : unsigned(4 downto 0);  
    tail_out							 : unsigned(4 downto 0);  
    stc_immediate_out					: unsigned (4 downto 0);
    rs1_data_out_special				: unsigned (31 downto 0);
    rs2_data_out_special			   : unsigned(31 downto 0);  
    sc_write_out						: std_logic;
    sc_read_out 						: std_logic;
    STT_instruction_type_out			: STT_inst_type;
    LDT_instruction_type_out			: LDT_inst_type;
end record;
       
-------------------------------------------
-- execution
-------------------------------------------
type execution_in_type is record
    reg_write_in                        : std_logic;
    ps_reg_write_in                     : std_logic;
    predicate_reg_write_in              : std_logic;
    predicate_condition				    : unsigned(2 downto 0);
    mem_read_in                         : std_logic;
    mem_write_in                        : std_logic;
    mem_to_reg_in                       : std_logic;
    alu_result_in                       : unsigned(31 downto 0);
    alu_result_predicate_in				: std_logic;
    mem_write_data_in       	 	    : unsigned(31 downto 0);
    write_back_reg_in                   : unsigned(4 downto 0);
    ps_write_back_reg_in		        : unsigned(2 downto 0);
    tail_in								: unsigned(4 downto 0);
    head_in								: unsigned(4 downto 0);
    st_in								: unsigned(31 downto 0);
    STT_instruction_type_in				: STT_inst_type;
    LDT_instruction_type_in				: LDT_inst_type;
    predicate_bit_in                    : std_logic;
	predicate_data_in					: unsigned(7 downto 0); 
end record;

type execution_out_type is record
    alu_result_out                      : unsigned(31 downto 0);
    alu_result_predicate_out			: std_logic;
    reg_write_out                     	 : std_logic;
    ps_reg_write_out                     : std_logic;
    predicate_reg_write_out              : std_logic;
    mem_read_out                        : std_logic;
    mem_write_out                       : std_logic;
    mem_to_reg_out                      : std_logic;
    mem_write_data_out          	    : unsigned(31 downto 0);
    write_back_reg_out                  : unsigned(4 downto 0);
    ps_write_back_reg_out		        : unsigned(2 downto 0);
    tail_out							: unsigned(4 downto 0);
    head_out							: unsigned(4 downto 0);
    st_out								: unsigned(31 downto 0);
    STT_instruction_type_out			: STT_inst_type;
    LDT_instruction_type_out			: LDT_inst_type;
    predicate_bit_out                   : std_logic;
	predicate_data_out					: unsigned(7 downto 0);
end record;

------------------------------------------
-- control
------------------------------------------
type alu_in_type is record
   rs1                         : unsigned(31 downto 0);
   rs2                         : unsigned(31 downto 0);
   ps1						   : std_logic;
   ps2						   : std_logic;
   ps1_negate					: std_logic;
   ps2_negate				   : std_logic;
   inst_type                   : instruction_type; 
   ALU_function_type           : unsigned(3 downto 0);
   ALU_instruction_type        : ALU_inst_type;
   stack_data_in			   : unsigned(31 downto 0);
 --  head_in						: unsigned(4 downto 0);
   STC_instruction_type			:STC_instruction_type;
  -- tail_in						: unsigned(4 downto 0);
   stc_immediate_in				: unsigned (4 downto 0);
   st_in						: unsigned (31 downto 0);
   STT_instruction_type			: STT_inst_type;
   LDT_instruction_type			: LDT_inst_type;				
end record;

type alu_out_type is record
  rd                          : unsigned(31 downto 0);
  pd						  : std_logic;
 -- head_out 		     			: unsigned(4 downto 0);
 -- tail_out					 : unsigned(4 downto 0);
  spill_out					: std_logic;
  fill_out					 : std_logic;
  st_out						: unsigned (31 downto 0);
end record;

------------------------------------------
-- mem
------------------------------------------
type mem_in_type is record
    reg_write_in                 : std_logic;
    ps_reg_write_in             : std_logic;
    predicate_reg_write_in       : std_logic;
    mem_to_reg_in                : std_logic;
    mem_data_in                  : unsigned(31 downto 0); 
    alu_result_in                : unsigned(31 downto 0);
    alu_result_predicate_in		 : std_logic;
    write_back_reg_in            : unsigned(4 downto 0); 
    ps_write_back_reg_in		  : unsigned(2 downto 0);
    mem_write_data_in            : unsigned(31 downto 0);
end record;

type mem_out_type is record
    reg_write_out                 : std_logic;
    ps_reg_write_out                 : std_logic;
    predicate_reg_write_out       : std_logic;
    mem_to_reg_out                : std_logic;
    mem_data_out                  : unsigned(31 downto 0); 
    alu_result_out                : unsigned(31 downto 0);
    alu_result_predicate_out	  : std_logic;
    write_back_reg_out            : unsigned(4 downto 0); 
    ps_write_back_reg_out		  : unsigned(2 downto 0);
    mem_write_data_out            : unsigned(31 downto 0);
end record;
     
type patmos_stack_cache_ctrl_in is record
		stc_immediate_in	: unsigned(4 downto 0);
		instruction			: STC_instruction_type; -- from decode
		st_in				: unsigned(31 downto 0);
end record;

type patmos_stack_cache_ctrl_out is record
		stall				: std_logic;
		head_tail			: unsigned(4 downto 0); -- connect to stack cache
		spill_fill     	 	: std_logic;
		st_out				: unsigned(31 downto 0);
		reg_write_out		: std_logic;
end record;

type patmos_stack_cache_in is record
	head_tail					: unsigned(4 downto 0);
	din_from_mem				: unsigned(31 downto 0); -- mem interface
	din_from_cpu				: unsigned(31 downto 0);
	spill_fill	        	    : std_logic;
    write_enable          	    : std_logic;
    address			 			: unsigned(4 downto 0);
end record;

type patmos_stack_cache_out is record
	dout_to_mem					: unsigned(31 downto 0); -- mem interface
	dout_to_cpu					: unsigned(31 downto 0);
end record;

type instruction_memory_in_type is record
    address               : unsigned(31 downto 0);
    inst_in               : unsigned(31 downto 0); -- from boot loader
    read_enable           : std_logic;
    write_enable          : std_logic;
end record;

type instruction_memory_out_type is record
	inst_out              : unsigned(31 downto 0); -- to fetch
end record;

type instruction_rom_in_type is record
	address 			  :  unsigned(7 downto 0);
end record;

type instruction_rom_out_type is record
	q 					  :  unsigned(31 downto 0);
end record;

end patmos_type_package;



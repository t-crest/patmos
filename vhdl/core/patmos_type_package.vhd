-- 
-- Copyright Technical University of Denmark. All rights reserved.
-- This file is part of the time-predictable VLIW Patmos.
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
-- 
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
-- 
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
-- 
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
-- NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-- 
-- The views and conclusions contained in the software and documentation are
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holder.
-- 


--------------------------------------------------------------------------------
-- Short descripton.
--
-- Author: Sahar Abbaspour
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------


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
	rs1_data_in                        : std_logic_vector (31 downto 0);
    rs2_data_in                        : std_logic_vector (31 downto 0);
end record;
type decode_out_type is record

	imm : std_logic_vector(31 downto 0);
	instr_cmp : std_logic;
	
	predicate_bit_out                   : std_logic;
	predicate_condition					: unsigned(2 downto 0);
	ps1_out 							:unsigned(3 downto 0);
    ps2_out 							:unsigned(3 downto 0);
    inst_type_out                       : instruction_type;
    ALU_function_type_out               : unsigned(3 downto 0);
    ALU_instruction_type_out            : ALU_inst_type;
    ALUi_immediate_out                  : unsigned(31 downto 0);
    pc_ctrl_gen_out                     : pc_type;
    rs1_out                             : unsigned (4 downto 0);
    rs2_out                             : unsigned (4 downto 0);
    rd_out                              : unsigned (4 downto 0);
    rs1_data_out                        : unsigned(31 downto 0);
    rs2_data_out                        : unsigned(31 downto 0);
    pd_out                              : unsigned (3 downto 0);
    ld_type_out                         : load_type;
    reg_write_out                       : std_logic;
    alu_src_out                         : std_logic; -- 0 for ALUi/ 1 for ALU
    mem_to_reg_out                      : std_logic; -- data to register file comes from alu or mem? 0 for alu and 1 for mem
    mem_read_out                        : std_logic;
    mem_write_out                       : std_logic;
    st_out							    : unsigned(3 downto 0);
    STC_instruction_type_out			: STC_instruction_type;
    stc_immediate_out					: unsigned (4 downto 0);
    sc_write_out						: std_logic;
    sc_read_out 						: std_logic;
    STT_instruction_type_out			: STT_inst_type;
    LDT_instruction_type_out			: LDT_inst_type;
end record;
       
       
type result_type is record
	value   : std_logic_vector(31 downto 0);
	reg_nr  : std_logic_vector(4 downto 0);
	valid   : std_logic; 
end record;

-------------------------------------------
-- execution
-------------------------------------------

type execution_out_type is record
	predicate : std_logic_vector(7 downto 0);
	result  : result_type;
    alu_result_out                      : unsigned(31 downto 0);
    reg_write_out                     	 : std_logic;
    mem_read_out                        : std_logic;
    -- two write back enable signals - shall be merged
    mem_write_out                       : std_logic;
    mem_to_reg_out                      : std_logic;
    mem_write_data_out          	    : unsigned(31 downto 0);
    write_back_reg_out                  : unsigned(4 downto 0);
    ps_write_back_reg_out		        : unsigned(2 downto 0);
    STT_instruction_type_out			: STT_inst_type;
    LDT_instruction_type_out			: LDT_inst_type;
end record;

------------------------------------------
-- control
------------------------------------------
type alu_in_type is record
   rs1                         : unsigned(31 downto 0);
   rs2                         : unsigned(31 downto 0);
   inst_type                   : instruction_type; 
   ALU_function_type           : unsigned(3 downto 0);
   ALU_instruction_type        : ALU_inst_type;
--   stack_data_in			   : unsigned(31 downto 0);
   STC_instruction_type			:STC_instruction_type;
--   stc_immediate_in				: unsigned (4 downto 0);
--   st_in						: unsigned (31 downto 0);
   STT_instruction_type			: STT_inst_type;
   LDT_instruction_type			: LDT_inst_type;
   mem_write_data_in       	 	    : unsigned(31 downto 0);
   			
end record;


------------------------------------------
-- mem
------------------------------------------
type mem_in_type is record
    data_in                  : std_logic_vector(31 downto 0); 
    -- following is forwarding 
    reg_write_in                : std_logic;
    write_back_reg_in                  : unsigned(4 downto 0);
    mem_write_data_in            : unsigned(31 downto 0);
end record;

type mem_out_type is record
	result  : result_type;
    data_out                  : std_logic_vector(31 downto 0);
    -- following is forwarding 
    reg_write_out                : std_logic;
    write_back_reg_out                  : unsigned(4 downto 0);
    mem_write_data_out            : std_logic_vector(31 downto 0);
end record;
     
type patmos_stack_cache_ctrl_in is record
		stc_immediate_in	: unsigned(4 downto 0);
		instruction			: STC_instruction_type; -- from decode
--		st_in				: unsigned(31 downto 0);
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

type write_back_in_out_type is record
	write_reg 			  : unsigned(4 downto 0);
	write_enable		  : std_logic;
	write_value			  : unsigned(31 downto 0);	
end record;

end patmos_type_package;



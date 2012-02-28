library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package type_package is
  type instruction_type is (ALUi, ALU, NOP, MoveToSpecial, WaitInst, MovefromSpecial, LoadTyped);
  type pc_type is (PCNext, PCBranch);
  type ALU_inst_type is (ALUr, ALUu, ALUm, ALUc, ALUp);
  signal predicate_register_bank : unsigned(7 downto 0);
  type forwarding_type is (FWMEM, FWALU, FWNOP);
end type_package;




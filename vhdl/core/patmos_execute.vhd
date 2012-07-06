--TO DO: 
-- replcae pd with predicate_reg(pd) (number of predicate register that should be written with 0 or 1)


 -----------------------------------
 -- execution
 -----------------------------------

library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use work.patmos_type_package.all;


entity patmos_execute is
  port
  (
    clk                             : in std_logic;
    rst                             : in std_logic;
    din                       	     : in execution_in_type;
    dout                            : out execution_out_type
  );

end entity patmos_execute;

architecture arch of patmos_execute is
--signal test : std_logic := '0';
begin

  process(clk)
  begin
    if rising_edge(clk) then
      if (din.predicate_data_in(to_integer(din.predicate_condition)) /= din.predicate_bit_in ) then
      	dout.mem_read_out <= din.mem_read_in;
      	dout.mem_write_out <= din.mem_write_in;
      	dout.reg_write_out <= din.reg_write_in;
      	dout.ps_reg_write_out <= din.ps_reg_write_in;
      --	test <= '1';
      else
      	dout.mem_read_out <= '0';
      	dout.mem_write_out <= '0';
     	dout.reg_write_out <= '0';
     	dout.ps_reg_write_out <= '0';
      end if;
      
      dout.mem_to_reg_out <= din.mem_to_reg_in;
      dout.alu_result_out <= din.alu_result_in;
      dout.mem_write_data_out <= din.mem_write_data_in;
      dout.write_back_reg_out <= din.write_back_reg_in;
      dout.ps_write_back_reg_out <= din.ps_write_back_reg_in;
      dout.head_out <= din.head_in;
      dout.tail_out <= din.tail_in;
      dout.STT_instruction_type_out <= din.STT_instruction_type_in;
      dout.LDT_instruction_type_out <= din.LDT_instruction_type_in;
      dout.alu_result_predicate_out <= din.alu_result_predicate_in;
    end if; 
  end process;
end arch;




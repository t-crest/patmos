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

begin

  process(clk)
  begin
    if rising_edge(clk) then
      dout.reg_write_out <= din.reg_write_in;
    end if; 
  end process;
end arch;


EX_MEM EX_MEM(
	.clk				(clk),
	.rst				(rst_n),
	.RegWrite_in		(RegWrite_EX),	// WB
	.MemtoReg_in		(MemtoReg_EX),	// WB
	.MemRead_in			(MemRead_EX),		// M
	.MemWrite_in		(MemWrite_EX),	// M
	.ALUData_in			(ALUResult_EX),
	.MemWriteData_in	(muxB_ALUsrc),
	.WBregister_in		(mux_RegDST_EX),
	.RegWrite_out		(RegWrite_MEM),	// WB
	.MemtoReg_out		(MemtoReg_MEM),	// WB
	.MemRead_out		(MemRead_MEM),	// M
	.MemWrite_out		(MemWrite_MEM),
	.ALUData_out		(ALUResult_MEM),
	.MemWriteData_out	(Rt_Data_MEM),
	.WBregister_out		(mux_RegDST_MEM)
);

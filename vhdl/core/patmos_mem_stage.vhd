library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;


entity patmos_mem_stage is 
  port
  (
    clk                          : in std_logic;
    rst       	 	                : in std_logic;
    din             	            : in mem_in_type;
    dout                         : out mem_out_type 
  );
end entity patmos_mem_stage;


architecture arch of patmos_mem_stage is

begin
  
  mem_wb: process(clk)
  begin
    if (rising_edge(clk)) then
      dout.reg_write_out <= din.reg_write_in;
      dout.mem_to_reg_out <= din.mem_to_reg_in;
      dout.mem_data_out <= din.mem_data_in;
      dout.alu_result_out <= din.alu_result_in;
      dout.write_back_reg_out <= din.write_back_reg_in;
    end if;
  end process mem_wb;

end arch;
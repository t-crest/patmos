-- TO DO:
-- move register manipulation to datapath
-- add write back control 
-- add memory control
library ieee;
use ieee.std_logic_1164.all;
use work.patmos_type_package.all;
use ieee.numeric_std.all;

entity patmos_decode is 
    port (
        clk, rst                        : in std_logic;
        din                             : in decode_in_type;
        dout                            : out decode_out_type                 
        -- two different vectors for instructions and functions should be implemented
    );
end entity patmos_decode;

architecture arch of patmos_decode is 

signal predicate_reg : integer;

begin
  
  --------------------------------
  -- decode instructions
  --------------------------------
  
  decode: process(clk)
  begin
     if rising_edge(clk) then
     --   if din.operation1(30) = '1' then -- predicate bits assignment
     --         dout.predicate_bit <= predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
     --       elsif din.operation1(30) = '0' then -- ~predicate bits assignment
     --         dout.predicate_bit <= not predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
     --   end if;   
        if din.operation(26 downto 25) = "00" then -- ALUi instruction
            dout.inst_type_out <= ALUi;  
            dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);
            dout.rd_out <= din.operation(21 downto 17); 
            dout.rs1_out <= din.operation(16 downto 12);
            dout.ALUi_immediate_out <= "00000000000000000000" & din.operation(11 downto 0);
            dout.rs1_data_out <= din.rs1_data_in;
           -- dout.reg_write_out <= din.reg_write_in;
            dout.alu_src_out <= '1'; -- choose the second source, i.e. immediate!
            dout.reg_write_out <= '1'; -- reg_write_out is reg_write_ex
            dout.mem_to_reg_out <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
            dout.mem_read_out <= '0';
            dout.mem_write_out <= '0';
       -- elsif din.operation1(26 downto 22) = "11111" then -- long immediate!
            
        elsif din.operation(26 downto 22) = "01000" then -- ALU instructions
            dout.inst_type_out <= ALU;  
            dout.ALU_function_type_out <= din.operation(3 downto 0);
            dout.rd_out <= din.operation(21 downto 17); 
            dout.rs1_out <= din.operation(16 downto 12);
            dout.rs2_out <= din.operation(11 downto 7);
            dout.rs1_data_out <= din.rs1_data_in;
            dout.rs2_data_out <= din.rs2_data_in;
          --  dout.reg_write_out <= din.reg_write_in;
            dout.alu_src_out <= '0'; -- choose the first source, i.e. reg!
            dout.reg_write_out <= '1'; -- reg_write_out is reg_write_ex
            dout.mem_to_reg_out <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
            dout.mem_read_out <= '0';
            dout.mem_write_out <= '0';
            case din.operation(6 downto 4) is
              when "000" => -- Register
                dout.ALU_instruction_type_out <= ALUr;
              when "001" => -- Unary
                dout.ALU_instruction_type_out <= ALUu;
              when "010" => -- Multuply
                dout.ALU_instruction_type_out <= ALUm;
              when "011" => -- Compare
                dout.ALU_instruction_type_out <= ALUc;
              when "100" => -- predicate
                dout.ALU_instruction_type_out <= ALUp;
              when others => NULL;
            end case; 
        elsif din.operation(26 downto 22) = "01011" then  -- store
            dout.inst_type_out <= STT;
         --   dout.rd_out <= din.operation(21 downto 17);
            dout.rs1_out <= din.operation(16 downto 12);
         --   dout.rs2_out <= din.operation(16 downto 12);
            dout.ALUi_immediate_out <= "0000000000000000000000000" & din.operation(6 downto 0);
            dout.rs1_data_out <= din.rs1_data_in;
            dout.rs2_data_out <= din.rs2_data_in; --value of rs2 is needed
--            dout.reg_write_out <= din.reg_write_in;
            dout.alu_src_out <= '1'; -- choose the second source, i.e. immediate!
            dout.reg_write_out <= '0'; -- we dont write in registers in store!
            dout.mem_to_reg_out <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
            dout.mem_read_out <= '0';
            dout.mem_write_out <= '1';
        elsif din.operation(26 downto 22) = "01010" then  -- load
            dout.inst_type_out <= LDT; 
--            dout.rd_out <= din.operation(21 downto 17);
            dout.rs1_out <= din.operation(16 downto 12);
            dout.ALUi_immediate_out <= "0000000000000000000000000" & din.operation(6 downto 0);
            dout.rs1_data_out <= din.rs1_data_in;
--            dout.reg_write_out <= din.reg_write_in;
            dout.alu_src_out <= '1'; -- choose the second source, i.e. immediate!
            dout.reg_write_out <= '1'; -- reg_write_out is reg_write_ex
            dout.mem_to_reg_out <= '1'; -- data comes from alu or mem ? 0 from alu and 1 from mem
            dout.mem_read_out <= '1';
            dout.mem_write_out <= '0';
     end if;
   end if;
   end process decode;
end arch;


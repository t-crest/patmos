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
--signal operation1_out : std_logic_vector (31 downto 0);
--signal operation2_out : std_logic_vector (31 downto 0);
--signal predicate_registers is std_logic_vector(7 downto 0);
signal predicate_reg : integer;

begin
  
  --------------------------------
  -- decode instructions
  --------------------------------
  
  decode: process(clk)
  begin
     if rising_edge(clk) then
        if din.operation1(30) = '1' then -- predicate bits assignment
              dout.predicate_bit <= predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
            elsif din.operation1(30) = '0' then -- ~predicate bits assignment
              dout.predicate_bit <= not predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
        end if;   
        
        if din.operation1(26 downto 25) = "00" then -- ALUi instruction
            dout.inst_type <= ALUi;  
            dout.ALU_function_type <= '0' & din.operation1(24 downto 22);
            dout.rd <= din.operation1(21 downto 17); -- move this to datapath
            dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
            dout.ALUi_immediate <= din.operation1(11 downto 0); --should be zero extended
            dout.pc_ctrl_gen <= PCNext;
            dout.wb_we <= '1';
       -- elsif din.operation1(26 downto 22) = "11111" then -- long immediate!
            
         elsif din.operation1(26 downto 22) = "01000" then -- ALU instructions
            dout.inst_type <= ALU;  
            dout.ALU_function_type <= din.operation1(3 downto 0);
            dout.wb_we <= '1';
            case din.operation1(6 downto 4) is
              when "000" => -- Register
                dout.ALU_instruction_type <= ALUr;
                dout.pc_ctrl_gen <= PCNext;
                dout.rd <= din.operation1(21 downto 17); -- move this to datapath
                dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
                dout.rs2 <= din.operation1(11 downto 7); -- move this to datapath
              when "001" => -- Unary
                dout.ALU_instruction_type <= ALUu;
                dout.pc_ctrl_gen <= PCNext;
                dout.rd <= din.operation1(21 downto 17); -- move this to datapath
                dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
              when "010" => -- Multuply
                dout.ALU_instruction_type <= ALUm;
                dout.pc_ctrl_gen <= PCNext;
                dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
                dout.rs2 <= din.operation1(11 downto 7); -- move this to datapath
              when "011" => -- Compare
                dout.ALU_instruction_type <= ALUc;
                dout.pc_ctrl_gen <= PCNext;
                dout.pd <= din.operation1(19 downto 17); -- move this to datapath
                dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
                dout.rs2 <= din.operation1(11 downto 7); -- move this to datapath
              when "100" => -- predicate
                dout.ALU_instruction_type <= ALUp;
                dout.pc_ctrl_gen <= PCNext; 
                dout.rd <= din.operation1(21 downto 17); -- move this to datapath-- we dont need the MSB on this
                dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath-- we dont need the MSB on this
                dout.rs2 <= din.operation1(11 downto 7); -- move this to datapath-- we dont need the MSB on this
              when others => NULL;
            end case;
        elsif din.operation1(26 downto 22) = "01001" then   
          dout.inst_type <= SPC;
          case din.operation1(6 downto 4) is
            --  when "000" =>               
            --  when "001" =>
              when "010" =>
                dout.sd <= din.operation1(3 downto 0);
                dout.rs1 <= din.operation1(16 downto 12);
              when "011" => 
                dout.ss <= din.operation1(3 downto 0);
                dout.rd <= din.operation1(16 downto 12);
              when others => NULL;
          end case;
          --SPC_function_type <= din.operation1(3 downto 0);
          
        elsif din.operation1(26 downto 22) = "01010" then   -- load
          dout.inst_type <= LDT;
          dout.pc_ctrl_gen <= PCNext;
          dout.rd <= din.operation1(21 downto 17); -- move this to datapath
          dout.rs1 <= din.operation1(16 downto 12); -- move this to datapath
          dout.load_immediate <= din.operation1(6 downto 0);
          dout.ld_function_type <= din.operation1(8 downto 7);
          case din.operation1(11 downto 9) is
              when "000" => 
                dout.ld_type <= lw;
              when "001" => 
                dout.ld_type <= lh; 
              when "010" => 
                dout.ld_type <= lb; 
              when "011" => 
                dout.ld_type <= lhu; 
              when "100" => 
                dout.ld_type <= lbu;  
              when "101" => 
                dout.ld_type <= dlwh;    
              when "110" => 
                dout.ld_type <= dlbh;    
              when "111" => 
                dout.ld_type <= dlbu;    
              when others => null;
          end case;
       -- elsif din.operation1(26 downto 22) = "01011" then  
     end if;
   end if;
   end process decode;
end arch;


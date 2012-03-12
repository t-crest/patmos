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
        predicate_bit                   : out std_logic;
        inst_type                       : out instruction_type;
      --  ALUi_function_type              : out unsigned(2 downto 0);
        ALU_function_type               : out unsigned(3 downto 0);
        ALU_instruction_type            : out ALU_inst_type;
        ALUi_immediate                  : out unsigned(11 downto 0);
        pc_ctrl_gen                     : out pc_type;
        multiplexer_a_ctrl              : out std_logic;
        multiplexer_b_ctrl              : out std_logic;
        alu_we                          : out std_logic;
        wb_we                           : out std_logic;
        operation1                      : in unsigned (31 downto 0);
       -- operation2                      : in std_logic_vector (31 downto 0);
        rs1                             : out unsigned (4 downto 0);
        rs2                             : out unsigned (4 downto 0);
       -- rt                              : out unsigned (4 downto 0);
        rd                              : out unsigned (4 downto 0);
        pd                              : out unsigned (2 downto 0);
        sd                              : out unsigned (3 downto 0);
        ss                              : out unsigned (3 downto 0)
     --   func                            : out unsigned (2 downto 0) --func and instruction are different 
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
        if operation1(30) = '1' then -- predicate bits assignment
              predicate_bit <= predicate_register_bank(to_integer(unsigned(operation1(29 downto 27))));
            elsif operation1(30) = '0' then -- ~predicate bits assignment
              predicate_bit <= not predicate_register_bank(to_integer(unsigned(operation1(29 downto 27))));
        end if;   
        
        if operation1(26 downto 25) = "00" then -- ALUi instruction
            inst_type <= ALUi;  
            ALU_function_type <= '0' & operation1(24 downto 22);
            rd <= operation1(21 downto 17); -- move this to datapath
            rs1 <= operation1(16 downto 12); -- move this to datapath
            ALUi_immediate <= operation1(11 downto 0); --should be zero extended
            pc_ctrl_gen <= PCNext;
            wb_we <= '1';
       -- elsif operation1(26 downto 22) = "11111" then -- long immediate!
            
         elsif operation1(26 downto 22) = "01000" then -- ALU instructions
            inst_type <= ALU;  
            ALU_function_type <= operation1(3 downto 0);
            wb_we <= '1';
            case operation1(6 downto 4) is
              when "000" => -- Register
                ALU_instruction_type <= ALUr;
                pc_ctrl_gen <= PCNext;
                rd <= operation1(21 downto 17); -- move this to datapath
                rs1 <= operation1(16 downto 12); -- move this to datapath
                rs2 <= operation1(11 downto 7); -- move this to datapath
              when "001" => -- Unary
                ALU_instruction_type <= ALUu;
                pc_ctrl_gen <= PCNext;
                rd <= operation1(21 downto 17); -- move this to datapath
                rs1 <= operation1(16 downto 12); -- move this to datapath
              when "010" => -- Multuply
                ALU_instruction_type <= ALUm;
                pc_ctrl_gen <= PCNext;
                rs1 <= operation1(16 downto 12); -- move this to datapath
                rs2 <= operation1(11 downto 7); -- move this to datapath
              when "011" => -- Compare
                ALU_instruction_type <= ALUc;
                pc_ctrl_gen <= PCNext;
                pd <= operation1(19 downto 17); -- move this to datapath
                rs1 <= operation1(16 downto 12); -- move this to datapath
                rs2 <= operation1(11 downto 7); -- move this to datapath
              when "100" => -- predicate
                ALU_instruction_type <= ALUp;
                pc_ctrl_gen <= PCNext; 
                rd <= operation1(21 downto 17); -- move this to datapath-- we dont need the MSB on this
                rs1 <= operation1(16 downto 12); -- move this to datapath-- we dont need the MSB on this
                rs2 <= operation1(11 downto 7); -- move this to datapath-- we dont need the MSB on this
              when others => NULL;
            end case;
        elsif operation1(26 downto 22) = "01001" then   
          inst_type <= SPC;
          case operation1(6 downto 4) is
            --  when "000" => 
                
            --  when "001" =>
              when "010" =>
                sd <= operation1(3 downto 0);
                rs1 <= operation1(16 downto 12);
              when "011" => 
                ss <= operation1(3 downto 0);
                rd <= operation1(16 downto 12);
              when others => NULL;
          end case;
          --SPC_function_type <= operation1(3 downto 0);
          
      --    case operation1(6 downto 4) is
       --       when "000" => 
       -- elsif operation1(26 downto 22) = "01010" then   
       -- elsif operation1(26 downto 22) = "01011" then  
     end if;
   end if;
   end process decode;
end arch;


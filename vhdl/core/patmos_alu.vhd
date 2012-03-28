library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_alu is
    port
    (
        clk                         : in std_logic;
        rst                         : in std_logic;
        din                         : in alu_in_type;
        dout                        : out alu_out_type
    );
end entity patmos_alu;

architecture arch of patmos_alu is

begin

    patmos_alu: process(din)
    begin
      case din.inst_type is
       when ALUi =>
        case din.ALU_function_type is
          when "0000" => dout.rd <= din.rs1 + din.rs2;
          when "0001" => dout.rd <= din.rs1 - din.rs2;
          when "0010" => dout.rd <= din.rs2 - din.rs1;
          when "0011" => dout.rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
          when "0100" => dout.rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
        --  when "0101" => dout.rd <= shift_right_arith(din.rs, ("00000000000000000000" & din.ALUi_immediate));
          when "0110" => dout.rd <= din.rs1 or din.rs2;
          when "0111" => dout.rd <= din.rs1 and din.rs2;
          when others => dout.rd <= din.rs1 + din.rs2;
        end case; 
      when ALU =>
        case din.ALU_instruction_type is
          when ALUr => 
            case din.ALU_function_type is
              when "0000" => dout.rd <= din.rs1 + din.rs2;
              when "0001" => dout.rd <= din.rs1 - din.rs2;
              when "0010" => dout.rd <= din.rs2 - din.rs1;
              when "0011" => dout.rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
              when "0100" => dout.rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
          ------------------?????when "0101" => rd <= SHIFT_RIGHT(signed(rs), to_integer(rt));
              when "0110" => dout.rd <= din.rs1 or din.rs2 ;
              when "0111" => dout.rd <= din.rs1 and din.rs2 ;
              --??  when "1000" => rd <= shift_left_logical(rs, rt) or ; --??
              when "1001" => dout.rd <= din.rs1 - din.rs2; --??
              when "1010" => dout.rd <= din.rs2 xor din.rs1; 
              when "1011" => dout.rd <= din.rs1 nor din.rs2; 
              --   when "1100" => rd <= shift_right_logical(rs, rt); --??
              --   when "1101" => rd <= shift_right_arith(rs, rt); --??
              when "1110" => dout.rd <= SHIFT_LEFT(din.rs1, 1)+ din.rs2;
              when "1111" => dout.rd <= SHIFT_LEFT(din.rs1, 2) + din.rs2 ;  
              when others => null;
            end case;
          when ALUu =>
            case din.ALU_function_type is
              when "0000" => dout.rd <= din.rs1(7)& din.rs1(7) & din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& 
                               din.rs1(7)& din.rs1(7) &din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)&
                               din.rs1(7)& din.rs1(7) &din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& 
                               din.rs1(7 downto 0);
              when "0001" => dout.rd <= din.rs1(7)& din.rs1(7) &din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)&
                               din.rs1(7)& din.rs1(7) &din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& din.rs1(7)& 
                               din.rs1(15 downto 0);
              when "0010" => dout.rd <= "0000000000000000" & din.rs1(15 downto 0);
              when "0101" => dout.rd <= "0" & din.rs1(30 downto 0);
              when others => null;
            end case;
          when others => null;
        end case;
        when LDT =>
            dout.rd <= din.rs1 + din.rs2;
        when STT =>
            dout.rd <= din.rs1 + din.rs2;
       when others => null; -- inst type
      end case; --inst type
    end process patmos_alu;
end arch;


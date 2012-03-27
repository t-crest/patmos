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

    alu: process(din)
    begin
      case din.inst_type is
       when ALUi =>
        case din.ALU_function_type is
          when "0000" => dout.rd <= din.rs1 + din.rs2;
          when "0001" => dout.rd <= din.rs1 -  din.rs2;
          when "0010" => dout.rd <= din.rs2 - din.rs1;
          when "0011" => dout.rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
          when "0100" => dout.rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
        --  when "0101" => dout.rd <= shift_right_arith(din.rs, ("00000000000000000000" & din.ALUi_immediate));
          when "0110" => dout.rd <= din.rs1 or din.rs2;
          when "0111" => dout.rd <= din.rs1 and din.rs2;
          when others => dout.rd <= din.rs1 + din.rs2;
        end case; 
       when others => null;
      end case;
    end process alu;
end arch;


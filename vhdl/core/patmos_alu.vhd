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
--	COMPONENT megaddsub
--	PORT (
--			add_sub							: IN STD_LOGIC ;
--			dataa, datab 					: IN STD_LOGIC_VECTOR(31 DOWNTO 0) ;
--			result							: OUT STD_LOGIC_VECTOR(31 DOWNTO 0) 
--			) ;
--	END COMPONENT ;
	
	--signal intermediate_add : STD_LOGIC_VECTOR(31 DOWNTO 0);
	--signal intermediate_sub : STD_LOGIC_VECTOR(31 DOWNTO 0);
	signal number_of_bytes_in_stack_cache			: unsigned(4 downto 0) := (others => '0');
	
begin
	--add: megaddsub
	--PORT MAP ('1', STD_LOGIC_VECTOR(din.rs1), STD_LOGIC_VECTOR(din.rs2), intermediate_add ) ;
	--sub: megaddsub
	--PORT MAP ('0', STD_LOGIC_VECTOR(din.rs1), STD_LOGIC_VECTOR(din.rs2), intermediate_sub ) ;
		
    patmos_alu: process(din)
    begin
      case din.inst_type is
       when ALUi =>
        case din.ALU_function_type is
          when "0000" => dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);
          when "0001" => dout.rd <= din.rs1 - din.rs2; --unsigned(intermediate_sub);--
--          when "0010" => dout.rd <= din.rs2 - din.rs1;
          when "0011" => dout.rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
          when "0100" => dout.rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
        --  when "0101" => dout.rd <= shift_right_arith(din.rs, ("00000000000000000000" & din.ALUi_immediate));
         when "0110" => dout.rd <= din.rs1 or din.rs2;
          when "0111" => dout.rd <= din.rs1 and din.rs2;
          when others => dout.rd <= din.rs1 + din.rs2; --unsigned(intermediate_add); 
        end case; 
      when ALU =>
        case din.ALU_instruction_type is
          when ALUr => 
            case din.ALU_function_type is
              when "0000" => dout.rd <= din.rs1 + din.rs2; --unsigned(intermediate_add);--
              when "0001" => dout.rd <= din.rs1 - din.rs2; --unsigned(intermediate_sub);--
              when "0010" => dout.rd <= din.rs2 - din.rs1; --unsigned(intermediate_sub);--
              when "0011" => dout.rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
              when "0100" => dout.rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
          ------------------?????when "0101" => rd <= SHIFT_RIGHT(signed(rs), to_integer(rt));
              when "0110" => dout.rd <= din.rs1 or din.rs2 ;
              when "0111" => dout.rd <= din.rs1 and din.rs2 ;
              --??  when "1000" => rd <= shift_left_logical(rs, rt) or ; --??
              when "1001" => dout.rd <= din.rs1 - din.rs2; --unsigned(intermediate_sub);-- --??
              when "1010" => dout.rd <= din.rs2 xor din.rs1; 
              when "1011" => dout.rd <= din.rs1 nor din.rs2; 
              --   when "1100" => rd <= shift_right_logical(rs, rt); --??
              --   when "1101" => rd <= shift_right_arith(rs, rt); --??
              when "1110" => dout.rd <= SHIFT_LEFT(din.rs1, 1)+ din.rs2;
              when "1111" => dout.rd <= SHIFT_LEFT(din.rs1, 2) + din.rs2 ;  
              when others => dout.rd <= din.rs1 + din.rs2; --unsigned(intermediate_add); --
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
              when others => dout.rd <= din.rs1 + din.rs2;
            end case;
          when others => dout.rd <= din.rs1 + din.rs2;
        end case;
        when LDT =>
        	case din.LDT_instruction_type is
        		when LWS =>
        			dout.rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 2)); -- igned, unsigned
        		when LHS =>
        			dout.rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 1));
        		when LBS =>
        			dout.rd <= din.rs1 + din.rs2;
        		when LHUS =>
        			dout.rd <= din.rs1 + din.rs2;
        		when LBUS =>
        			dout.rd <= din.rs1 + din.rs2;	
        		when others => null;
        	end case;	
           --dout.rd <= din.rs1 + din.rs2; --unsigned(intermediate_add);--
        when STT =>
        	case din.STT_instruction_type is
        		when SWS =>
        			dout.rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 2));
        		when SHS =>
        			dout.rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 1));
        		when SBS =>
        			dout.rd <= din.rs1 + din.rs2;
        		when others => dout.rd <= din.rs1 + din.rs2;
        	end case;
         --   dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
        when BEQ =>
            dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
    	when others => dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
    	end case;
    end process patmos_alu;
end arch;

--function SHIFT_RIGHT (ARG: SIGNED; COUNT: NATURAL) return SIGNED;
  -- Result subtype: SIGNED(ARG'LENGTH-1 downto 0)
  -- Result: Performs a shift-right on a SIGNED vector COUNT times.
  --         The vacated positions are filled with the leftmost
  --         element, ARG'LEFT. The COUNT rightmost elements are lost.



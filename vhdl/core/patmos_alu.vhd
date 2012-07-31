-- 
-- Copyright Technical University of Denmark. All rights reserved.
-- This file is part of the time-predictable VLIW Patmos.
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
-- 
--    1. Redistributions of source code must retain the above copyright notice,
--       this list of conditions and the following disclaimer.
-- 
--    2. Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
-- 
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
-- OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
-- NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
-- THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-- 
-- The views and conclusions contained in the software and documentation are
-- those of the authors and should not be interpreted as representing official
-- policies, either expressed or implied, of the copyright holder.
-- 


--------------------------------------------------------------------------------
-- Short descripton.
--
-- Author: Sahar Abbaspour
--------------------------------------------------------------------------------

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
          when ALUp =>
          	case din.ALU_function_type is
          	  when "0110" => dout.pd <= (din.ps1_negate xor din.ps1) or (din.ps1_negate xor din.ps2);
              when "0111" => dout.pd <= (din.ps1_negate xor din.ps1) and (din.ps1_negate xor din.ps2);
              when "1010" => dout.pd <= (din.ps1_negate xor din.ps1) xor (din.ps1_negate xor din.ps2);
              when "1011" => dout.pd <= not ((din.ps1_negate xnor din.ps1) or (din.ps1_negate xor din.ps2)); --nor
              when others => dout.pd <= (din.ps1_negate xor din.ps1) or (din.ps1_negate xor din.ps2);
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
        		when others => dout.rd <= din.rs1 + din.rs2;
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
    	when others => dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
    	end case;
    end process patmos_alu;
end arch;

--function SHIFT_RIGHT (ARG: SIGNED; COUNT: NATURAL) return SIGNED;
  -- Result subtype: SIGNED(ARG'LENGTH-1 downto 0)
  -- Result: Performs a shift-right on a SIGNED vector COUNT times.
  --         The vacated positions are filled with the leftmost
  --         element, ARG'LEFT. The COUNT rightmost elements are lost.




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
-- Author: Martin Schoeberl (martin@jopdesign.com)
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_alu is
	port(
		clk     : in  std_logic;
		rst     : in  std_logic;
		decdout : in  decode_out_type;
		din     : in  alu_in_type;
		doutex  : out execution_out_type
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
	signal number_of_bytes_in_stack_cache : unsigned(4 downto 0) := (others => '0');
	signal rd                             : unsigned(31 downto 0);
	signal cmp_equal, cmp_result          : std_logic;
	signal predicate, predicate_reg       : std_logic_vector(7 downto 0);
begin
	--add: megaddsub
	--PORT MAP ('1', STD_LOGIC_VECTOR(din.rs1), STD_LOGIC_VECTOR(din.rs2), intermediate_add ) ;
	--sub: megaddsub
	--PORT MAP ('0', STD_LOGIC_VECTOR(din.rs1), STD_LOGIC_VECTOR(din.rs2), intermediate_sub ) ;

	-- MS: TODO: This ALU needs to be restructured to share functional units
	-- also means more decoding in decode and not in execute

	-- we should assign default values;
	patmos_alu : process(din, decdout, predicate, predicate_reg, cmp_equal, cmp_result)
	begin
		case din.inst_type is
			when ALUi =>
				case din.ALU_function_type is
					when "0000" => rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);
					when "0001" => rd <= din.rs1 - din.rs2; --unsigned(intermediate_sub);--
					when "0010" => rd <= din.rs2 - din.rs1;
					when "0011" => rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2(4 downto 0)));
					when "0100" => rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2(4 downto 0)));
				    when "0101" => rd <= unsigned(std_logic_vector(resize(signed(din.rs1(31 downto to_integer(din.rs2(4 downto 0)))), 32)));
					when "0110" => rd <= din.rs1 or din.rs2;
					when "0111" => rd <= din.rs1 and din.rs2;
					when others => rd <= din.rs1 + din.rs2; --unsigned(intermediate_add); 
				end case;
			when ALU =>
				case din.ALU_instruction_type is
					when ALUr =>
						case din.ALU_function_type is
							when "0000" => rd <= din.rs1 + din.rs2; --unsigned(intermediate_add);--
							when "0001" => rd <= din.rs1 - din.rs2; --unsigned(intermediate_sub);--
							when "0010" => rd <= din.rs2 - din.rs1; --unsigned(intermediate_sub);--
							when "0011" => rd <= SHIFT_LEFT(din.rs1, to_integer(din.rs2));
							when "0100" => rd <= SHIFT_RIGHT(din.rs1, to_integer(din.rs2));
							when "0101" => rd <= unsigned(std_logic_vector(resize(signed(din.rs1(31 downto to_integer(din.rs2(4 downto 0)))), 32)));
							when "0110" => rd <= din.rs1 or din.rs2;
							when "0111" => rd <= din.rs1 and din.rs2;
							when "1000" => rd <= --SHIFT_LEFT(din.rs1, to_integer(din.rs2(4 downto 0))) or 
										 unsigned(std_logic_vector(resize(signed(din.rs1(31 downto 32 - to_integer(din.rs2(4 downto 0)))), 32)));
							when "1001" => rd <= SHIFT_LEFT(din.rs1, 32 - to_integer(din.rs2(4 downto 0))) or 
										 unsigned(std_logic_vector(resize(signed(din.rs1(31 downto to_integer(din.rs2(4 downto 0)))), 32)));
							when "1010" => rd <= din.rs2 xor din.rs1;
							when "1011" => rd <= din.rs1 nor din.rs2;
							when "1100" => rd <= SHIFT_LEFT(din.rs1, 1) + din.rs2;
							when "1101" => rd <= SHIFT_LEFT(din.rs1, 2) + din.rs2;
							when others => rd <= din.rs1 + din.rs2; 
						end case;
					when ALUu =>
						case din.ALU_function_type is
							when "0000" => rd <= din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) &
									din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7 downto 0);
							when "0001" => rd <= din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(7) & din.rs1(15 downto 0);
							when "0010" => rd <= "0000000000000000" & din.rs1(15 downto 0);
							when "0101" => rd <= "0" & din.rs1(30 downto 0);
							when others => rd <= din.rs1 + din.rs2;
						end case;
--					when ALUp =>
--						case din.ALU_function_type is
--						-- TODO: the predicate register is now right here in this stage
--						-- rewrite the following
--							when "0110" => pd <= (din.ps1_negate xor din.ps1) or (din.ps1_negate xor din.ps2);
--							when "0111" => pd <= (din.ps1_negate xor din.ps1) and (din.ps1_negate xor din.ps2);
--							when "1010" => pd <= (din.ps1_negate xor din.ps1) xor (din.ps1_negate xor din.ps2);
--							when "1011" => pd <= not ((din.ps1_negate xnor din.ps1) or (din.ps1_negate xor din.ps2)); --nor
--							when others => pd <= (din.ps1_negate xor din.ps1) or (din.ps1_negate xor din.ps2);
--						end case;

					when others => rd <= din.rs1 + din.rs2;
				end case;
			when LDT =>
				case din.LDT_instruction_type is
					when LWS =>
						rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 2)); -- igned, unsigned
					when LHS =>
						rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 1));
					when LBS =>
						rd <= din.rs1 + din.rs2;
					when LHUS =>
						rd <= din.rs1 + din.rs2;
					when LBUS =>
						rd            <= din.rs1 + din.rs2;
					when others => rd <= din.rs1 + din.rs2;
				end case;
			--dout.rd <= din.rs1 + din.rs2; --unsigned(intermediate_add);--
			when STT =>
				case din.STT_instruction_type is
					when SWS =>
						rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 2));
					when SHS =>
						rd <= unsigned(SHIFT_LEFT(signed(din.rs1 + din.rs2), 1));
					when SBS =>
						rd            <= din.rs1 + din.rs2;
					when others => rd <= din.rs1 + din.rs2;
				end case;
			--   dout.rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
			when others => rd <= din.rs1 + din.rs2; -- unsigned(intermediate_add);--
		end case;

		-- compare instructions
		cmp_equal <= '0';
		cmp_result <= '0';
		predicate  <= predicate_reg;
		
		if din.rs1 = din.rs2 then
			cmp_equal <= '1';
		end if;

		case decdout.ALU_function_type_out(2 downto 0) is
			when "000" => cmp_result <= cmp_equal;
			when "001" => cmp_result <= not cmp_equal;
			when others => null;
		end case;
		if decdout.instr_cmp='1' then
			predicate(to_integer(decdout.pd_out(2 downto 0))) <= cmp_result;
		end if;
		-- the ever true predicate
		predicate(0) <= '1';

	end process patmos_alu;

	-- TODO: remove all predicate related stuff from EX out  
	process(rst, clk)
	begin
		if rst = '1' then
			predicate_reg <= "00000001";
			doutex.predicate <= "00000001";
		elsif rising_edge(clk) then
			if predicate_reg(to_integer(decdout.predicate_condition)) /= decdout.predicate_bit_out then
				doutex.mem_read_out  <= decdout.mem_read_out;
				doutex.mem_write_out <= decdout.mem_write_out;
				doutex.reg_write_out <= decdout.reg_write_out;
			--      	doutex.ps_reg_write_out <= decdout.ps_reg_write_out;
			--	test <= '1';
			else
				doutex.mem_read_out     <= '0';
				doutex.mem_write_out    <= '0';
				doutex.reg_write_out    <= '0';
			end if;

			doutex.mem_to_reg_out           <= decdout.mem_to_reg_out;
			doutex.alu_result_out           <= rd;
			doutex.mem_write_data_out       <= din.mem_write_data_in;
			doutex.write_back_reg_out       <= decdout.rd_out;
			doutex.STT_instruction_type_out <= decdout.STT_instruction_type_out;
			doutex.LDT_instruction_type_out <= decdout.LDT_instruction_type_out;
			-- this should be under predicate condition as well
			doutex.predicate                <= predicate;
			predicate_reg                   <= predicate;
		end if;
	end process;

end arch;

--function SHIFT_RIGHT (ARG: SIGNED; COUNT: NATURAL) return SIGNED;
-- Result subtype: SIGNED(ARG'LENGTH-1 downto 0)
-- Result: Performs a shift-right on a SIGNED vector COUNT times.
--         The vacated positions are filled with the leftmost
--         element, ARG'LEFT. The COUNT rightmost elements are lost.




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
	signal number_of_bytes_in_stack_cache : std_logic_vector(4 downto 0) := (others => '0');
	signal rd                              : std_logic_vector(31 downto 0);
	signal cmp_equal, cmp_result          : std_logic;
	signal predicate, predicate_reg       : std_logic_vector(7 downto 0);
	signal rs1, rs2							: unsigned(31 downto 0);
	signal rd_tmp							: std_logic_vector(31 downto 0);
begin

	-- MS: TODO: This ALU needs to be restructured to share functional units
	-- also means more decoding in decode and not in execute

	-- we should assign default values;
	process(din.rs1, din.rs2)
	begin
		rs1 <= unsigned(din.rs1);
		rs2 <= unsigned(din.rs2);
	end process;
	patmos_alu : process(din, decdout, predicate, predicate_reg, cmp_equal, cmp_result, rs1, rs2)
	begin
--		rs1 <= unsigned(din.rs1);
--		rs2 <= unsigned(din.rs2);
		case din.inst_type is
			when ALUi =>
				case din.ALU_function_type is
					when "0000" => rd <= std_logic_vector(rs1 + rs2); --add
					when "0001" => rd <= std_logic_vector(rs1 - rs2); --sub
					when "0010" => rd <= std_logic_vector(rs2 - rs1); -- sub invert
					when "0011" => rd <= std_logic_vector(SHIFT_LEFT(rs1, to_integer(rs2(4 downto 0)))); --sl
					when "0100" => rd <= std_logic_vector(SHIFT_RIGHT(rs1, to_integer(rs2(4 downto 0)))); -- sr
				    when "0101" => rd <= std_logic_vector(SHIFT_RIGHT(signed(rs1), to_integer(rs2(4 downto 0)))); -- sra
					when "0110" => rd <= std_logic_vector(rs1 or rs2); -- or
					when "0111" => rd <= std_logic_vector(rs1 and rs2); -- and
					when others => rd <= std_logic_vector(rs1 + rs2); -- default add! 
				end case;
			when ALU =>
				case din.ALU_instruction_type is
					when ALUr =>
						case din.ALU_function_type is
							when "0000" => rd <= std_logic_vector(rs1 + rs2); --add
							when "0001" => rd <= std_logic_vector(rs1 - rs2); --sub
							when "0010" => rd <= std_logic_vector(rs2 - rs1); -- sub invert
							when "0011" => rd <= std_logic_vector(SHIFT_LEFT(rs1, to_integer(rs2(4 downto 0)))); --sl
							when "0100" => rd <= std_logic_vector(SHIFT_RIGHT(rs1, to_integer(rs2(4 downto 0)))); -- sr
							when "0101" => rd <= std_logic_vector(SHIFT_RIGHT(signed(rs1), to_integer(rs2(4 downto 0)))); -- sra
							when "0110" => rd <= std_logic_vector(rs1 or rs2); -- or
							when "0111" => rd <= std_logic_vector(rs1 and rs2); -- and
							-----
							when "1000" => rd <= std_logic_vector(SHIFT_LEFT(rs1, to_integer(rs2(4 downto 0))) or 
										 						  SHIFT_RIGHT(rs1, 32 - to_integer(rs2(4 downto 0))	)); -- rl
							when "1001" => rd <= std_logic_vector(SHIFT_LEFT(rs1, 32 - to_integer(rs2(4 downto 0))) or 
															      SHIFT_RIGHT(rs2, to_integer(rs2(4 downto 0))));
							when "1010" => rd <= std_logic_vector(din.rs2 xor din.rs1);
							when "1011" => rd <= std_logic_vector(din.rs1 nor din.rs2);
							when "1100" => rd <= std_logic_vector(SHIFT_LEFT(rs1, 1) + rs2);
							when "1101" => rd <= std_logic_vector(SHIFT_LEFT(rs1, 2) + rs2);
							when others => rd <= std_logic_vector(rs1 + rs2); -- default add! 
						end case;
					when ALUu =>
						case din.ALU_function_type is
							when "0000" => rd <= std_logic_vector(rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) &
									rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7 downto 0));
							when "0001" => rd <= std_logic_vector(rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(15 downto 0));
							when "0010" => rd <= std_logic_vector("0000000000000000" & rs1(15 downto 0));
							when "0101" => rd <= std_logic_vector("0" & rs1(30 downto 0));
							when others => rd <= std_logic_vector(rs1 + rs2);
						end case;
					when ALUp =>
						case din.ALU_function_type is
						
							when "0110" => predicate_reg(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <= 					
												(decdout.ps1_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps1_out(2 downto 0)))) ) or 
												(decdout.ps2_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps2_out(2 downto 0)))));
						
							when "0111" =>  predicate_reg(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <= 
											(decdout.ps1_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps1_out(2 downto 0)))) ) and 
											(decdout.ps2_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps2_out(2 downto 0)))));
						
							when "1010" =>  predicate_reg(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <= 
											(decdout.ps1_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps1_out(2 downto 0)))) ) xor 
											(decdout.ps2_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps2_out(2 downto 0)))));
						
							when "1011" =>  predicate_reg(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <=
											not 
											((decdout.ps1_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps1_out(2 downto 0)))) ) or 
											(decdout.ps2_out(3) xor predicate_reg(to_integer(unsigned(decdout.ps2_out(2 downto 0)))))
											); --nor
						
							when others =>  predicate_reg(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <= '0';
						end case;

					when others => rd <= std_logic_vector(rs1 + rs2);
				end case;
			when LDT =>
				case din.LDT_instruction_type is
					----- scratchpad memory
					when LWL =>
						 rd <= std_logic_vector(rs1 + (SHIFT_LEFT(rs2, 2)));
					when LHL =>
						 rd_tmp <= std_logic_vector(rs1 + (SHIFT_LEFT(rs2, 1)));
						 rd <= std_logic_vector(resize(signed(rd_tmp(15 downto 0)), 32));
					when LBL =>
						 rd_tmp <= std_logic_vector(rs1 + rs2);
						 rd <= std_logic_vector(resize(signed(rd_tmp(7 downto 0)), 32));	
					when LHUL =>
						 rd_tmp <= std_logic_vector(rs1 + (SHIFT_LEFT(rs2, 1)));
						 rd <= std_logic_vector(resize(unsigned(rd_tmp(15 downto 0)), 32));	
					when LBUL =>
						 rd_tmp <= std_logic_vector(rs1 + rs2);
						 rd <= std_logic_vector(resize(unsigned(rd_tmp(7 downto 0)), 32));
					----------------------------------------
					when LWS =>
						rd <= std_logic_vector(SHIFT_LEFT(signed(rs1 + rs2), 2)); 
					when LHS =>
						rd <= std_logic_vector(SHIFT_LEFT(signed(rs1 + rs2), 1));
					when LBS =>
						rd <= std_logic_vector(rs1 + rs2);
					when LHUS =>
						rd <= std_logic_vector(rs1 + rs2);
					when LBUS =>
						rd    <= std_logic_vector(rs1 + rs2);
					when others => rd <= std_logic_vector(rs1 + rs2);
				end case;

			when STT =>
				case din.STT_instruction_type is
					----- scratchpad memory
					when SWL =>
						rd <=  std_logic_vector(rs1 + (SHIFT_LEFT(rs2, 2)));
					when SHL =>
						rd_tmp <= std_logic_vector(rs1 + (SHIFT_LEFT(rs2, 1)));
						rd <= std_logic_vector(resize(signed(rd_tmp(15 downto 0)), 32));
					when SBL =>
						rd_tmp <= std_logic_vector(rs1 + rs2);
						rd <= std_logic_vector(resize(unsigned(rd_tmp(7 downto 0)), 32));	
					----------------------------------------
					when SWS =>
						rd <= std_logic_vector(SHIFT_LEFT(signed(rs1 + rs2), 2));
					when SHS =>
						rd <= std_logic_vector(SHIFT_LEFT(signed(rs1 + rs2), 1));
					when SBS =>
						rd <= std_logic_vector(rs1 + rs2);
					when others => rd <= std_logic_vector(rs1 + rs2);
				end case;
			--   dout.rd <= rs1 + rs2; -- unsigned(intermediate_add);--
			when others => rd <= std_logic_vector(rs1 + rs2); -- unsigned(intermediate_add);--
		end case;

		-- compare instructions
		cmp_equal <= '0';
		cmp_result <= '0';
		predicate  <= predicate_reg;
		
		if signed(rs1) = signed(rs2) then
			cmp_equal <= '1';
		end if;

		case decdout.ALU_function_type_out(2 downto 0) is
			when "000" => cmp_result <= cmp_equal;
			when "001" => cmp_result <= not cmp_equal;
			when "010" =>  if (signed(rs1) < signed(rs2) ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when "011" =>  if (signed(rs1) <= signed(rs2) ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when "100" =>  if (rs1 < rs2 ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when "101" =>  if (rs1 <= rs2 ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when "110" =>  if (rs1(to_integer(rs2)) = '1') then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when others => null;
		end case;
		if decdout.instr_cmp='1' then
			predicate(to_integer(unsigned(decdout.pd_out(2 downto 0)))) <= cmp_result;
		end if;
		-- the ever true predicate
		predicate(0) <= '1';
		
	end process patmos_alu;
	process(rd)
	begin
		doutex.alu_result <= rd;
	end process;
	-- TODO: remove all predicate related stuff from EX out  
	process(rst, clk)
	begin
		if rst = '1' then
			predicate_reg <= "00000001";
			doutex.predicate <= "00000001";
		elsif rising_edge(clk) then
			if predicate_reg(to_integer(unsigned(decdout.predicate_condition))) /= decdout.predicate_bit_out then
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




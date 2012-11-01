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
		clk										: in  std_logic;
		rst										: in  std_logic;
		decdout									: in  decode_out_type;
		doutex									: out execution_out_type;
		memdout									: in mem_out_type
		
	);
end entity patmos_alu;

architecture arch of patmos_alu is

	signal rd, rd1, rd2, adrs					: std_logic_vector(31 downto 0);
	signal cmp_equal, cmp_result				: std_logic;
	signal predicate, predicate_reg				: std_logic_vector(7 downto 0);
	signal rs1, rs2								: unsigned(31 downto 0);
	signal doutex_alu_result_reg				: std_logic_vector(31 downto 0);
	signal doutex_alu_adrs_reg					: std_logic_vector(31 downto 0);
	signal doutex_write_back_reg				: std_logic_vector(4 downto 0);
	signal doutex_reg_write						: std_logic;
	signal din_rs1, din_rs2, alu_src2			: std_logic_vector(31 downto 0);
	signal shamt 								: integer;
	signal shifted_arg							: unsigned(31 downto 0);
	signal pc									: std_logic_vector(pc_length - 1 downto 0);
	signal doutex_lm_write						: std_logic;
	signal doutex_reg_write_reg					: std_logic;
	signal doutex_lm_read						: std_logic;
	signal predicate_checked					: std_logic_vector(7 downto 0);
begin


	-- we should assign default values;
	process(din_rs1, din_rs2)
	begin
		rs1 <= unsigned(din_rs1);
		rs2 <= unsigned(din_rs2);
	end process;
	
	
	process(decdout)
	begin
		case decdout.adrs_type is
			when word => 
				shamt <= 2;
			when half =>
				shamt <= 1;
			when byte =>
				shamt <= 0;
			when others => null;
		end case;
	end process;
	process(shamt, rs2)
	begin
		shifted_arg <= SHIFT_LEFT(rs2, shamt);
	end process;
	process(rs1, rs2, shifted_arg)
	begin 
		adrs <= std_logic_vector(rs1 + shifted_arg);
	end process;
	
	process(decdout, predicate, predicate_reg, cmp_result)
	begin
		predicate  <= predicate_reg;
		if (decdout.is_predicate_inst = '1') then 
			case decdout.pat_function_type_alu_p is
				when pat_por => predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <= 					
								(decdout.ps1(3) xor predicate_reg(to_integer(unsigned(decdout.ps1(2 downto 0)))) ) or 
								(decdout.ps2(3) xor predicate_reg(to_integer(unsigned(decdout.ps2(2 downto 0)))));
						
				when pat_pand => predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <=
								(decdout.ps1(3) xor predicate_reg(to_integer(unsigned(decdout.ps1(2 downto 0)))) ) and 
								(decdout.ps2(3) xor predicate_reg(to_integer(unsigned(decdout.ps2(2 downto 0)))));
						
				when pat_pxor =>  predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <= 
								(decdout.ps1(3) xor predicate_reg(to_integer(unsigned(decdout.ps1(2 downto 0)))) ) xor 
								(decdout.ps2(3) xor predicate_reg(to_integer(unsigned(decdout.ps2(2 downto 0)))));
						
				when pat_pnor =>  predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <=
										not ((decdout.ps1(3) xor predicate_reg(to_integer(unsigned(decdout.ps1(2 downto 0)))) ) or 
											(decdout.ps2(3) xor predicate_reg(to_integer(unsigned(decdout.ps2(2 downto 0)))))); --nor
				when others =>  predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <= '0';
			end case;
		end if;
		if decdout.instr_cmp='1' then
			predicate(to_integer(unsigned(decdout.pd(2 downto 0)))) <= cmp_result;
		end if;
		-- the ever true predicate
		predicate(0) <= '1';
	end process;
	
	process(decdout, rs1, rs2) -- ALU
	begin 
		rd1 <= "00000000000000000000000000000000";
		case decdout.pat_function_type_alu is
			when pat_add => rd1 <= std_logic_vector(rs1 + rs2); --add
			when pat_sub => rd1 <= std_logic_vector(rs1 - rs2); --sub
			when pat_rsub => rd1 <= std_logic_vector(rs2 - rs1); -- sub invert
			when pat_sl => rd1 <= std_logic_vector(SHIFT_LEFT(rs1, to_integer(rs2(4 downto 0)))); --sl
			when pat_sr => rd1 <= std_logic_vector(SHIFT_RIGHT(rs1, to_integer(rs2(4 downto 0)))); -- sr
			when pat_sra => rd1 <= std_logic_vector(SHIFT_RIGHT(signed(rs1), to_integer(rs2(4 downto 0)))); -- sra
			when pat_or => rd1 <= std_logic_vector(rs1 or rs2); -- or
			when pat_and => rd1 <= std_logic_vector(rs1 and rs2); -- and
						-----
			when pat_rl => rd1 <= std_logic_vector(ROTATE_LEFT(rs1, to_integer(rs2(4 downto 0))));-- rl
			when pat_rr => rd1 <= std_logic_vector(ROTATE_RIGHT(rs1, to_integer(rs2(4 downto 0))));
			when pat_xor => rd1 <= std_logic_vector(rs2 xor rs1);
			when pat_nor => rd1 <= std_logic_vector(rs1 nor rs2);
			when pat_shadd => rd1 <= std_logic_vector(SHIFT_LEFT(rs1, 1) + rs2);
			when pat_shadd2 => rd1 <= std_logic_vector(SHIFT_LEFT(rs1, 2) + rs2);

			when others => rd1 <= std_logic_vector(rs1 + rs2); -- default add! 
		end case;
	end process;
	
	process(decdout, rs1, rs2)
	begin
		rd2 <= "00000000000000000000000000000000";
		case decdout.pat_function_type_alu_u is
			when pat_sext8 => rd2 <= std_logic_vector(rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) &
									rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7) & rs1(7 downto 0));
			when pat_sext16 => rd2 <= std_logic_vector(rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15) & rs1(15 downto 0));
			when pat_zext16 => rd2 <= std_logic_vector("0000000000000000" & rs1(15 downto 0));
			when pat_abs => rd2 <= std_logic_vector(abs(signed(rs1)));
			when others => rd2 <= std_logic_vector(rs1 + rs2); -- default add! 
		end case;
	end process;
	
	process(decdout, cmp_equal, cmp_result, rs1, rs2)
	begin
		cmp_equal <= '0';
		cmp_result <= '0';
		if signed(rs1) = signed(rs2) then
			cmp_equal <= '1';
		end if;

		case decdout.pat_function_type_alu_cmp is
			when  pat_cmpeq => cmp_result <= cmp_equal;
			when pat_cmpneq => cmp_result <= not cmp_equal;
			when pat_cmplt =>  if (signed(rs1) < signed(rs2) ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when pat_cmple =>  if (signed(rs1) <= signed(rs2) ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when pat_cmpult =>  if (rs1 < rs2 ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when pat_cmpule =>  if (rs1 <= rs2 ) then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when pat_btest =>  if (rs1(to_integer(rs2(4 downto 0))) = '1') then cmp_result <= '1'; else cmp_result <= '0' ; end if;
			when others => null;
		end case;
	end process;
	
	process(rd1, rd2, decdout)
	begin
		if (decdout.alu_alu_u = '1') then
			rd <= rd1;
		else
			rd <= rd2;	
		end if;
	end process;
		
	-- TODO: remove all predicate related stuff from EX out  
	process(rst, clk)
	begin
		if rst = '1' then
			predicate_reg <= "00000001";
			doutex.predicate <= "00000001";
		elsif rising_edge(clk) then
			doutex.lm_write <= doutex_lm_write; 
			doutex.reg_write <= doutex_reg_write;
			doutex_reg_write_reg <= doutex_reg_write;
			doutex.lm_read <= doutex_lm_read;
			doutex.mem_to_reg           <= decdout.mem_to_reg;
			doutex.alu_result_reg           <= rd;
			doutex.adrs_reg		      	  <= adrs;
			doutex.write_back_reg       <= decdout.rd;
			-- this should be under predicate condition as well
			doutex.predicate                <= predicate_checked;
			predicate_reg                   <= predicate_checked;
			

			doutex_alu_result_reg           <= rd;
			doutex_alu_adrs_reg           <= adrs;
			doutex_write_back_reg       <= decdout.rd;
			
			
		end if;
	end process;
	
	
	process(decdout, alu_src2, rd, adrs, predicate_reg, predicate)
	begin
		doutex.lm_write_out_not_reg              <= '0';
		doutex.lm_read_out_not_reg              <= '0';
		predicate_checked						<= "00000001";
		if predicate_reg(to_integer(unsigned(decdout.predicate_condition))) /= decdout.predicate_bit then
				doutex.lm_write_out_not_reg              <= decdout.lm_write;
				doutex.lm_read_out_not_reg              <= decdout.lm_read;
		end if;
		doutex.mem_write_data <= alu_src2;
		doutex.alu_result <= rd;
		doutex.adrs <= adrs;
		if predicate_reg(to_integer(unsigned(decdout.predicate_condition))) /= decdout.predicate_bit then
			doutex_lm_write              <= decdout.lm_write;
			doutex_lm_read              <= decdout.lm_read;
			doutex_reg_write <= decdout.reg_write;
			predicate_checked <= predicate;
		else
			doutex_lm_write              <= '0';
			doutex_lm_read              <= '0';
			doutex_reg_write    <= '0';
			doutex_reg_write <= '0';
		end if;
	end process;

	process(decdout) -- branch pc relative
	begin
		doutex.pc <= std_logic_vector(unsigned(decdout.pc) + unsigned(decdout.imm));
	end process;
	
	process(doutex_alu_result_reg, doutex_write_back_reg, doutex_reg_write_reg , decdout, memdout)
	begin
		if (decdout.rs1 = doutex_write_back_reg and doutex_reg_write_reg = '1' ) then
			din_rs1 <= doutex_alu_result_reg;
		elsif (decdout.rs1 = memdout.write_back_reg_out and memdout.reg_write_out = '1' ) then
			din_rs1 <= memdout.data_out;
		else
			din_rs1 <= decdout.rs1_data;
		end if;
	end process;
	
	process(doutex_alu_result_reg, doutex_write_back_reg, doutex_reg_write_reg , decdout, memdout)
	begin
		if (decdout.rs2 = doutex_write_back_reg and doutex_reg_write_reg = '1' ) then
			alu_src2 <= doutex_alu_result_reg;
		elsif (decdout.rs2 = memdout.write_back_reg_out and memdout.reg_write_out = '1' ) then
			alu_src2 <= memdout.data_out;
		else
			alu_src2 <= decdout.rs2_data;
		end if;
	end process;


	process(alu_src2, decdout.imm, decdout.alu_src)
	begin
		if (decdout.alu_src = '0') then
			din_rs2 <= alu_src2;
		else
			din_rs2 <= decdout.imm;
		end if;
	end process;
	

end arch;





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
use work.patmos_type_package.all;
use ieee.numeric_std.all;

entity patmos_decode is
	port(
		clk, rst : in  std_logic;
		din      : in  decode_in_type;
		dout     : out decode_out_type
	-- two different vectors for instructions and functions should be implemented
	);
end entity patmos_decode;

architecture arch of patmos_decode is
	
	signal alu_func : std_logic_vector(3 downto 0);
	
begin

	--------------------------------
	-- decode instructions
	--------------------------------

	-- MS: are we sure that each field is assigned a value in each condition?
	-- MS: wouldn't it be better to have one combinational process and one register
	-- process. So we get a latch warning.

	-- MS: I would prefer a case statement instead of if elsif priority based decoding.

	-- And all this copy of assignments. We need sensible default assignments.
	
	-- The source selection between immediate and register might be done here
	--		Depends on the critical path
	

	decode : process(clk, alu_func)
	begin
		if rising_edge(clk) then
			dout.imm <= std_logic_vector(resize(signed(din.operation(11 downto 0)), 32));
			-- MS: that's the way I would like decoding:
			-- a single bit for a condition in the ALU
			dout.instr_cmp <= '0';
			
			
			-- MS: time for some defaults to get a clearer view:
			dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);

			dout.predicate_bit_out   <= din.operation(30); -- 
			dout.predicate_condition <= din.operation(29 downto 27);
			dout.rd_out              <= din.operation(21 downto 17);
			dout.rs1_out             <= din.operation(16 downto 12);
			dout.ps1_out             <= din.operation(15 downto 12);
			dout.ps2_out             <= din.operation(10 downto 7);
			dout.pd_out              <= '0' & din.operation(19 downto 17);
			dout.rd_out              <= din.operation(21 downto 17);
			dout.rs1_out             <= din.operation(16 downto 12);
			dout.rs2_out             <= din.operation(11 downto 7);
			dout.rs1_data_out        <= din.rs1_data_in;
			dout.rs2_data_out        <= din.rs2_data_in;
			-- dout.reg_write_out <= din.reg_write_in;
			dout.alu_src_out      <= '1'; -- choose the second source, i.e. immediate!
		--	dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
			dout.mem_to_reg_out   <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
--			dout.mem_read_out     <= '0';
			dout.mem_write_out    <= '0';
			
			dout.sc_write_out             <= '0';
			dout.lm_write_out			  <= '0';
			dout.s_u 					  <= '1';
			dout.BC 							  <= '0';
			-- TODO: get defaults for all signals and remove redundant assignments

			--   if din.operation1(30) = '1' then -- predicate bits assignment
			--         dout.predicate_bit <= predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
			--       elsif din.operation1(30) = '0' then -- ~predicate bits assignment
			--         dout.predicate_bit <= not predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
			--   end if;   
			dout.alu_alu_u <= '1';
			case alu_func is
				when "0000" =>  dout.pat_function_type_alu <= pat_add;
				when "0001" => dout.pat_function_type_alu <= pat_sub;
				when "0010" => dout.pat_function_type_alu <= pat_rsub;
				when "0011" => dout.pat_function_type_alu <= pat_sl;
				when "0100" => dout.pat_function_type_alu <= pat_sr;
				when "0101" => dout.pat_function_type_alu <= pat_sra;
				when "0110" => dout.pat_function_type_alu <= pat_or;
				when "0111" => dout.pat_function_type_alu <= pat_and;
				-----
				when "1000" => dout.pat_function_type_alu <= pat_rl;
				when "1001" => dout.pat_function_type_alu <= pat_rr;
				when "1010" => dout.pat_function_type_alu <= pat_xor;
				when "1011" => dout.pat_function_type_alu <= pat_nor;
				when "1100" => dout.pat_function_type_alu <= pat_shadd;
				when "1101" => dout.pat_function_type_alu <= pat_shadd2;
				when others => dout.pat_function_type_alu <= pat_add; -- default add! 
			end case;
			dout.is_predicate_inst			 <= '0';
			
			
			if din.operation(26 downto 25) = "00" then -- ALUi instruction
				dout.reg_write_out    <= '1';
				dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);
				
			elsif din.operation(26 downto 22) = "11111" then -- long immediate!
				dout.ALU_function_type_out <= din.operation(3 downto 0);
				dout.reg_write_out    <= '1';
				dout.imm  <= din.instr_b;
			
			elsif din.operation(26 downto 22) = "01000" then -- ALU instructions
				dout.ALU_function_type_out <= din.operation(3 downto 0);

				--  dout.reg_write_out <= din.reg_write_in;
				dout.alu_src_out <= '0'; -- choose the first source, i.e. reg!

			--	dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
				case din.operation(6 downto 4) is
					when "000" =>       -- Register
						dout.ALU_instruction_type_out <= ALUr;
						dout.reg_write_out    <= '1';
					when "001" =>       -- Unary
						dout.ALU_instruction_type_out <= ALUu;
						dout.reg_write_out    <= '1';
						case din.operation(3 downto 0) is
							when "0000" => dout.pat_function_type_alu_u <= pat_sext8;
							when "0001" => dout.pat_function_type_alu_u <= pat_sext16;
							when "0010" => dout.pat_function_type_alu_u <= pat_zext16;
							when "0011" => dout.pat_function_type_alu_u <=  pat_abs;
							when others => dout.pat_function_type_alu_u <= pat_sext8;
						end case;
						dout.alu_alu_u <= '0';
					when "010" =>       -- Multuply
						dout.ALU_instruction_type_out <= ALUm;
						dout.reg_write_out    <= '1';
					when "011" =>       -- Compare
						dout.ALU_instruction_type_out <= ALUc;
						dout.instr_cmp <= '1';
						dout.reg_write_out <= '0';
						
					when "100" =>       -- predicate
						dout.ALU_instruction_type_out <= ALUp;
						dout.reg_write_out <= '0';
						dout.is_predicate_inst <= '1';
						case din.operation(3 downto 0) is
							when "0110" =>  dout.pat_function_type_alu_p <= pat_por;
							when "0111" => dout.pat_function_type_alu_p <= pat_pand;
							when "1010" => dout.pat_function_type_alu_p <= pat_pxor;
							when "1011" => dout.pat_function_type_alu_p <= pat_pnor;
							when others => dout.pat_function_type_alu_p <= pat_por;
						end case;
					when others => NULL;
				end case;

			elsif din.operation(26 downto 22) = "01011" then -- store
--						dout.sc_write_out             <= '1';
--						dout.sc_read_out              <= '0';
				case din.operation(21 downto 17) is
					----- scratchpad memory
					when "00001" =>
--						dout.STT_instruction_type_out <= SWL;
						dout.lm_write_out			  <= '1';
						dout.adrs_type <= word;
					when "00101" =>
--						dout.STT_instruction_type_out <= SHL;
						dout.lm_write_out			  <= '1';
						dout.adrs_type <= half;
					when "01001" =>	
--						dout.STT_instruction_type_out <= SBL;
						dout.lm_write_out			  <= '1';
						dout.adrs_type <= byte;
					----------------------------------------	
					when "00000" =>
--						dout.STT_instruction_type_out <= SWS;
						dout.adrs_type <= word;
--						dout.sc_write_out             <= '1';
					when "00100" =>
--						dout.STT_instruction_type_out <= SHS;
						dout.adrs_type <= half;
	--					dout.sc_write_out             <= '1';
					when "01000" =>
--						dout.STT_instruction_type_out <= SBS;
						dout.adrs_type <= byte;
--						dout.sc_write_out             <= '1';
					----------------------------------------- global memory	
					when "00011" =>
--						dout.STT_instruction_type_out <= SWM;
						dout.adrs_type <= word;
						dout.lm_write_out			  <= '1';
					--	dout.mem_write_out      <= '1';
					when "00111" =>
--						dout.STT_instruction_type_out <= SHM;
						dout.adrs_type <= half;
						dout.lm_write_out			  <= '1';
					when "01011" =>
--						dout.STT_instruction_type_out <= SBM;
						dout.adrs_type <= byte;
						dout.lm_write_out			  <= '1';
						
					---------------------------------------- data cache
					when "00010" =>
--						dout.STT_instruction_type_out <= SWC;
						dout.adrs_type <= word;
						dout.lm_write_out			  <= '1';
					--	dout.mem_write_out      <= '1';
					when "00110" =>
--						dout.STT_instruction_type_out <= SHC;
						dout.adrs_type <= half;
						dout.lm_write_out			  <= '1';
					when "01010" =>
--						dout.STT_instruction_type_out <= SBC;
						dout.adrs_type <= byte;
						dout.lm_write_out			  <= '1';
						
					
						-- MS: why is sc_write_out here '0'?
						--dout.sc_write_out             <= '0';
						--dout.sc_read_out              <= '0';
					when others => null;
				end case;
				dout.rs1_out            <= din.operation(16 downto 12);
				dout.rs2_out            <= din.operation(11 downto 7);
				dout.imm <= std_logic_vector(resize(signed(din.operation(6 downto 0)), 32));
				dout.alu_src_out        <= '1'; -- choose the second source, i.e. immediate!
				dout.reg_write_out      <= '0'; -- we dont write in registers in store!
				

			elsif din.operation(26 downto 22) = "01010" then -- load
				case din.operation(11 downto 7) is
					----- scratchpad memory
					when "00001" =>
						dout.adrs_type <= word;
					when "00101" =>
						dout.adrs_type <= half;
					when "01001" =>
						dout.adrs_type <= byte;	
					when "01101" =>
						dout.adrs_type <= half;
						dout.s_u		<= '0';
					when "10001" =>
						dout.adrs_type <= byte;			
						dout.s_u		<= '0';
					----------------------------------------
					when "00000" =>
						dout.adrs_type <= word;
					when "00100" =>
						dout.adrs_type <= half;
					when "01000" =>
						dout.adrs_type <= byte;
					when "01100" =>
						dout.adrs_type <= half;
						dout.s_u		<= '0';
					when "10000" =>
						dout.adrs_type <= byte;
						dout.s_u		<= '0';
					----------------------------------------- global memory	
					when "00011" =>
						dout.adrs_type <= word;
					when "00111" =>
						dout.adrs_type <= half;
					when "01011" =>
						dout.adrs_type <= byte;
					when "01111" =>
						dout.adrs_type <= half;
						dout.s_u		<= '0';
					when "10011" =>
						dout.adrs_type <= byte;
						dout.s_u		<= '0';
					---------------------------------------- data cache
					when "00010" =>
						dout.adrs_type <= word;
					when "00110" =>
						dout.adrs_type <= half;
					when "01010" =>
						dout.adrs_type <= byte;
					when "01110" =>
						dout.adrs_type <= half;
						dout.s_u		<= '0';
					when "10010" =>
						dout.adrs_type <= byte;	
						dout.s_u		<= '0';
					when others => null;
				end case;
				dout.rd_out             <= din.operation(21 downto 17);
				dout.rs1_out            <= din.operation(16 downto 12);
				dout.imm <= std_logic_vector(resize(signed(din.operation(6 downto 0)), 32));
				--            dout.reg_write_out <= din.reg_write_in;
				dout.alu_src_out      <= '1'; -- choose the second source, i.e. immediate!
				dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
				dout.mem_to_reg_out   <= '1'; -- data comes from alu or mem ? 0 from alu and 1 from mem
				

			elsif din.operation(26 downto 22) = "11001" then -- branch, cache relative
				dout.alu_src_out        <= '0'; -- choose the second source, i.e. immediate!
				dout.reg_write_out      <= '0'; -- reg_write_out is reg_write_ex
				dout.mem_to_reg_out     <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
				dout.BC						<= '1';
			elsif din.operation(26 downto 24) = "011" then -- STC
				case din.operation(23 downto 22) is
					when "00" =>        -- reserve
						dout.st_out                   <= "0111"; -- s6 is st (7th register in special reg file)
						--	dout.stc_immediate_out <= din.operation(4 downto 0);--"0000000000" & din.operation(21 downto 0); 
						dout.imm <= std_logic_vector(resize(signed(din.operation(4 downto 0)), 32));
						dout.alu_src_out      <= '0'; -- choose the first source, i.e. reg!
						dout.reg_write_out    <= '0'; -- reg_write_out is reg_write_ex

					when "01" =>        -- ensure
						dout.st_out                   <= "0111";
--						dout.stc_immediate_out        <= din.operation(4 downto 0);
					when "10" =>
						dout.imm       <= std_logic_vector(resize(signed(din.operation(4 downto 0)), 32));
						dout.alu_src_out              <= '0'; -- choose the first source, i.e. reg!
						dout.reg_write_out            <= '0'; -- reg_write_out is reg_write_ex
					when others => NULL;
				end case;
			elsif din.operation(26 downto 22) = "01001" then -- nop   
				dout.imm <= std_logic_vector(resize(signed(din.operation(3 downto 0)), 32));
				dout.alu_src_out      <= '0'; -- choose the second source, i.e. immediate!
				dout.reg_write_out    <= '0'; -- reg_write_out is reg_write_ex

			end if;
		end if;
	end process decode;
	
	process(din)
	begin
		--if rising_edge(clk) then
			case din.operation(26 downto 25) is 
				when "00" =>
					alu_func <= '0' & din.operation(24 downto 22);
				when "01" =>
					alu_func <= din.operation(3 downto 0);
				when "11" =>
					alu_func <= din.operation(3 downto 0);
				when others => null; 
			end case;
	end process;
end arch;





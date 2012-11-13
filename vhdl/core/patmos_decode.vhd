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
		clk									: in  std_logic;
		rst									: in  std_logic;
		din									: in  decode_in_type;
		dout								: out decode_out_type
	);
end entity patmos_decode;

architecture arch of patmos_decode is
	
	signal alu_func							: std_logic_vector(3 downto 0);
--	signal dout						: decode_out_type;
--	signal prev_dout						: decode_out_type;
begin

	--------------------------------
	-- decode instructions
	--------------------------------

	-- MS: are we sure that each field is assigned a value in each condition?
	-- MS: wouldn't it be better to have one combinational process and one register
	-- process. So we get a latch warning.
	
	-- The source selection between immediate and register might be done here
	--		Depends on the critical path
	

	decode : process(clk, alu_func)
	begin
		if rising_edge(clk) then
			dout.imm <= std_logic_vector(resize(signed(din.operation(11 downto 0)), 32));
			-- MS: that's the way I would like decoding:
			-- a single bit for a condition in the ALU
			dout.instr_cmp <= '0';
--			dout.stall <= '0';
			dout.pc <= din.pc;
			-- MS: time for some defaults to get a clearer view:
--			dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);
			dout.inst <= nop;
			dout.predicate_bit   <= din.operation(30); -- 
			dout.predicate_condition <= din.operation(29 downto 27);
			dout.rd              <= din.operation(21 downto 17);
			dout.rs1             <= din.operation(16 downto 12);
			dout.ps1             <= din.operation(15 downto 12);
			dout.ps2             <= din.operation(10 downto 7);
			dout.pd              <= '0' & din.operation(19 downto 17);
			dout.rd              <= din.operation(21 downto 17);
			dout.rs1             <= din.operation(16 downto 12);
			dout.rs2             <= din.operation(11 downto 7);
			dout.rs1_data        <= din.rs1_data_in;
			dout.rs2_data        <= din.rs2_data_in;
			dout.alu_src      	 <= '1'; -- choose the second source, i.e. immediate!
			dout.mem_to_reg   	 <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
			dout.lm_read         <= '0';
			dout.sc_read	     <= '0';
			dout.lm_write		 <= '0';
			dout.sc_write		 <= '0';
			dout.s_u 			 <= '1';
			dout.BC 			 <= '0';
			-- TODO: get defaults for all signals and remove redundant assignments 
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
				dout.reg_write    <= '1';
				dout.imm  <= "00000000000000000000" & din.operation(11 downto 0);
				dout.inst <= alui;
			
			elsif din.operation(26 downto 24) = "011" then -- STC
				case din.operation(23 downto 22) is
					when "00" =>        -- reserve
						dout.pat_function_type_sc <= reserve;
--						dout.st_out                   <= "0111"; -- s6 is st (7th register in special reg file)
						--	dout.stc_immediate_out <= din.operation(4 downto 0);--"0000000000" & din.operation(21 downto 0); 
						dout.imm <= std_logic_vector(resize(signed(din.operation(21 downto 0)), 32));
						dout.inst <= res;
					when "01" =>        -- ensure
						dout.pat_function_type_sc <= ensure;
						dout.inst <= ens;
--						dout.st_out                   <= "0111";
--						dout.stc_immediate_out        <= din.operation(4 downto 0);
					when "10" =>
						dout.pat_function_type_sc <= free;
						dout.imm       <= std_logic_vector(resize(signed(din.operation(4 downto 0)), 32));
						dout.inst <= free;
						
					when others => NULL;
				end case;
		--	end if;
			else
			case din.operation(26 downto 22) is
				when "11111" => -- long immediate!
					dout.reg_write    <= '1';
					dout.imm  <= din.instr_b;
				
				when "01000" => -- ALU instructions
					dout.inst <= alu;
					dout.alu_src <= '0'; -- choose the first source, i.e. reg!
	
					case din.operation(6 downto 4) is
						when "000" =>       -- Register
							dout.reg_write    <= '1';
						when "001" =>       -- Unary
							dout.reg_write    <= '1';
							case din.operation(3 downto 0) is
								when "0000" => dout.pat_function_type_alu_u <= pat_sext8;
								when "0001" => dout.pat_function_type_alu_u <= pat_sext16;
								when "0010" => dout.pat_function_type_alu_u <= pat_zext16;
								when "0011" => dout.pat_function_type_alu_u <=  pat_abs;
								when others => dout.pat_function_type_alu_u <= pat_sext8;
							end case;
							dout.alu_alu_u <= '0';
						when "010" =>       -- Multiply
							dout.reg_write    <= '1';
						when "011" =>       -- Compare
							dout.instr_cmp <= '1';
							dout.reg_write <= '0';
						case din.operation(2 downto 0) is
								when "000" =>  dout.pat_function_type_alu_cmp <= pat_cmpeq;
								when "001" => dout.pat_function_type_alu_cmp <= pat_cmpneq;
								when "010" => dout.pat_function_type_alu_cmp <= pat_cmplt;
								when "011" => dout.pat_function_type_alu_cmp <= pat_cmple;
								when "100" => dout.pat_function_type_alu_cmp <= pat_cmpult;
								when "101" => dout.pat_function_type_alu_cmp <= pat_cmpule;
								when "110" => dout.pat_function_type_alu_cmp <= pat_btest;
								when others => null;
						end case;
						when "100" =>       -- predicate
							dout.reg_write <= '0';
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
	
				when  "01011" => -- store
	--						dout.sc_write_out             <= '1';
	--						dout.sc_read_out              <= '0';
					dout.inst <= st;
					case din.operation(21 downto 17) is
						----- scratchpad memory
						when "00001" =>
							dout.lm_write			  <= '1';
							dout.adrs_type <= word;
						when "00101" =>
							dout.lm_write			  <= '1';
							dout.adrs_type <= half;
						when "01001" =>	
							dout.lm_write			  <= '1';
							dout.adrs_type <= byte;
						----------------------------------------	stack cache
						when "00000" =>
							dout.adrs_type <= word;
							dout.sc_write				<= '1';
						when "00100" =>
							dout.adrs_type <= half;
							dout.sc_write				<= '1';
						when "01000" =>
							dout.adrs_type <= byte;
							dout.sc_write				<= '1';
						----------------------------------------- global memory	
						when "00011" =>
							dout.adrs_type <= word;
							dout.lm_write			  <= '1';
						when "00111" =>
							dout.adrs_type <= half;
							dout.lm_write			  <= '1';
						when "01011" =>
							dout.adrs_type <= byte;
							dout.lm_write			  <= '1';
							
						---------------------------------------- data cache
						when "00010" =>
							dout.adrs_type <= word;
							dout.lm_write			  <= '1';
						when "00110" =>
							dout.adrs_type <= half;
							dout.lm_write			  <= '1';
						when "01010" =>
							dout.adrs_type <= byte;
							dout.lm_write			  <= '1';
							
						
							-- MS: why is sc_write_out here '0'?
							--dout.sc_write_out             <= '0';
							--dout.sc_read_out              <= '0';
						when others => null;
					end case;
					dout.rs1            <= din.operation(16 downto 12);
					dout.rs2            <= din.operation(11 downto 7);
					dout.imm <= std_logic_vector(resize(signed(din.operation(6 downto 0)), 32));
					dout.alu_src        <= '1'; -- choose the second source, i.e. immediate!
					dout.reg_write      <= '0'; -- we dont write in registers in store!
					
	
				when "01010" => -- load
					dout.inst <= ld;
					case din.operation(11 downto 7) is
						----- scratchpad memory
						when "00001" =>
							dout.adrs_type <= word;
							dout.lm_read        <= '1';
						when "00101" =>
							dout.adrs_type <= half;
							dout.lm_read        <= '1';
						when "01001" =>
							dout.adrs_type <= byte;	
							dout.lm_read        <= '1';
						when "01101" =>
							dout.adrs_type <= half;
							dout.lm_read        <= '1';
							dout.s_u		<= '0';
						when "10001" =>
							dout.adrs_type <= byte;			
							dout.lm_read       <= '1';
							dout.s_u		<= '0';
						---------------------------------------- stack cache
						when "00000" =>
							dout.adrs_type <= word;
							dout.sc_read			  <= '1';
						when "00100" =>
							dout.adrs_type <= half;
							dout.sc_read			  <= '1';
						when "01000" =>
							dout.adrs_type <= byte;
							dout.sc_read			  <= '1';
						when "01100" =>
							dout.adrs_type <= half;
							dout.s_u		<= '0';
							dout.sc_read			  <= '1';
						when "10000" =>
							dout.adrs_type <= byte;
							dout.s_u		<= '0';
							dout.sc_read			  <= '1';
						----------------------------------------- global memory	
						when "00011" =>
							dout.adrs_type <= word;
							dout.lm_read       <= '1';
						when "00111" =>
							dout.adrs_type <= half;
							dout.lm_read        <= '1';
						when "01011" =>
							dout.adrs_type <= byte;
							dout.lm_read       <= '1';
						when "01111" =>
							dout.adrs_type <= half;
							dout.lm_read       <= '1';
							dout.s_u		<= '0';
						when "10011" =>
							dout.adrs_type <= byte;
							dout.lm_read       <= '1';
							dout.s_u		<= '0';
						---------------------------------------- data cache
						when "00010" =>
							dout.adrs_type <= word;
							dout.lm_read       <= '1';
						when "00110" =>
							dout.adrs_type <= half;
							dout.lm_read       <= '1';
						when "01010" =>
							dout.adrs_type <= byte;
							dout.lm_read        <= '1';
						when "01110" =>
							dout.adrs_type <= half;
							dout.lm_read       <= '1';
							dout.s_u		<= '0';
						when "10010" =>
							dout.adrs_type <= byte;	
							dout.lm_read        <= '1';
							dout.s_u		<= '0';
						when others => null;
					end case;
					dout.rd             <= din.operation(21 downto 17);
					dout.rs1            <= din.operation(16 downto 12);
					dout.imm <= std_logic_vector(resize(signed(din.operation(6 downto 0)), 32));				
					dout.alu_src      <= '1'; -- choose the second source, i.e. immediate!
					dout.reg_write    <= '1'; -- reg_write_out is reg_write_ex
					dout.mem_to_reg   <= '1'; -- data comes from alu or mem ? 0 from alu and 1 from mem
					
	
				when "11001" => -- branch, cache relative
					dout.alu_src        <= '0'; -- choose the second source, i.e. immediate!
					dout.reg_write      <= '0'; -- reg_write_out is reg_write_ex
					dout.mem_to_reg     <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
					dout.BC						<= '1';
					dout.inst <= br;
		--		elsif din.operation(26 downto 22) = "01001" then -- nop  "is removed from ISA"
		--			dout.imm <= std_logic_vector(resize(signed(din.operation(3 downto 0)), 32));
		--			dout.alu_src      <= '0'; -- choose the second source, i.e. immediate!
		--			dout.reg_write    <= '0'; -- reg_write_out is reg_write_ex
		--			dout.mem_to_reg   <= '0';
				
--				when "01001" => -- wait
--					case din.operation(6 downto 4) is
--						when "001" => 
--							dout.stall <= '1';
--					  	when others => null;
--					end case;
				when others => null;
			end case;
			end if; -- if instruction
		end if;
	end process decode;
	
--	process(dout)
--	begin
--		prev_dout <= dout;
--		if(dout.stall = '0') then
--			dout <= dout;
--			prev_dout <= dout;
--		else
--			dout <= prev_dout; 	
--		end if;
--	end process;
	
	
	process(din)
	begin
			alu_func <= '0' & din.operation(24 downto 22);
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





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
	signal predicate_reg : integer;

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

	decode : process(clk)
	begin
		if rising_edge(clk) then
			dout.imm <= std_logic_vector(resize(signed(din.operation(11 downto 0)), 32));

			-- MS: time for some defaults to get a clearer view:
			dout.inst_type_out         <= ALUi;
			dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);

			dout.predicate_bit_out   <= din.operation(30); -- 
			dout.predicate_condition <= din.operation(29 downto 27);
			dout.predicate_data_out  <= din.predicate_data_in;
			dout.rd_out              <= din.operation(21 downto 17);
			dout.rs1_out             <= din.operation(16 downto 12);
			-- MS: I think this should be sign extended, like above dout.imm
			dout.ALUi_immediate_out  <= "00000000000000000000" & din.operation(11 downto 0);
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
			dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
			dout.ps_reg_write_out <= '0';
			dout.mem_to_reg_out   <= '0'; -- data comes from alu or mem ? 0 from alu and 1 from mem
			dout.mem_read_out     <= '0';
			dout.mem_write_out    <= '0';
			-- TODO: get defaults for all signals and remove redundant assignments

			--   if din.operation1(30) = '1' then -- predicate bits assignment
			--         dout.predicate_bit <= predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
			--       elsif din.operation1(30) = '0' then -- ~predicate bits assignment
			--         dout.predicate_bit <= not predicate_register_bank(to_integer(unsigned(din.operation1(29 downto 27))));
			--   end if;   
			if din.operation(26 downto 25) = "00" then -- ALUi instruction
				dout.inst_type_out         <= ALUi;
				dout.ALU_function_type_out <= '0' & din.operation(24 downto 22);
			-- elsif din.operation1(26 downto 22) = "11111" then -- long immediate!
			elsif din.operation(26 downto 22) = "01000" then -- ALU instructions
				dout.inst_type_out         <= ALU;
				dout.ALU_function_type_out <= din.operation(3 downto 0);

				--  dout.reg_write_out <= din.reg_write_in;
				dout.alu_src_out <= '0'; -- choose the first source, i.e. reg!

				dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
				dout.ps_reg_write_out <= '0';
				case din.operation(6 downto 4) is
					when "000" =>       -- Register
						dout.ALU_instruction_type_out <= ALUr;
					when "001" =>       -- Unary
						dout.ALU_instruction_type_out <= ALUu;
					when "010" =>       -- Multuply
						dout.ALU_instruction_type_out <= ALUm;
					when "011" =>       -- Compare
						dout.ALU_instruction_type_out <= ALUc;
					when "100" =>       -- predicate
						dout.ALU_instruction_type_out <= ALUp;
					when others => NULL;
				end case;

			elsif din.operation(26 downto 22) = "01011" then -- store
				dout.inst_type_out <= STT;
						dout.sc_write_out             <= '1';
						dout.sc_read_out              <= '0';
				case din.operation(21 downto 17) is
					when "00000" =>
						dout.STT_instruction_type_out <= SWS;
					when "00100" =>
						dout.STT_instruction_type_out <= SHS;
					when "01000" =>
						dout.STT_instruction_type_out <= SBS;
					when "00011" =>
						dout.STT_instruction_type_out <= SWM;
						-- MS: why is sc_write_out here '0'?
						dout.sc_write_out             <= '0';
						dout.sc_read_out              <= '0';
					when others => null;
				end case;
				dout.rs1_out            <= din.operation(16 downto 12);
				dout.rs2_out            <= din.operation(11 downto 7);
				dout.ALUi_immediate_out <= "0000000000000000000000000" & din.operation(6 downto 0);
				dout.alu_src_out        <= '1'; -- choose the second source, i.e. immediate!
				dout.reg_write_out      <= '0'; -- we dont write in registers in store!
				dout.ps_reg_write_out   <= '0';
				dout.mem_write_out      <= '1';

			elsif din.operation(26 downto 22) = "01010" then -- load
				dout.inst_type_out <= LDT;
						dout.sc_write_out             <= '0';
						dout.sc_read_out              <= '1';
				case din.operation(11 downto 7) is
					when "00000" =>
						dout.LDT_instruction_type_out <= LWS;
					when "00100" =>
						dout.LDT_instruction_type_out <= LHS;
					when "01000" =>
						dout.LDT_instruction_type_out <= LBS;
					when "01100" =>
						dout.LDT_instruction_type_out <= LHUS;
					when "10000" =>
						dout.LDT_instruction_type_out <= LBUS;
					when "00011" =>
						dout.LDT_instruction_type_out <= LWM;
						-- again no read, no write?
						dout.sc_write_out             <= '0';
						dout.sc_read_out              <= '0';
					when others => null;
				end case;
				dout.rd_out             <= din.operation(21 downto 17);
				dout.rs1_out            <= din.operation(16 downto 12);
				dout.ALUi_immediate_out <= "0000000000000000000000000" & din.operation(6 downto 0);
				--            dout.reg_write_out <= din.reg_write_in;
				dout.alu_src_out      <= '1'; -- choose the second source, i.e. immediate!
				dout.reg_write_out    <= '1'; -- reg_write_out is reg_write_ex
				dout.ps_reg_write_out <= '0';
				dout.mem_to_reg_out   <= '1'; -- data comes from alu or mem ? 0 from alu and 1 from mem
				dout.mem_read_out     <= '1';

			elsif din.operation(26 downto 22) = "11001" then -- branch, cache relative
				dout.inst_type_out <= BC;
				dout.alu_src_out        <= '0'; -- choose the second source, i.e. immediate!
				dout.reg_write_out      <= '0'; -- reg_write_out is reg_write_ex
				dout.ps_reg_write_out   <= '0';
				dout.mem_to_reg_out     <= '1'; -- data comes from alu or mem ? 0 from alu and 1 from mem

			elsif din.operation(26 downto 24) = "011" then -- STC
				dout.inst_type_out <= STC;
				case din.operation(23 downto 22) is
					when "00" =>        -- reserve
						dout.STC_instruction_type_out <= SRES;
						dout.st_out                   <= "0111"; -- s6 is st (7th register in special reg file)
						--	dout.stc_immediate_out <= din.operation(4 downto 0);--"0000000000" & din.operation(21 downto 0); 
						dout.ALUi_immediate_out <= "000000000000000000000000000" & din.operation(4 downto 0);
						dout.alu_src_out      <= '0'; -- choose the first source, i.e. reg!
						dout.reg_write_out    <= '0'; -- reg_write_out is reg_write_ex
						dout.ps_reg_write_out <= '0';

					when "01" =>        -- ensure
						dout.STC_instruction_type_out <= SENS;
						dout.st_out                   <= "0111";
						dout.stc_immediate_out        <= din.operation(4 downto 0);
					--	dout.ALUi_immediate_out <= 
					when "10" =>
						dout.STC_instruction_type_out <= SFREE;
						dout.ALUi_immediate_out       <= "000000000000000000000000000" & din.operation(4 downto 0);
						dout.alu_src_out              <= '0'; -- choose the first source, i.e. reg!
						dout.reg_write_out            <= '0'; -- reg_write_out is reg_write_ex
						dout.ps_reg_write_out         <= '0';
					when others => NULL;
				end case;
			elsif din.operation(26 downto 22) = "01001" then -- nop   
				dout.inst_type_out <= NOP;
				dout.ALUi_immediate_out <= "0000000000000000000000000000" & din.operation(3 downto 0);
				dout.alu_src_out      <= '0'; -- choose the second source, i.e. immediate!
				dout.reg_write_out    <= '0'; -- reg_write_out is reg_write_ex
				dout.ps_reg_write_out <= '0';

			end if;
		end if;
	end process decode;
end arch;





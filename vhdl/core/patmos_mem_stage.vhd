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

entity patmos_mem_stage is
	port(
		clk  : in  std_logic;
		rst  : in  std_logic;
		din  : in  mem_in_type;
		dout : out mem_out_type
	);
end entity patmos_mem_stage;

architecture arch of patmos_mem_stage is
	signal en0, en1, en2, en3               : std_logic;
	signal dout0, dout1, dout2, dout3       : std_logic_vector(7 downto 0);
	signal mem_write_data0, mem_write_data1 : std_logic_vector(7 downto 0);
	signal mem_write_data2, mem_write_data3 : std_logic_vector(7 downto 0);
	signal byte_enable0, byte_enable1       : std_logic;
	signal byte_enable2, byte_enable3       : std_logic;
	signal word_enable0, word_enable1       : std_logic;
--signal test : std_logic;
begin
	mem_wb : process(clk)
	begin
		if (rising_edge(clk)) then
			dout.data_out <= din.data_in;
			-- forwarding
			dout.reg_write_out      <= din.reg_write_in;
			dout.write_back_reg_out <= din.write_back_reg_in;
			dout.mem_write_data_out <= din.mem_write_data_in;
		end if;
	end process mem_wb;

	--	memory: entity work.patmos_data_memory(arch)
	--	port map(clk, din.alu_result,
	--	din.mem_write_data_in, din.mem_write, din.alu_result, dout.data_mem_data_out);

	memory0 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.alu_result(9 downto 0),
			     mem_write_data0,
			     en0,
			     din.alu_result(9 downto 0),
			     dout0);

	memory1 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.alu_result(9 downto 0),
			     mem_write_data1,
			     en1,
			     din.alu_result(9 downto 0),
			     dout1);

	memory2 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.alu_result(9 downto 0),
			     mem_write_data2,
			     en2,
			     din.alu_result(9 downto 0),
			     dout2);

	memory3 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.alu_result(9 downto 0),
			     mem_write_data3,
			     en3,
			     din.alu_result(9 downto 0),
			     dout3);

	ld_type : process(din, dout0, dout1, dout2, dout3)
	begin
		dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;
		if (din.LDT_instruction_type_out = LWL or din.LDT_instruction_type_out = LWC or din.LDT_instruction_type_out = LWM) then
			dout.data_mem_data_out <= dout0 & dout1 & dout2 & dout3;

		elsif (din.LDT_instruction_type_out = LHL or din.LDT_instruction_type_out = LHC or din.LDT_instruction_type_out = LHM or din.LDT_instruction_type_out = LHUL) then
			case din.alu_result(0) is
				when '0' =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout0 & dout1), 32));
				when '1' =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout2 & dout3), 32));
				when others => null;
			end case;

		elsif (din.LDT_instruction_type_out = LBL or din.LDT_instruction_type_out = LBC or din.LDT_instruction_type_out = LBM) then
			case din.alu_result(1 downto 0) is
				when "00" =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout0), 32));
				when "01" =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout1), 32));
				when "10" =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout2), 32));
				when "11" =>
					dout.data_mem_data_out <= std_logic_vector(resize(signed(dout3), 32));
				when others => null;
			end case;
		elsif (din.LDT_instruction_type_out = LHUL or din.LDT_instruction_type_out = LHUC or din.LDT_instruction_type_out = LHUM) then
			--	dout.data_mem_data_out <= std_logic_vector(resize(unsigned( dout0 & dout1), 32));
			case din.alu_result(0) is
				when '0' =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout0 & dout1), 32));
				when '1' =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout2 & dout3), 32));
				when others => null;
			end case;
		elsif (din.LDT_instruction_type_out = LBUL or din.LDT_instruction_type_out = LBUC or din.LDT_instruction_type_out = LBUM) then
			case din.alu_result(1 downto 0) is
				when "00" =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout0), 32));
				when "01" =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout1), 32));
				when "10" =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout2), 32));
				when "11" =>
					dout.data_mem_data_out <= std_logic_vector(resize(unsigned(dout3), 32));
				when others => null;
			end case;

		end if;
	end process;

	-- MS: why is enable always the same for all four bytes?
	-- Doesn't this depend on the store size and the address?
	-- Probably better to have one multiplexer independent of
	--  typs, just dependent on store size and address?
	-- Should this be part of the address calculation, in it's
	-- own component?
	byte_address_decode : process(din)
	begin
		byte_enable0 <= '0';
		byte_enable1 <= '0';
		byte_enable2 <= '0';
		byte_enable3 <= '0';
		case din.alu_result(1 downto 0) is
			when "00"   => byte_enable0 <= din.mem_write;
			when "01"   => byte_enable1 <= din.mem_write;
			when "10"   => byte_enable2 <= din.mem_write;
			when "11"   => byte_enable3 <= din.mem_write;
			when others => null;
		end case;
	end process byte_address_decode;

	word_address_decode : process(din)
	begin
		word_enable0 <= '0';
		word_enable1 <= '0';
		case din.alu_result(0) is
			when '0'    => word_enable0 <= din.mem_write;
			when '1'    => word_enable1 <= din.mem_write;
			when others => null;
		end case;
	end process word_address_decode;

	st_store : process(din, din.mem_write, word_enable0, word_enable1, byte_enable0, byte_enable1, byte_enable2, byte_enable3)
	begin
		en0             <= '0';
		en1             <= '0';
		en2             <= '0';
		en3             <= '0';
		mem_write_data0 <= din.mem_write_data_in(31 downto 24);
		mem_write_data1 <= din.mem_write_data_in(23 downto 16);
		mem_write_data2 <= din.mem_write_data_in(15 downto 8);
		mem_write_data3 <= din.mem_write_data_in(7 downto 0);
		if (din.STT_instruction_type_out = SWL or din.STT_instruction_type_out = SWM or din.STT_instruction_type_out = SWC) then
			en0             <= din.mem_write;
			en1             <= din.mem_write;
			en2             <= din.mem_write;
			en3             <= din.mem_write;
			mem_write_data0 <= din.mem_write_data_in(31 downto 24);
			mem_write_data1 <= din.mem_write_data_in(23 downto 16);
			mem_write_data2 <= din.mem_write_data_in(15 downto 8);
			mem_write_data3 <= din.mem_write_data_in(7 downto 0);
		elsif (din.STT_instruction_type_out = SHL or din.STT_instruction_type_out = SHM or din.STT_instruction_type_out = SHC) then
			en0             <= word_enable0;
			en1             <= word_enable0;
			en2             <= word_enable1;
			en3             <= word_enable1;
			mem_write_data0 <= din.mem_write_data_in(15 downto 8);
			mem_write_data1 <= din.mem_write_data_in(7 downto 0);
			mem_write_data2 <= din.mem_write_data_in(15 downto 8);
			mem_write_data3 <= din.mem_write_data_in(7 downto 0);
		elsif (din.STT_instruction_type_out = SBL or din.STT_instruction_type_out = SBM or din.STT_instruction_type_out = SBC) then
			en0 <= byte_enable0;
			en1 <= byte_enable1;
			en2 <= byte_enable2;
			en3 <= byte_enable3;

			mem_write_data0 <= din.mem_write_data_in(7 downto 0);
			mem_write_data1 <= din.mem_write_data_in(7 downto 0);
			mem_write_data2 <= din.mem_write_data_in(7 downto 0);
			mem_write_data3 <= din.mem_write_data_in(7 downto 0);
		end if;
	end process;

end arch;
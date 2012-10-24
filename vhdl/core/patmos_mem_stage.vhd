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
		mem_data_out_muxed : in std_logic_vector(31 downto 0);
		exout : in execution_out_type;
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
    signal ldt_type							: address_type;
    signal datain						    : std_logic_vector(31 downto 0);
    signal ld_word							: std_logic_vector(31 downto 0);
    signal ld_half							: std_logic_vector(15 downto 0);
    signal ld_byte							: std_logic_vector(7 downto 0);
    signal	s_u								: std_logic;
    signal half_ext, byte_ext				: std_logic_vector(31 downto 0);
begin
	mem_wb : process(clk)
	begin
		if (rising_edge(clk)) then
			dout.data_out <= datain;
			-- forwarding
			dout.reg_write_out      <= din.reg_write_in;
			dout.write_back_reg_out <= din.write_back_reg_in;
			dout.mem_write_data_out <= din.mem_write_data_in;
		--	read_address <= din.alu_result_out;
			ldt_type <= din.adrs_type;
			s_u		<= din.s_u;
		end if;
	end process mem_wb;


	memory0 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.adrs(9 downto 0),
			     mem_write_data0,
			     en0,
			     din.adrs(9 downto 0),
			     dout0);

	memory1 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.adrs(9 downto 0),
			     mem_write_data1,
			     en1,
			     din.adrs(9 downto 0),
			     dout1);

	memory2 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.adrs(9 downto 0),
			     mem_write_data2,
			     en2,
			     din.adrs(9 downto 0),
			     dout2);

	memory3 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     din.adrs(9 downto 0),
			     mem_write_data3,
			     en3,
			     din.adrs(9 downto 0),
			     dout3);
	
	--------------------------- address muxes begin--------------------------		     
	process(din, dout0, dout1, dout2, dout3)
	begin
		ld_word <= dout0 & dout1 & dout2 & dout3;
	end process;
	
	ld_add_half:process(din, dout0, dout1, dout2, dout3)
	begin
		case din.adrs_out(1) is
			when '0' =>
				ld_half <= dout0 & dout1;
			when '1' =>
				ld_half <= dout2 & dout3;
			when others => null;
		end case;
	end process;
	
	process(din, dout0, dout1, dout2, dout3)
	begin
		case din.adrs_out(1 downto 0) is
			when "00" =>
				ld_byte <= dout0;
			when "01" =>
				ld_byte <= dout1;
			when "10" =>
				ld_byte <= dout2;
			when "11" =>
				ld_byte <= dout3;
			when others => null;
		end case;		
	end process;
	--------------------------- address muxes end--------------------------	
	
	--------------------------- sign extension begin--------------------------
	process(ld_half, s_u)
	begin
		if (s_u = '1') then
			half_ext <= std_logic_vector(resize(signed(ld_half), 32));
		else
			half_ext <= std_logic_vector(resize(unsigned(ld_half), 32));
		end if;
	end process;
		
	process(ld_byte, s_u)
	begin
		if (s_u = '1') then
			byte_ext <= std_logic_vector(resize(signed(ld_byte), 32));
		else
			byte_ext <= std_logic_vector(resize(unsigned(ld_byte), 32));
		end if;
	end process;
	--------------------------- sign extension end--------------------------
	
	--------------------------- size muxe begin--------------------------
	process(byte_ext, half_ext, ld_word, ldt_type)
	begin
		case ldt_type is
			when word => 
				dout.data_mem_data_out <= ld_word;
			when half =>
				dout.data_mem_data_out <= half_ext;
			when byte =>
				dout.data_mem_data_out <= byte_ext;
			when others => null;
		end case;
	end process;
	
	--------------------------- size muxe end--------------------------

	process(din)
	begin
		byte_enable0 <= '0';
		byte_enable1 <= '0';
		byte_enable2 <= '0';
		byte_enable3 <= '0';
		case din.adrs(1 downto 0) is
			when "00"   => byte_enable0 <= din.mem_write;
			when "01"   => byte_enable1 <= din.mem_write;
			when "10"   => byte_enable2 <= din.mem_write;
			when "11"   => byte_enable3 <= din.mem_write;
			when others => null;
		end case;
	end process;

	process(din)
	begin
		word_enable0 <= '0';
		word_enable1 <= '0';
		case din.adrs(1) is
			when '0'    => word_enable0 <= din.mem_write;
			when '1'    => word_enable1 <= din.mem_write;
			when others => null;
		end case;
	end process;
		
	process(din, word_enable0, word_enable1, byte_enable0, byte_enable1, byte_enable2, byte_enable3)
	begin
		case din.adrs_type is
			when word => 
				en0             <= din.mem_write;
				en1             <= din.mem_write;
				en2             <= din.mem_write;
				en3             <= din.mem_write;
				mem_write_data0 <= din.mem_write_data_in(31 downto 24);
				mem_write_data1 <= din.mem_write_data_in(23 downto 16);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when half =>
				en0             <= word_enable0;
				en1             <= word_enable0;
				en2             <= word_enable1;
				en3             <= word_enable1;
				mem_write_data0 <= din.mem_write_data_in(15 downto 8);
				mem_write_data1 <= din.mem_write_data_in(7 downto 0);
				mem_write_data2 <= din.mem_write_data_in(15 downto 8);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when byte =>
				en0 <= byte_enable0;
				en1 <= byte_enable1;
				en2 <= byte_enable2;
				en3 <= byte_enable3;
	
				mem_write_data0 <= din.mem_write_data_in(7 downto 0);
				mem_write_data1 <= din.mem_write_data_in(7 downto 0);
				mem_write_data2 <= din.mem_write_data_in(7 downto 0);
				mem_write_data3 <= din.mem_write_data_in(7 downto 0);
			when others => null;
		end case;
	end process;

	process(mem_data_out_muxed, exout)
	begin
		if exout.mem_to_reg_out = '1' then
			dout.data <= mem_data_out_muxed;
			datain <= mem_data_out_muxed;
		else
			dout.data <= exout.alu_result_out;
			datain <= exout.alu_result_out;
		end if;
	end process;
end arch;
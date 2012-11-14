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
		mem_write  : in  std_logic;
		mem_data_out_muxed : in std_logic_vector(31 downto 0);
		exout : in execution_out_type;
		dout : out mem_out_type;
		decdout									: in  decode_out_type
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
    signal ldt_type							 : address_type;
    signal datain						     : std_logic_vector(31 downto 0);
    signal ld_word							 : std_logic_vector(31 downto 0);
    signal ld_half							 : std_logic_vector(15 downto 0);
    signal ld_byte						 	 : std_logic_vector(7 downto 0);
    signal	s_u								 : std_logic;
    signal half_ext, byte_ext				 : std_logic_vector(31 downto 0);
    
    ------ stack cache
    signal sc_en0, sc_en1, sc_en2, sc_en3   : std_logic;
    signal sc_word_enable0, sc_word_enable1 : std_logic;
    signal sc_byte_enable0, sc_byte_enable1 : std_logic;
    signal sc_byte_enable2, sc_byte_enable3 : std_logic;
    signal sc_read_data0, sc_read_data1		 : std_logic_vector(7 downto 0);
    signal sc_read_data2, sc_read_data3		 : std_logic_vector(7 downto 0);
	signal sc_write_data0, sc_write_data1	 : std_logic_vector(7 downto 0);
	signal sc_write_data2, sc_write_data3	 : std_logic_vector(7 downto 0);
    signal sc_ld_word						 : std_logic_vector(31 downto 0);
    signal sc_ld_half						 : std_logic_vector(15 downto 0);
    signal sc_ld_byte					 	 : std_logic_vector(7 downto 0);
    signal sc_half_ext, sc_byte_ext			 : std_logic_vector(31 downto 0);
    signal sc_data_out						 : std_logic_vector(31 downto 0);
    signal sc_lm_data						 : std_logic_vector(31 downto 0);
    
    signal state 							 : sc_state;
    signal head, tail, head_tail			 : std_logic_vector(sc_depth - 1 downto 0);

	signal spill, fill						 : std_logic;


begin
	mem_wb : process(clk)
	begin
		if (rising_edge(clk)) then
			dout.data_out <= datain;
			-- forwarding
			dout.reg_write_out      <= exout.reg_write or exout.mem_to_reg;
			dout.write_back_reg_out <= exout.write_back_reg;
			ldt_type <= decdout.adrs_type;
			s_u		<= decdout.s_u;
		end if;
	end process mem_wb;

	---------------------------------------------- stack cache
--	        clk       	             : in std_logic;
--        wr_address               : in std_logic_vector(addr_width -1 downto 0);
--        data_in                  : in std_logic_vector(width -1 downto 0); -- store
--        write_enable             : in std_logic;
--        rd_address               : in std_logic_vector(addr_width - 1 downto 0);
--        data_out                 : out std_logic_vector(width -1 downto 0) -- load
	sc0: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     head_tail,
			     sc_write_data0,
			     sc_en0,
			     head_tail,
			     sc_read_data0);
 
	sc1: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     head_tail,
			     sc_write_data1,
			     sc_en1,
			     head_tail,
			     sc_read_data0);
			     
	sc2: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     head_tail,
			     sc_write_data2,
			     sc_en2,
			     head_tail,
			     sc_read_data0);
			     
	sc3: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     head_tail,
			     sc_write_data3,
			     sc_en3,
			     head_tail,
			     sc_read_data0);		
			     
	process(mem_write_data0, mem_write_data1,  mem_write_data2, mem_write_data3) -- write to stack cache from main memory or register
	begin
		--if (exout.sc_write_not_reg = '1') then -- normal store
			sc_write_data0 <=  mem_write_data0;
			sc_write_data1 <=  mem_write_data1;
			sc_write_data2 <=  mem_write_data2;
			sc_write_data3 <=  mem_write_data3;
		if (fill = '1') then 				-- fill stack cache
		end if;	
		
	end process;    
	
	
	process(exout)
	begin
		head_tail <= exout.head;
--		head_tail <= (others => '0');
--		if (spill = '1') then
--			head_tail <= head;
--		elsif (fill = '1') then
--			head_tail <= tail;
--		end if;
	end process;			          
--			     
--			     
--	process(clk, rst, spill, fill) -- adjust head/tail
--	begin 
--		if (rst='1') then
--			state <= init;
--
--		elsif rising_edge(clk) then
--			case state is
--				when init => 
--					spill <= '0';
--					fill  <= '0';	
--					head <= exout.head;
--					tail <= exout.tail;
--			--	dout.stall <= '0';
--					if (spill = '1') then
--						state <= spill;
--					elsif (fill = '1') then 
--						state <= fill;
--					else 
--						state <= init;
--					end if;
--				when spill =>
--					if (spill = '1') then
--						--tail <= ; update tail
--						state <= spill;
--					else
--						state <= init;
--					end if;
--				when fill  => 
--					if (spill = '1') then
--						--tail <= ; update tail
--						state <= fill;
--					else
--						state <= init;
--					end if;
--			end case;	
--		end if;
--	end process;		  
--	
--	
--	--	

	-----------------------------------------------
	   
	memory0 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout.adrs(9 downto 0),
			     mem_write_data0,
			     en0,
			     exout.adrs(9 downto 0),
			     dout0);

	memory1 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout.adrs(9 downto 0),
			     mem_write_data1,
			     en1,
			     exout.adrs(9 downto 0),
			     dout1);

	memory2 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout.adrs(9 downto 0),
			     mem_write_data2,
			     en2,
			     exout.adrs(9 downto 0),
			     dout2);

	memory3 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout.adrs(9 downto 0),
			     mem_write_data3,
			     en3,
			     exout.adrs(9 downto 0),
			     dout3);
	
	--------------------------- address muxes begin--------------------------		     
	process( dout0, dout1, dout2, dout3, sc_read_data0, sc_read_data1, sc_read_data2, sc_read_data3)
	begin
		ld_word <= dout0 & dout1 & dout2 & dout3;
		sc_ld_word <= sc_read_data0 & sc_read_data1 & sc_read_data2 & sc_read_data3; 
	end process;
	
	ld_add_half:process(exout, dout0, dout1, dout2, dout3, sc_read_data0, sc_read_data1, sc_read_data2, sc_read_data3)
	begin
		case exout.adrs_reg(1) is
			when '0' =>
				ld_half <= dout0 & dout1;
				sc_ld_half <= sc_read_data0 & sc_read_data1;
			when '1' =>
				ld_half <= dout2 & dout3;
				sc_ld_half <= sc_read_data2 & sc_read_data3;
			when others => null;
		end case;
	end process;
	
	process(exout, dout0, dout1, dout2, dout3, sc_read_data0, sc_read_data1, sc_read_data2, sc_read_data3)
	begin
		case exout.adrs_reg(1 downto 0) is
			when "00" =>
				ld_byte <= dout0;
				sc_ld_byte <= sc_read_data0;
			when "01" =>
				ld_byte <= dout1;
				sc_ld_byte <= sc_read_data1;
			when "10" =>
				ld_byte <= dout2;
				sc_ld_byte <= sc_read_data2;
			when "11" =>
				ld_byte <= dout3;
				sc_ld_byte <= sc_read_data3;
			when others => null;
		end case;		
	end process;
	--------------------------- address muxes end--------------------------	
	
	--------------------------- sign extension begin--------------------------
	process(ld_half, sc_ld_half, s_u)
	begin
		if (s_u = '1') then
			half_ext <= std_logic_vector(resize(signed(ld_half), 32));
			sc_half_ext <= std_logic_vector(resize(signed(sc_ld_half), 32));
		else
			half_ext <= std_logic_vector(resize(unsigned(ld_half), 32));
			sc_half_ext <= std_logic_vector(resize(unsigned(sc_ld_half), 32));
		end if;
	end process;
		
	process(ld_byte, sc_ld_byte, s_u)
	begin
		if (s_u = '1') then
			byte_ext <= std_logic_vector(resize(signed(ld_byte), 32));
			sc_byte_ext <= std_logic_vector(resize(signed(sc_ld_byte), 32));
		else
			byte_ext <= std_logic_vector(resize(unsigned(ld_byte), 32));
			sc_byte_ext <= std_logic_vector(resize(unsigned(sc_ld_byte), 32));
		end if;
	end process;
	--------------------------- sign extension end--------------------------
	
	--------------------------- size muxe begin--------------------------
	process(byte_ext, half_ext, ld_word, ldt_type)
	begin
		case ldt_type is
			when word => 
				dout.data_mem_data_out <= ld_word;
				sc_data_out	   <= sc_ld_word;	
			when half =>
				dout.data_mem_data_out <= half_ext;
				sc_data_out	   <= sc_half_ext;
			when byte =>
				dout.data_mem_data_out <= byte_ext;
				sc_data_out	   <= sc_byte_ext;
			when others => null;
		end case;
	end process;
	
	--------------------------- size muxe end--------------------------

	process(exout, mem_write)
	begin
		byte_enable0 <= '0';
		byte_enable1 <= '0';
		byte_enable2 <= '0';
		byte_enable3 <= '0';
		sc_byte_enable0 <= '0';
		sc_byte_enable0 <= '0';
		sc_byte_enable0 <= '0';
		sc_byte_enable0 <= '0';
		case exout.adrs(1 downto 0) is
			when "00"   => byte_enable0 <= mem_write;
							sc_byte_enable0 <= exout.sc_write_not_reg;
			when "01"   => byte_enable1 <= mem_write;
							sc_byte_enable1 <= exout.sc_write_not_reg;
			when "10"   => byte_enable2 <= mem_write;
							sc_byte_enable2 <= exout.sc_write_not_reg;
			when "11"   => byte_enable3 <= mem_write;
							sc_byte_enable3 <= exout.sc_write_not_reg;
			when others => null;
		end case;
	end process;

	process(exout, mem_write)
	begin
		word_enable0 <= '0';
		word_enable1 <= '0';
		sc_word_enable0 <= '0';
		sc_word_enable1 <= '0';
		case exout.adrs(1) is
			when '0'    => word_enable0 <= mem_write;
							sc_word_enable0 <= exout.sc_write_not_reg;
			when '1'    => word_enable1 <= mem_write;
							sc_word_enable1 <= exout.sc_write_not_reg;
			when others => null;
		end case;
	end process;
		
	process(word_enable0, word_enable1, byte_enable0, byte_enable1, byte_enable2, byte_enable3, decdout, exout, mem_write)
	begin
		case decdout.adrs_type is
			when word => 
				en0             <= mem_write;
				en1             <= mem_write;
				en2             <= mem_write;
				en3             <= mem_write;
				
				sc_en0			<= exout.sc_write_not_reg;
				sc_en1			<= exout.sc_write_not_reg;
				sc_en2			<= exout.sc_write_not_reg;
				sc_en3			<= exout.sc_write_not_reg;
				
				mem_write_data0 <= exout.mem_write_data(31 downto 24);
				mem_write_data1 <= exout.mem_write_data(23 downto 16);
				mem_write_data2 <= exout.mem_write_data(15 downto 8);
				mem_write_data3 <= exout.mem_write_data(7 downto 0);
			when half =>
				en0             <= word_enable0;
				en1             <= word_enable0;
				en2             <= word_enable1;
				en3             <= word_enable1;
				
				sc_en0          <= sc_word_enable0;
				sc_en1          <= sc_word_enable0;
				sc_en2          <= sc_word_enable1;
				sc_en3          <= sc_word_enable1;
				
				mem_write_data0 <= exout.mem_write_data(15 downto 8);
				mem_write_data1 <= exout.mem_write_data(7 downto 0);
				mem_write_data2 <= exout.mem_write_data(15 downto 8);
				mem_write_data3 <= exout.mem_write_data(7 downto 0);
			when byte =>
				en0 <= byte_enable0;
				en1 <= byte_enable1;
				en2 <= byte_enable2;
				en3 <= byte_enable3;
				
				sc_en0 <= sc_byte_enable0;
				sc_en1 <= sc_byte_enable1;
				sc_en2 <= sc_byte_enable2;
				sc_en3 <= sc_byte_enable3;
	
				mem_write_data0 <= exout.mem_write_data(7 downto 0);
				mem_write_data1 <= exout.mem_write_data(7 downto 0);
				mem_write_data2 <= exout.mem_write_data(7 downto 0);
				mem_write_data3 <= exout.mem_write_data(7 downto 0);
			when others => null;
		end case;
	end process;
	
	process(exout, mem_data_out_muxed, sc_data_out) -- ld from stack cache or  io/scratchpad
	begin
		sc_lm_data <= mem_data_out_muxed;
		if (exout.lm_read = '1') then 
			sc_lm_data <= mem_data_out_muxed;
		elsif (exout.sc_read = '1') then
			sc_lm_data <= sc_data_out;
		end if;
	end process;
	
-- write back
	process(mem_data_out_muxed, exout, sc_lm_data)
	begin
		if exout.mem_to_reg = '1' then
			dout.data <= sc_lm_data;--mem_data_out_muxed; --
			datain <= sc_lm_data;--mem_data_out_muxed;--
		else
			dout.data <= exout.alu_result_reg;
			datain <= exout.alu_result_reg;
		end if;
	end process;
end arch;
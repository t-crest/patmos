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
		clk  								: in  std_logic;
		rst  								: in  std_logic;
		mem_write  							: in  std_logic;
		mem_data_out_muxed 					: in std_logic_vector(31 downto 0);
		exout_reg 							: in execution_reg;
		exout_not_reg 						: in execution_not_reg;
		dout 								: out mem_out_type;
		decdout								: in  decode_out_type
	);
end entity patmos_mem_stage;

architecture arch of patmos_mem_stage is
	--signal en0, en1, en2, en3               : std_logic;
	signal en								 : std_logic_vector(3 downto 0);

	-- MS: here and also further down to either:
	-- use an array of 4 bytes
	-- or (better) define a 32-bit std_logic_vector and use parts of the vector where needed
	signal lm_dout								 : std_logic_vector(31 downto 0);
	signal mem_write_data					 : std_logic_vector(31 downto 0);

	signal byte_enable						 : std_logic_vector(3 downto 0);
	signal word_enable					     : std_logic_vector(1 downto 0);
    signal ldt_type							 : address_type;
    signal datain						     : std_logic_vector(31 downto 0);
    signal ld_word							 : std_logic_vector(31 downto 0);
    signal ld_half							 : std_logic_vector(15 downto 0);
    signal ld_byte						 	 : std_logic_vector(7 downto 0);
    signal	s_u								 : std_logic;
    signal half_ext, byte_ext				 : std_logic_vector(31 downto 0);
  	signal exout_reg_adr, prev_exout_reg_adr: std_logic_vector(31 downto 0);
    signal mem_write_data_stall				 : std_logic_vector(31 downto 0);
    signal prev_mem_write_data_reg			 : std_logic_vector(31 downto 0);
    signal prev_en_reg						 : std_logic_vector(3 downto 0);
    signal en_reg							 : std_logic_vector(3 downto 0);
    
    -- Main Memory
    
    signal mm_write_data					 : std_logic_vector(31 downto 0);
	signal mm_read_data						 : std_logic_vector(31 downto 0);
    signal mm_en							 : std_logic_vector(3 downto 0);
    signal mm_read_add, mm_write_add		 : std_logic_vector(9 downto 0);
    signal mm_en_spill						 : std_logic_vector(3 downto 0);
    signal mm_spill							 : std_logic_vector(3 downto 0);
    
    ------ stack cache
    -- MS: what about using arrays for those xxx0 - xxx3 signals?
    signal sc_en							 : std_logic_vector(3 downto 0);
    signal sc_word_enable					 : std_logic_vector(1 downto 0);
    signal sc_byte_enable					 : std_logic_vector(3 downto 0);
    signal sc_read_data						 : std_logic_vector(31 downto 0);
	signal sc_write_data					 : std_logic_vector(31 downto 0);
    signal sc_ld_word						 : std_logic_vector(31 downto 0);
    signal sc_ld_half						 : std_logic_vector(15 downto 0);
    signal sc_ld_byte					 	 : std_logic_vector(7 downto 0);
    signal sc_half_ext, sc_byte_ext			 : std_logic_vector(31 downto 0);
    signal sc_data_out						 : std_logic_vector(31 downto 0);
    signal sc_lm_data						 : std_logic_vector(31 downto 0);
  
  	signal sc_read_add, sc_write_add		 : std_logic_vector(sc_depth - 1 downto 0);
    signal state_reg, next_state			 : sc_state;
    signal mem_top, mem_top_next			 : std_logic_vector(31 downto 0);
	signal sc_fill							 : std_logic_vector(3 downto 0);
	signal sc_en_fill						 : std_logic_vector(3 downto 0);

	signal spill, fill						 : std_logic;
	signal stall							 : std_logic;	
	signal nspill_fill, nspill_fill_next	 : std_logic_vector(31 downto 0);

--	signal test								 : signed(sc_depth - 1 downto 0);
begin
	mem_wb : process(clk)
	begin
		if (rising_edge(clk)) then
			dout.data_out <= datain;
			-- forwarding
			dout.reg_write_out      <= exout_reg.reg_write or exout_reg.mem_to_reg;
			dout.write_back_reg_out <= exout_reg.write_back_reg;
			ldt_type <= decdout.adrs_type;
			s_u		<= decdout.s_u;
		end if;
	end process mem_wb;

	process(exout_reg_adr, spill, fill, mem_top, mm_spill, mm_en,
		sc_read_data, mem_write_data_stall
	) --SA: Main memory read/write address, normal load/store or fill/spill
	begin
		mm_read_add <= exout_reg_adr(9 downto 0);
		mm_write_add <= exout_reg_adr(9 downto 0);
		mm_en_spill <= mm_en;
		mm_write_data <= mem_write_data_stall;
		if (spill = '1' or fill = '1') then	
			mm_read_add <= mem_top(9 downto 0);
			mm_en_spill <= mm_spill; -- this is for spilling ( writing to main memory)
			mm_write_data <= sc_read_data;
			--sc_write_add <= ; -- spill
		end if;
	end process;

	--- main memory for simulation
	mm0: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     mm_write_add,
			     mm_write_data(7 downto 0),
			     mm_en_spill(0),
			     mm_read_add,
			     mm_read_data(7 downto 0));
 
	mm1: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     mm_write_add,
			     mm_write_data(15 downto 8),
			     mm_en_spill(1),
			     mm_read_add,
			     mm_read_data(15 downto 8));
			     
	mm2: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     mm_write_add,
			     mm_write_data(23 downto 16),
			     mm_en_spill(2),
			     mm_read_add,
			     mm_read_data(23 downto 16));
			     
	mm3: entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     mm_write_add,
			     mm_write_data(31 downto 24),
			     mm_en_spill(3),
			     mm_read_add,
			     mm_read_data(31 downto 24));		
	
	---------------------------------------------- stack cache
--	        clk       	             : in std_logic;
--        wr_address               : in std_logic_vector(addr_width -1 downto 0);
--        data_in                  : in std_logic_vector(width -1 downto 0); -- store
--        write_enable             : in std_logic;
--        rd_address               : in std_logic_vector(addr_width - 1 downto 0);
--        data_out                 : out std_logic_vector(width -1 downto 0) -- load
	process(exout_reg_adr, spill, fill, mem_top, sc_fill, sc_en,
		mem_write_data_stall, mm_read_data) --SA: Stack cache read/write address, normal load/store or fill/spill
	begin
		sc_read_add <= exout_reg_adr(sc_depth - 1 downto 0);
		sc_write_add <= exout_reg_adr(sc_depth - 1 downto 0);
		sc_en_fill <= sc_en;
		sc_write_data <= mem_write_data_stall;
		if (spill = '1' or fill = '1') then	
			sc_read_add <= mem_top(sc_depth - 1 downto 0) and SC_MASK;
			sc_en_fill <= sc_fill; -- this is for filling!
			sc_write_data <= mm_read_data;
			--sc_write_add <= ; -- spill
		end if;
	end process;
	
	sc0: entity work.patmos_data_memory(arch)
		generic map(8, sc_depth)
		port map(clk,
			     sc_write_add,
			     sc_write_data(7 downto 0),
			     sc_en_fill(0),
			     sc_read_add,
			     sc_read_data(7 downto 0));
 
	sc1: entity work.patmos_data_memory(arch)
		generic map(8, sc_depth)
		port map(clk,
			     sc_write_add,
			     sc_write_data(15 downto 8),
			     sc_en_fill(1),
			     sc_read_add,
			     sc_read_data(15 downto 8));
			     
	sc2: entity work.patmos_data_memory(arch)
		generic map(8, sc_depth)
		port map(clk,
			     sc_write_add,
			     sc_write_data(23 downto 16),
			     sc_en_fill(2),
			     sc_read_add,
			     sc_read_data(23 downto 16));
			     
	sc3: entity work.patmos_data_memory(arch)
		generic map(8, sc_depth)
		port map(clk,
			     sc_write_add,
			     sc_write_data(31 downto 24),
			     sc_en_fill(3),
			     sc_read_add,
			     sc_read_data(31 downto 24));		    

	process(clk, rst)
	begin 
		if rst='1' then
			state_reg <= init;
			--spill <= '0';
			fill <= '0';
			mem_top <= "00000000000000000000000111110100";
		elsif rising_edge(clk) then
			state_reg 	<= next_state;
			mem_top		<= mem_top_next;
			nspill_fill <= nspill_fill_next;
		end if;
	end process;

	process(state_reg, exout_not_reg, spill, fill) -- adjust tail
	begin 
		next_state <= state_reg;
		case state_reg is
			when init => 
				if (exout_not_reg.spill = '1') then
					next_state <= spill_state;
				elsif(exout_not_reg.fill = '1') then 
					next_state <= fill_state;
				else 
					next_state <= init;
				end if;
			when spill_state =>
				if (spill = '1') then
					next_state <= spill_state;
				else
					next_state <= init;
				end if;
			when fill_state  => 
				if (fill = '1') then
					next_state <= fill_state;
				else
					next_state <= init;
				end if;
		end case;	
	end process;		  
	
	-- Output process
	process(state_reg, exout_not_reg, mem_top)
	begin
		mem_top_next <= mem_top;
		case state_reg is
			when init =>
				
				nspill_fill_next <= exout_not_reg.nspill_fill;
				dout.stall <= '0';
			when spill_state =>
				if ((signed(nspill_fill) - 1) >= 0) then
					mem_top_next <= std_logic_vector(signed(mem_top) - 1); 
					nspill_fill_next <= std_logic_vector(signed(nspill_fill) - 1);
			
					spill <= '1';
					stall <= '1';
				else
					spill <= '0';
					stall <= '0';
					nspill_fill_next <= exout_not_reg.nspill_fill;
				end if;
				 
			when fill_state =>
				nspill_fill_next <= exout_not_reg.nspill_fill;
				--mem_top <= 
		end case;
	end process;
	
	process(mem_top)
	begin
		dout.mem_top <= mem_top;
	end process;
	-----------------------------------------------
	-- MS: If a registered address from EX is used here and there is an address
	-- register in the memory, are we now moving the MEM stage into WB?
	-- SA: The address is not registered, in case there is the stall the address
	-- should be registered, I can change the name though
	-- MS: a non-registered signal shall not end with _reg.
	memory0 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout_reg_adr(9 downto 0),-- exout_not_reg.adrs(9 downto 0),
			     mem_write_data_stall(7 downto 0),--mem_write_data0,
			     en_reg(0),
			     exout_reg_adr(9 downto 0), --exout_not_reg.adrs(9 downto 0),
			     lm_dout(7 downto 0));

	memory1 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout_reg_adr(9 downto 0), --exout_not_reg.adrs(9 downto 0),
			     mem_write_data_stall(15 downto 8), --mem_write_data1,
			     en_reg(1),
			     exout_reg_adr(9 downto 0),--exout_not_reg.adrs(9 downto 0),
			     lm_dout(15 downto 8));

	memory2 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout_reg_adr(9 downto 0),--exout_not_reg.adrs(9 downto 0),
			     mem_write_data_stall(23 downto 16), --mem_write_data2,
			     en_reg(2),
			     exout_reg_adr(9 downto 0),--exout_not_reg.adrs(9 downto 0),
			     lm_dout(23 downto 16));

	memory3 : entity work.patmos_data_memory(arch)
		generic map(8, 10)
		port map(clk,
			     exout_reg_adr(9 downto 0), --exout_not_reg.adrs(9 downto 0),
			     mem_write_data_stall(31 downto 24), --
			     en_reg(3),
			     exout_reg_adr(9 downto 0), --exout_not_reg.adrs(9 downto 0),
			     lm_dout(31 downto 24));
	
	process(clk) --to register the enable and address and data of memory in case of stall
	begin
	--	if (rst = '1') then
--			exout_reg_adr		<= exout_not_reg.adrs;
--			mem_write_data0_stall <= mem_write_data0;
--			mem_write_data1_stall <= mem_write_data1;
--			mem_write_data2_stall <= mem_write_data2;
--			mem_write_data3_stall <= mem_write_data3;
		if rising_edge(clk) then
				prev_exout_reg_adr <= exout_not_reg.adrs;
				prev_mem_write_data_reg <= mem_write_data;
				prev_en_reg			<= en;
		end if;	
	end process;
	
	process(stall, en, prev_en_reg,
			exout_not_reg, mem_write_data, prev_exout_reg_adr, 
			prev_mem_write_data_reg)
	begin
		if (stall = '1') then
			exout_reg_adr		<= prev_exout_reg_adr;
			mem_write_data_stall <= prev_mem_write_data_reg;
			en_reg				<= prev_en_reg;
		else
			exout_reg_adr		<= exout_not_reg.adrs;
			mem_write_data_stall <= mem_write_data;
			en_reg				<= en;
		end if;
	end process;
	
	--	decode : process(clk, alu_func)
--	begin
--		if rising_edge(clk) then
--			dout <= comb_out;
--			prev_dout <= comb_out;
--			if(memout.stall = '1') then
--				dout <= prev_dout;
--			end if;
--		end if;
--	end process decode;
	--------------------------- address muxes begin--------------------------		     
	process( lm_dout, sc_read_data)
	begin
		ld_word <= lm_dout(7 downto 0) & lm_dout(15 downto 8) & lm_dout(23 downto 16) & lm_dout(31 downto 24);
		sc_ld_word <= sc_read_data(7 downto 0) & sc_read_data(15 downto 8) & sc_read_data(23 downto 16) & sc_read_data(31 downto 24); 
	end process;
	
	ld_add_half:process(exout_reg, lm_dout, sc_read_data)
	begin
		case exout_reg.adrs_reg(1) is
			when '0' =>
				ld_half <= lm_dout(7 downto 0) & lm_dout(15 downto 8);
				sc_ld_half <= sc_read_data(7 downto 0) & sc_read_data(15 downto 8);
			when '1' =>
				ld_half <= lm_dout(23 downto 16) & lm_dout(31 downto 24);
				sc_ld_half <= sc_read_data(23 downto 16) & sc_read_data(31 downto 24);
			when others => null;
		end case;
	end process;
	
	process(exout_reg, lm_dout, sc_read_data)
	begin
		case exout_reg.adrs_reg(1 downto 0) is
			when "00" =>
				ld_byte <= lm_dout(7 downto 0);
				sc_ld_byte <= sc_read_data(7 downto 0);
			when "01" =>
				ld_byte <= lm_dout(15 downto 8);
				sc_ld_byte <= sc_read_data(15 downto 8);
			when "10" =>
				ld_byte <= lm_dout(23 downto 16);
				sc_ld_byte <= sc_read_data(23 downto 16);
			when "11" =>
				ld_byte <= lm_dout(31 downto 24);
				sc_ld_byte <= sc_read_data(31 downto 24);
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
	process(byte_ext, half_ext, ld_word, ldt_type, sc_ld_word, sc_half_ext, sc_byte_ext)
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

	process(exout_not_reg, mem_write)
	begin
		byte_enable(3 downto 0) <= (others =>'0');
		sc_byte_enable(3 downto 0) <= (others =>'0');
		case exout_not_reg.adrs(1 downto 0) is
			when "00"   => byte_enable(0) <= mem_write;
							sc_byte_enable(0) <= exout_not_reg.sc_write_not_reg;
			when "01"   => byte_enable(1) <= mem_write;
							sc_byte_enable(1) <= exout_not_reg.sc_write_not_reg;
			when "10"   => byte_enable(2) <= mem_write;
							sc_byte_enable(2) <= exout_not_reg.sc_write_not_reg;
			when "11"   => byte_enable(3) <= mem_write;
							sc_byte_enable(3) <= exout_not_reg.sc_write_not_reg;
			when others => null;
		end case;
	end process;

	process(exout_not_reg, mem_write)
	begin
		word_enable(1 downto 0) <= (others => '0');
		sc_word_enable(1 downto 0) <= (others => '0');
		case exout_not_reg.adrs(1) is
			when '0'    => word_enable(0) <= mem_write;
							sc_word_enable(0) <= exout_not_reg.sc_write_not_reg;
			when '1'    => word_enable(1) <= mem_write;
							sc_word_enable(1) <= exout_not_reg.sc_write_not_reg;
			when others => null;
		end case;
	end process;
		
	process(word_enable, byte_enable, decdout, exout_not_reg, mem_write, sc_word_enable)
	begin
		case decdout.adrs_type is
			when word => 
				en(3 downto 0)             <= mem_write & mem_write & mem_write & mem_write;
				
				sc_en(3 downto 0)  			<= exout_not_reg.sc_write_not_reg & exout_not_reg.sc_write_not_reg & exout_not_reg.sc_write_not_reg & exout_not_reg.sc_write_not_reg;
				
				mem_write_data(7 downto 0) <= exout_not_reg.mem_write_data(31 downto 24);
				mem_write_data(15 downto 8) <= exout_not_reg.mem_write_data(23 downto 16);
				mem_write_data(23 downto 16) <= exout_not_reg.mem_write_data(15 downto 8);
				mem_write_data(31 downto 24) <= exout_not_reg.mem_write_data(7 downto 0);
			when half =>
				
				en(3 downto 2)             <= word_enable(1) & word_enable(1);
				en(1 downto 0)             <= word_enable(0) & word_enable(0);
				
				sc_en(3 downto 2)          <= sc_word_enable(1) & sc_word_enable(1);
				sc_en(1 downto 0)          <= sc_word_enable(0) & sc_word_enable(0);
				
				mem_write_data(7 downto 0) <= exout_not_reg.mem_write_data(15 downto 8);
				mem_write_data(15 downto 8) <= exout_not_reg.mem_write_data(7 downto 0);
				mem_write_data(23 downto 16) <= exout_not_reg.mem_write_data(15 downto 8);
				mem_write_data(31 downto 24) <= exout_not_reg.mem_write_data(7 downto 0);
			when byte =>
				en(3 downto 0) <= byte_enable(3 downto 0);
				
				sc_en(3 downto 0) <= sc_byte_enable(3 downto 0);
	
				mem_write_data(7 downto 0) <= exout_not_reg.mem_write_data(7 downto 0);
				mem_write_data(15 downto 8) <= exout_not_reg.mem_write_data(7 downto 0);
				mem_write_data(23 downto 16) <= exout_not_reg.mem_write_data(7 downto 0);
				mem_write_data(31 downto 24) <= exout_not_reg.mem_write_data(7 downto 0);
			when others => null;
		end case;
	end process;
	
	process(exout_reg, mem_data_out_muxed, sc_data_out) -- ld from stack cache or  io/scratchpad
	begin
		sc_lm_data <= mem_data_out_muxed;
		if (exout_reg.lm_read = '1') then 
			sc_lm_data <= mem_data_out_muxed;
		elsif (exout_reg.sc_read = '1') then
			sc_lm_data <= sc_data_out;
		end if;
	end process;
	
-- write back
	process(mem_data_out_muxed, exout_reg, sc_lm_data)
	begin
		if exout_reg.mem_to_reg = '1' then
			dout.data <= sc_lm_data;--mem_data_out_muxed; --
			datain <= sc_lm_data;--mem_data_out_muxed;--
		else
			dout.data <= exout_reg.alu_result_reg;
			datain <= exout_reg.alu_result_reg;
		end if;
	end process;
end arch;
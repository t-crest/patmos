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
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- Author: Sahar Abbaspour
--------------------------------------------------------------------------------


------------------------------------------
--general purpose registers
------------------------------------------

library std;
use std.textio.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_register_file is          --general purpose registers
	port(
		clk           : in  std_logic;
		rst           : in  std_logic;
		read_address1 : in  std_logic_vector(4 downto 0);
		read_address2 : in  std_logic_vector(4 downto 0);
		write_address : in  std_logic_vector(4 downto 0);
		read_data1    : out std_logic_vector(31 downto 0);
		read_data2    : out std_logic_vector(31 downto 0);
		write_data    : in  std_logic_vector(31 downto 0);
		write_enable  : in  std_logic
	);
end entity patmos_register_file;

architecture arch of patmos_register_file is
	type ram_type is array (0 to 31) of std_logic_vector(31 downto 0);
	signal ram : ram_type;

	signal wr_addr_reg  : std_logic_vector(4 downto 0);
	signal wr_data_reg  : std_logic_vector(31 downto 0);
	signal wr_en_reg    : std_logic;
	signal rd_addr_reg1 : std_logic_vector(4 downto 0);
	signal rd_addr_reg2 : std_logic_vector(4 downto 0);
	signal fwd1, fwd2   : std_logic;

begin
	process(clk)
		variable l : line;

	begin
		if rising_edge(clk) then
			-- Edgar: Moved write logic here to get rid of Xilinx inferred latches
			if (write_enable = '1') then
				ram(to_integer(unsigned(write_address))) <= write_data;
-- Edgar: Might also try this cleaner version (should work in Xilinx, need to check altera)
-- The writes to R0 could be disabled, this way we could get rid of mux to drive 0 on read of R0.
--				if write_address = read_address1 then
--					read_data1 <= write_data;
--				end if;
--				if write_address = read_address2 then
--					read_data2 <= write_data;
--				end if;
--			else
--				read_data1 <= ram(to_integer(unsigned(read_address1)));
--				read_data2 <= ram(to_integer(unsigned(read_address2)));
			end if;
			wr_addr_reg  <= write_address;
			wr_data_reg  <= write_data;
			wr_en_reg    <= write_enable;
			rd_addr_reg1 <= read_address1;
			rd_addr_reg2 <= read_address2;
			if read_address1 = write_address and write_enable = '1' then
				fwd1 <= '1';
			else
				fwd1 <= '0';
			end if;
			if read_address2 = write_address and write_enable = '1' then
				fwd2 <= '1';
			else
				fwd2 <= '0';
			end if;
			--pragma synthesis_off
			for i in 0 to 31 loop
				write(l, integer'image(to_integer(signed(ram(i)))));
				write(l, ' ');
			end loop;
			writeline(output, l);
		--pragma synthesis_on
		end if;
	end process;

	process(ram, wr_addr_reg, wr_data_reg, wr_en_reg, rd_addr_reg1, rd_addr_reg2, fwd1, fwd2)
	begin
-- Edgar: Xilinx inferred latches, so the write logic moved to synchronous process
--		if wr_en_reg = '1' then
--			ram(to_integer(unsigned(wr_addr_reg))) <= wr_data_reg;
--		end if;

		if rd_addr_reg1 = "00000" then
			read_data1 <= (others => '0');
		elsif fwd1 = '1' then
			read_data1 <= wr_data_reg;
		else
			read_data1 <= ram(to_integer(unsigned(rd_addr_reg1)));
		end if;
		if rd_addr_reg2 = "00000" then
			read_data2 <= (others => '0');
		elsif fwd2 = '1' then
			read_data2 <= wr_data_reg;
		else
			read_data2 <= ram(to_integer(unsigned(rd_addr_reg2)));
		end if;
	end process;

end arch;


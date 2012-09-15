--
--  Copyright 2011 Martin Schoeberl <masca@imm.dtu.dk>,
--                 Technical University of Denmark, DTU Informatics. 
--  All rights reserved.
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


--
--	leros_de2-70.vhd
--
--	top level for Altera DE2-70 board
--
--	2011-02-20	creation
--
--


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.leros_types.all;


entity leros_core is

port (
		clk                : in  std_logic;
		led                : out std_logic;
		txd                : out std_logic;
		rxd                : in  std_logic
		;
		-- dma controll interface
		dma_addr_special_i : out std_logic;
		dma_addr_i         : out std_logic_vector(4 downto 0);
		dma_rd_i           : out std_logic;
		dma_rd_data_i      : in  std_logic_vector(31 downto 0);
		dma_wr_i           : out std_logic;
		dma_wr_data_i      : out std_logic_vector(31 downto 0)
);
end leros_core;

architecture rtl of leros_core is

	-- for generation of internal reset
	signal int_res			: std_logic;
	signal res_cnt			: unsigned(2 downto 0) := "000";	-- for the simulation

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

	signal ioout : io_out_type;
	signal ioin : io_in_type;

	signal uart_rd, uart_wr			: std_logic;
	signal outp : std_logic_vector(15 downto 0);
	signal rddata, dma_data : std_logic_vector(15 downto 0);
	signal btn_reg : std_logic_vector(3 downto 0);
	
	
begin

--
--	internal reset generation
--	should include the PLL lock signal
--
process(clk)
begin
	if rising_edge(clk) then
		if (res_cnt/="111") then
			res_cnt <= res_cnt+1;
		end if;
		int_res <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
	end if;
end process;


	cpu: entity work.leros
		port map(clk, int_res, ioout, ioin);

--	ioin.rddata(15 downto 4) <= (others => '0');
	
	ua: entity work.uart generic map (
		clk_freq => 200000000,
		baud_rate => 115200,
		txf_depth => 1,
		rxf_depth => 1
	)
	port map(
		clk => clk,
		reset => int_res,

		address => ioout.addr(0),
		wr_data => ioout.wrdata,
		rd => uart_rd,
		wr => uart_wr,
		rd_data => rddata,

		txd	 => txd,
		rxd	 => rxd
	);

uart_wr <= '1' when ioout.wr = '1' and ioout.addr(7) = '0' and ioout.addr(1) = '1'
	 else '0';
uart_rd <= '1' when ioout.rd = '1' and ioout.addr(7) = '0' and ioout.addr(1) = '1'
	 else '0';

dma_wr_i <= '1' when ioout.wr = '1' and ioout.addr(7) = '1'
	 else '0';
dma_rd_i <= '1' when ioout.rd = '1' and ioout.addr(7) = '1'
	 else '0';

dma_addr_special_i  <= ioout.addr(dma_addr_i'high+1);
dma_addr_i  <= ioout.addr(dma_addr_i'high downto dma_addr_i'low);
dma_wr_data_i <= X"0000" & ioout.wrdata;

ioin.rddata <= dma_rd_data_i(15 downto 0) when ioout.addr(7) = '1' else
    "000000000000" & btn_reg when ioout.addr(1) = '0' else
      rddata ;

process(clk)
begin

	if rising_edge(clk) then
		if ioout.wr='1' and ioout.addr(7) = '0' and ioout.addr(1) = '0' then
			outp <= ioout.wrdata;
		end if;
		led <= outp(0);
		btn_reg <= (others=>'0');
	end if;
end process;

end rtl;

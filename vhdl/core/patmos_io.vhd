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
use work.sc_pack.all;
use work.patmos_config_global.all;
use work.patmos_config.all;


entity patmos_io is
	port(
		clk                : in  std_logic;
		pat_rst				   : out std_logic;
		mem_write			: out std_logic;
		data_mem_data_out	: in std_logic_vector(31 downto 0);
		mem_data_out_muxed : out std_logic_vector(31 downto 0);
		execute_dout		: in execution_out_type;
		led                : out std_logic;
		txd                : out std_logic;
		rxd                : in  std_logic
--		;-- dma controll interface
--		dma_addr_special_i : out std_logic;
--		dma_addr_i         : out std_logic_vector(3 downto 0);
--		dma_rd_i           : out std_logic;
--		dma_rd_data_i      : in  std_logic_vector(31 downto 0);
--		dma_wr_i           : out std_logic;
--		dma_wr_data_i      : out std_logic_vector(31 downto 0)

	--		oSRAM_A      : out   std_logic_vector(18 downto 0); -- edit
	--		SRAM_DQ      : inout std_logic_vector(31 downto 0); -- edit
	--		oSRAM_CE1_N  : out   std_logic;
	--		oSRAM_OE_N   : out   std_logic;
	--		oSRAM_BE_N   : out   std_logic_vector(3 downto 0);
	--		oSRAM_WE_N   : out   std_logic;
	--		oSRAM_GW_N   : out   std_logic;
	--		oSRAM_CLK    : out   std_logic;
	--		oSRAM_ADSC_N : out   std_logic;
	--		oSRAM_ADSP_N : out   std_logic;
	--		oSRAM_ADV_N  : out   std_logic;
	--		oSRAM_CE2    : out   std_logic;
	--		oSRAM_CE3_N  : out   std_logic
	);
end entity patmos_io;

architecture arch of patmos_io is

	signal memdin_reg : std_logic_vector(31 downto 0);


	signal mem_data_out_uart  : std_logic_vector(31 downto 0);

	
	signal int_res : std_logic;
	signal res_cnt : std_logic_vector(1 downto 0) := "00"; -- for the simulation
	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is
	"POWER_UP_LEVEL=LOW";
	signal rst : std_logic;             --out_rst

	-- I/O: Led
	signal led_reg : std_logic;
	signal counter, cntus : unsigned(31 downto 0);
	signal cnt_div : unsigned(7 downto 0);

	-- Edgar: should probably use stage name instead of _next/_reg. Or include it in appropriate stage register signals
	-- MS: I'm not sure if I like this IO type thing. IO devices are at addresses in my world
	signal io_next, io_reg : io_info_type;

	-- for the SSRAM interface - not used shall go into a top level
	--	signal clk2               : std_logic;
	--	signal sc_mem_out_wr_data : unsigned(31 downto 0);
	--	signal ram_addr    : std_logic_vector(18 downto 0); -- edit
	--	signal ram_dout    : std_logic_vector(31 downto 0); -- edit
	--	signal ram_din     : std_logic_vector(31 downto 0); -- edit
	--	signal ram_dout_en : std_logic;
	--	signal ram_clk     : std_logic;
	--	signal ram_nsc     : std_logic;
	--	signal ram_ncs     : std_logic;
	--	signal ram_noe     : std_logic;
	--	signal ram_nwe     : std_logic;
	--	signal sc_mem_out  : sc_out_type;
	--	signal sc_mem_in   : sc_in_type;
	------------------------------------------------------- uart signals
	signal uart_rd, uart_wr : std_logic;
	-- signal rdy_cnt   : unsigned(1 downto 0); 
	-- signal address          : std_logic_vector(31 downto 0) := (others => '0');

	component sc_mem_if
		generic(ram_ws    : integer;
			    addr_bits : integer);

		port(
			clk, reset  : in  std_logic;
			clk2        : in  std_logic; -- an inverted clock

			--
			--	SimpCon memory interface
			--
			sc_mem_out  : in  sc_out_type;
			sc_mem_in   : out sc_in_type;

			-- memory interface

			ram_addr    : out std_logic_vector(addr_bits - 1 downto 0);
			ram_dout    : out std_logic_vector(31 downto 0);
			ram_din     : in  std_logic_vector(31 downto 0);
			ram_dout_en : out std_logic;
			ram_clk     : out std_logic;
			ram_nsc     : out std_logic;
			ram_ncs     : out std_logic;
			ram_noe     : out std_logic;
			ram_nwe     : out std_logic
		);
	end component;

begin                                   -- architecture begin


	--
	--	internal reset generation
	--	should include the PLL lock signal
	--
	-- clk_int shall be generated from a PLL
	--
	process(clk)
	begin
		if rising_edge(clk) then
			if (res_cnt /= "11") then
				res_cnt <= std_logic_vector(unsigned(res_cnt) + 1);
			end if;
			int_res <= not res_cnt(0) or not res_cnt(1);
		end if;
	end process;

	-- MS: do we want an external reset?
	-- If yes, it has to be synchronized
	-- rst <= not out_rst;
	rst <= int_res;
	led <= led_reg;
	pat_rst <= int_res;
	
	io_decode : process(execute_dout, io_next)
		variable addr : std_logic_vector(31 downto 0);

	begin
		-- Everything disabled by default: device enabled in particular branch
		addr       := execute_dout.adrs;
		io_next    <= (address => addr, device => io_none, others => '0');
		io_next.rd <= execute_dout.lm_read_out_not_reg;
		io_next.wr <= execute_dout.lm_write_out_not_reg;

		
		-- Edgar: maybe can also use constants for different devices instead of one hot enables	
		if (addr(31 downto 28) = "1111") then -- UART, counters, LED
			case addr(11 downto 8) is
				when "0000" =>          -- UART
					io_next.uart_en <= '1';
					io_next.device  <= io_uart;
				when "0001" =>          -- counter
					io_next.counter_en <= '1';
					io_next.device     <= io_counter;
				when "0010" =>          -- LED
					io_next.led_en <= '1';
					io_next.device <= io_leds;
				when others =>
					io_next.sdram_en <= '1';
					io_next.device   <= io_sdram;
			end case;
		else
			io_next.mem_en             <= '1';
			io_next.device             <= io_memmory;
		end if;

	end process;


	-- Would also be clearer is address calculation has it's own signals.
	-- Maybe it shall be in it's own component (together with some address
	-- decoding).
	io_mem_read_mux : process(mem_data_out_uart, data_mem_data_out, execute_dout, io_reg, counter, cntus)
	begin
		mem_data_out_muxed <= (others  => '1'); -- The value for unused I/O address
		if io_reg.mem_en = '0' then
			case io_reg.device is
				when io_uart =>
					mem_data_out_muxed <= mem_data_out_uart;
				when io_counter =>
				    if io_reg.address(0) = '0' then
						mem_data_out_muxed <= std_logic_vector(counter);
					else
						mem_data_out_muxed <= std_logic_vector(cntus);
					end if;
				when io_sdram =>
--					mem_data_out_muxed <= dma_rd_data_i;
				when others =>
			end case;
		else
			mem_data_out_muxed <= data_mem_data_out;
		end if;
	end process;



	process(clk, rst)
	begin
		if rst = '1' then
			led_reg <= '0';
			counter <= (others => '0');
			cnt_div <= (others => '0');
			cntus <= (others => '0');
		elsif rising_edge(clk) then
			-- register the value decoded in ALU stage, to use it in mem stage
			io_reg     <= io_next;
			memdin_reg <= execute_dout.mem_write_data;       
			-- state for some I/O devices
			if io_reg.wr = '1' and io_reg.led_en = '1' then 
				led_reg <= memdin_reg(0);
			end if;
			counter <= counter + 1;
			-- Maybe counting down and testing against 0 consumes a little less resources. Do we care?
			if cnt_div=(CLK_FREQ/1000000-1) then
				cntus <= cntus + 1;
				cnt_div <= (others => '0');
			else
				cnt_div <= cnt_div + 1;
			end if;
		end if;
	end process;

	-- UART
	uart_rd <= io_reg.rd and io_reg.uart_en;
	uart_wr <= io_reg.wr and io_reg.uart_en;
	ua : entity work.uart generic map(
            clk_freq  => CLK_FREQ,
			baud_rate => UART_BAUD_RATE,
			txf_depth => UART_TXF_DEPTH,
			rxf_depth => UART_RXF_DEPTH
		)
		port map(
			clk     => clk,
			reset   => rst,
			address => io_reg.address(2),
			wr_data => memdin_reg,      --memdin,--execute_dout.mem_write_data_out,
			-- Edgar: Can we use VHDL 2008? 
			-- rd => io_reg.rd and io_reg.uart_en,
			-- wr => io_reg.wd and io_reg.uart_en,
			rd      => uart_rd,
			wr      => uart_wr,
			rd_data => mem_data_out_uart,
			txd     => txd,
			rxd     => rxd
		);
		
--	-- SDRAM: the I/O device uses word addressess, so we shift io_reg.address by 2 (x4)
--	dma_addr_special_i  <= io_reg.address(dma_addr_i'high+3);
--	dma_addr_i  <= io_reg.address(dma_addr_i'high+2 downto dma_addr_i'low+2);
--	dma_rd_i  <= io_reg.rd and io_reg.sdram_en;
--	dma_wr_i  <= io_reg.wr and io_reg.sdram_en;
--	dma_wr_data_i  <= memdin_reg;



	mem_write             <= io_next.wr and io_next.mem_en;

------------------------------------------------------ SRAM Interface
--	sc_mem_out.wr_data <= std_logic_vector(stack_cache_dout.dout_to_mem);
--	sc_mem_out.address <= std_logic_vector(stack_cache_ctrl_dout.st_out(22 downto 0));
--
--	scm : sc_mem_if
--		generic map(
--			ram_ws    => ram_cnt - 1,
--			addr_bits => 19             -- edit
--		)
--		port map(clk,
--			     int_res,
--			     clk2,
--			     sc_mem_out,
--			     sc_mem_in,
--			     ram_addr    => ram_addr,
--			     ram_dout    => ram_dout,
--			     ram_din     => ram_din,
--			     ram_dout_en => ram_dout_en,
--			     ram_clk     => ram_clk,
--			     ram_nsc     => ram_nsc,
--			     ram_ncs     => ram_ncs,
--			     ram_noe     => ram_noe,
--			     ram_nwe     => ram_nwe
--		);
--	oSRAM_A     <= ram_addr;
--	oSRAM_CE1_N <= ram_ncs;
--	oSRAM_OE_N  <= ram_noe;
--	oSRAM_WE_N  <= ram_nwe;
--	oSRAM_BE_N  <= (others => '0');
--	oSRAM_GW_N  <= '1';
--	oSRAM_CLK   <= ram_clk;
--
--	oSRAM_ADSC_N <= ram_ncs;
--	oSRAM_ADSP_N <= '1';
--	oSRAM_ADV_N  <= '1';
--
--	oSRAM_CE2   <= not ram_ncs;
--	oSRAM_CE3_N <= ram_ncs;

end architecture arch;





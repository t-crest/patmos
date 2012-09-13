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

entity patmos_core is
	port(
		clk                : in  std_logic;
		led                : out std_logic;
		txd                : out std_logic;
		rxd                : in  std_logic
--		;
--		-- dma controll interface
--		dma_addr_special_i : out std_logic;
--		dma_addr_i         : out std_logic_vector(4 downto 0);
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
end entity patmos_core;

architecture arch of patmos_core is
	signal write_enable : std_logic;
	signal test         : std_logic;

	signal memdin : std_logic_vector(31 downto 0);
	-- Edgar: Trying to reverse engineer the memdin/mem_write_data_in/mem_write_data_out:
	--	  memdin - seames to be alu_src2 (after forwarding)	[in patmos_alu()]
	--	  alu_din.mem_write_data_in <= memdin 
	--	  mem_din.mem_write_data_in <= memdin 
	--	  execute_dout.mem_write_data_out <=clk'<= alu_din.mem_write_data_in; [in patmos_alu()]
	--	  mem_dout.mem_write_data_out <=clk'<= mem_din.mem_write_data_in; [in patmos_mem_stage()]
	-- Seams that those are just copies of the same signals (registered and non registered memdin)
	-- Instead of beeing delayed version
	-- Don't know the pipeline though, so don't one to interfere with work in progress.
	-- Adding new signal which is easy to spot and rename. 

	signal memdin_reg : std_logic_vector(31 downto 0);

	signal data_mem_data_out      : std_logic_vector(31 downto 0);
	signal fetch_dout             : fetch_out_type;
	signal fetch_reg1, fetch_reg2 : std_logic_vector(4 downto 0);
	signal decode_din             : decode_in_type;
	signal decode_dout            : decode_out_type;
	signal alu_din                : alu_in_type;
	signal execute_dout           : execution_out_type;
	signal write_back             : write_back_in_out_type;
	signal stack_cache_din        : patmos_stack_cache_in;
	signal stack_cache_dout       : patmos_stack_cache_out;
	signal stack_cache_ctrl_din   : patmos_stack_cache_ctrl_in;
	signal stack_cache_ctrl_dout  : patmos_stack_cache_ctrl_out;
	signal mem_din                : mem_in_type;
	signal mem_dout               : mem_out_type;
	signal mux_mem_reg            : std_logic_vector(31 downto 0);
	signal fw_ctrl_rs1            : forwarding_type;
	signal fw_ctrl_rs2            : forwarding_type;
	signal mem_data_out           : std_logic_vector(31 downto 0);

	signal out_rxd            : std_logic := '0';
	signal mem_data_out_uart  : std_logic_vector(31 downto 0);
	signal mem_data_out_muxed : std_logic_vector(31 downto 0);
	signal mem_data_out3      : std_logic_vector(31 downto 0);

	signal mem_data_out_sdram : std_logic_vector(31 downto 0);

	signal spill, fill          : std_logic;
	signal instruction_mem_din  : instruction_memory_in_type;
	signal instruction_mem_dout : instruction_memory_out_type;
	signal instruction_rom_out  : std_logic_vector(31 downto 0);

	signal clk_int : std_logic;
	-- MS: maybe some signal sorting would be nice
	-- for generation of internal reset
	signal int_res : std_logic;
	signal res_cnt : std_logic_vector(1 downto 0) := "00"; -- for the simulation
	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is
	"POWER_UP_LEVEL=LOW";
	signal rst : std_logic;             --out_rst

	-- I/O: Led
	signal led_reg : std_logic;
	signal counter : unsigned(31 downto 0);

	-- Edgar: should probably use stage name instead of _next/_reg. Or include it in appropriate stage register signals
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
	------------------------------------------------------- fetch	

	fet : entity work.patmos_fetch
		port map(clk, rst, decode_dout, execute_dout, fetch_reg1, fetch_reg2, fetch_dout);

	-- MS: this shall go into the fetch stage

	--   instruction_mem_address: process(execute_dout.alu_result_out, pc, instruction_rom_out, instruction_mem_dout.inst_out) --read/write enable here
	--  begin
	--  	if(pc <= 70 ) then --  change this after the final version of boot loader
	--  		if (execute_dout.mem_write_out = '1') then
	--  			instruction_mem_din.address <= execute_dout.alu_result_out - 512;
	--  		end if;
	--  	end if;
	--  	if (pc >= 70) then
	--  		--instruction_mem_din.address <= pc - 70 + 7;
	--  		
	--  		--instruction_mem_din.read_enable <= '1';
	--  	end if;
	--  end process;


	--  instruction_mem : entity work.patmos_instruction_memory(arch)
	--  port map(clk, rst, instruction_mem_din.address, 
	--           execute_dout.mem_write_data_out,
	--           instruction_mem_dout.inst_out, 
	--	        instruction_mem_din.read_enable, instruction_mem_din.write_enable);
	-------------------------------------------------------- decode

	reg_file : entity work.patmos_register_file(arch)
		port map(clk,
			     rst,
			     fetch_reg1,
			     fetch_reg2,
			     std_logic_vector(execute_dout.write_back_reg_out),
			     --			     fetch_dout.instruction(16 downto 12),
			     --			     fetch_dout.instruction(11 downto 7),
			     decode_din.rs1_data_in,
			     decode_din.rs2_data_in,
			     mux_mem_reg,
			     execute_dout.reg_write_out);

	decode_din.operation <= fetch_dout.instruction;
	decode_din.instr_b   <= fetch_dout.instr_b;
	dec : entity work.patmos_decode(arch)
		port map(clk, rst, decode_din, decode_dout);

	---------------------------------------------------- execute
	wb : process(clk)
	begin
		if rising_edge(clk) then
			write_back.write_value  <= mem_dout.data_out;
			write_back.write_reg    <= mem_dout.write_back_reg_out;
			write_back.write_enable <= mem_dout.reg_write_out;
		end if;
	end process wb;

	alu_din.inst_type            <= decode_dout.inst_type_out;
	alu_din.ALU_instruction_type <= decode_dout.ALU_instruction_type_out;
	alu_din.ALU_function_type    <= decode_dout.ALU_function_type_out;
	alu_din.STT_instruction_type <= decode_dout.STT_instruction_type_out;
	alu_din.LDT_instruction_type <= decode_dout.LDT_instruction_type_out;

	alu_din.mem_write_data_in <= memdin;
	---------------------------------------alu
	alu : entity work.patmos_alu(arch)
		port map(clk, rst, decode_dout, alu_din, execute_dout, mem_dout, memdin);

	-----------------------------------------------cache - memory------------------------------------------------------------
	---------------------------------------------------- stack cache controller


	stack_cache_ctrl_din.stc_immediate_in <= decode_dout.ALUi_immediate_out(4 downto 0);
	stack_cache_ctrl_din.instruction      <= decode_dout.STC_instruction_type_out;
	--	stack_cache_ctrl_din.st_in 
	stack_cache_ctrl : entity work.patmos_stack_cache_ctrl(arch)
		port map(clk, rst, stack_cache_ctrl_din, stack_cache_ctrl_dout);

	---------------------------------------------------- stack cache

	stack_cache_in : process(execute_dout) -- which type of transfer to stack cache?
	begin
		-- some defaults
		stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out;
		if (execute_dout.STT_instruction_type_out = SWS) then
			stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out;
		elsif (execute_dout.STT_instruction_type_out = SBS) then
			stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) &
				execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) &
				execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15) & execute_dout.mem_write_data_out(15 downto 0); -- <= (16 downto 0) <= execute_dout.mem_write_data_out(16 downto 0);
		elsif (execute_dout.STT_instruction_type_out = SHS) then
			stack_cache_din.din_from_cpu(8 downto 0) <= execute_dout.mem_write_data_out(8 downto 0);
		end if;
	end process;

	--	stack_cache_out : process(execute_dout, stack_cache_dout.dout_to_cpu) -- which type of transfer from stack cache?
	--	begin
	--		mem_data_out <= stack_cache_dout.dout_to_cpu;
	--		if (execute_dout.LDT_instruction_type_out = LWS) then
	--			mem_data_out <= stack_cache_dout.dout_to_cpu;
	--		elsif (execute_dout.LDT_instruction_type_out = LBS) then
	--			mem_data_out(16 downto 0) <= stack_cache_dout.dout_to_cpu(16 downto 0);
	--		elsif (execute_dout.LDT_instruction_type_out = LHS) then
	--			mem_data_out(8 downto 0) <= stack_cache_dout.dout_to_cpu(8 downto 0);
	--		end if;
	--	end process;

	--	stack_cache_din.din_from_cpu <= execute_dout.mem_write_data_out; -- transfer to stack cache no matter what, should change based on controlling signals

	--	mem_data_out <= stack_cache_dout.dout_to_cpu;
	stack_cache_din.spill_fill <= stack_cache_ctrl_dout.spill_fill;

	-- Edgar: is the double register on address intended? (the input is registered in block ram allready)
	stack_cache_din.address   <= execute_dout.alu_result_out(4 downto 0);
	stack_cache_din.head_tail <= stack_cache_ctrl_dout.head_tail;

	stack_cache : entity work.patmos_stack_cache(arch)
		port map(clk, rst, stack_cache_din, stack_cache_dout);

	------------------------------------------------------- memory
	-- mem/io decoder

	-- MS: IO shall go into it's own 'top level' component
	-- We need to find a reasonable address mapping, not starting IO at
	-- address 0
	io_decode : process(execute_dout, decode_dout, io_next)
		variable addr : std_logic_vector(31 downto 0);

	begin
		-- Everything disabled by default: device enabled in particular branch
		addr       := execute_dout.alu_result;
		io_next    <= (address => addr, device => io_none, others => '0');
		io_next.rd <= decode_dout.lm_read_out;
		io_next.wr <= decode_dout.lm_write_out;

		-- MS: This decoding will also trigger the IO devices as it goes form
		-- different address bits.
		-- MS: decoding from two different pipeline stages (alu_result for
		-- the SPM and alu_result_out for others) does NOT work!!!!!

		-- we need a clear solution for this in the right pipeline stage
		--		if (execute_dout.alu_result(8) = '1') then --data mem

		-- Edgar: maybe can also use constants for different devices instead of one hot enables	
		if (addr(31 downto 28) = "1111") then -- UART, counters, LED
			case addr(11 downto 8) is
				when "0000" =>          -- UART
					io_next.uart_en <= '1';
					io_next.device  <= io_uart;
				when "0001" =>          -- sdram I/O device
					io_next.sdram_en <= '1';
					io_next.device   <= io_sdram;
				when "0010" =>          -- LED
					io_next.led_en <= '1';
					io_next.device <= io_leds;
				-- Edgar: maybe counter should also get an fixed address. Leaving the original behaviour for now.	
				when others =>
					io_next.counter_en <= '1';
					io_next.device     <= io_counter;
			end case;
		else                            --if (execute_dout.alu_result(8) = '1') then --data mem
			io_next.mem_en             <= '1';
			io_next.device             <= io_memmory;
			-- Edgar: didn't want to remove those, maybe somebody is using them.
			test                       <= '1';
			io_next.instruction_mem_wr <= '0'; -- instruction_mem_din.write_enable <= '0';
		end if;

	end process;

	-- Copy decoded value to pipeline records. (maybe can use only one version later)
	instruction_mem_din.write_enable <= io_next.instruction_mem_wr;
	stack_cache_din.write_enable     <= io_next.stack_cache_wr;

	--	io_mem_read_mux : process(mem_data_out_uart, data_mem_data_out, execute_dout)
	--	begin
	--		if (execute_dout.alu_result(8) = '0') then
	--			mem_data_out_muxed <= mem_data_out_uart;
	--		else
	--			mem_data_out_muxed <= data_mem_data_out;
	--		end if;
	--	end process;

	-- MS: what is the difference between alu_result and alu_result_out?
	-- Maybe _out is the registered value. I don't like that read decode and
	-- read mux selection is done from two different signals.
	-- Would also be clearer is address calculation has it's own signals.
	-- Maybe it shall be in it's own component (together with some address
	-- decoding).
	io_mem_read_mux : process(mem_data_out_uart, data_mem_data_out, execute_dout, io_reg)
	begin
		if io_reg.mem_en = '0' then
			case io_reg.device is
				when io_uart =>
					mem_data_out_muxed <= mem_data_out_uart;
				when io_sdram =>
--					mem_data_out_muxed <= dma_rd_data_i;
				when others =>
					mem_data_out_muxed <= std_logic_vector(counter);
			end case;
		else
			mem_data_out_muxed <= data_mem_data_out;
		end if;
	end process;

	write_back_proc : process(execute_dout, mem_data_out_muxed)
	begin
		if execute_dout.mem_to_reg_out = '1' then
			mux_mem_reg <= mem_data_out_muxed;
		else
			mux_mem_reg <= execute_dout.alu_result_out;
		end if;
	end process;

	process(clk, rst)
	begin
		if rst = '1' then
			led_reg <= '0';
			counter <= (others => '0');
		elsif rising_edge(clk) then
			-- register the value decoded in ALU stage, to use it in mem stage
			io_reg     <= io_next;
			memdin_reg <= memdin;       -- Edgar: there are multiple copies of registered memdin signal (see comments in the declaration), but maybe some of them are work in progress and will change, so don't want to use the wrong one
			-- state for some I/O devices
			if io_reg.wr = '1' and io_reg.led_en = '1' then -- Edgar: was using unregistered value, so suppose it was a bug
				led_reg <= memdin(0);
			end if;
			counter <= counter + 1;
		end if;
	end process;

	-- UART
	uart_rd <= io_reg.rd and io_reg.uart_en;
	uart_wr <= io_reg.wr and io_reg.uart_en;
	ua : entity work.uart generic map(
			clk_freq  => 50 * 1000 * 1000, -- altera DE2-70
			baud_rate => 115200,
			-- clk_freq  => 200 * 1000 * 1000, -- xilinx ML605
			--			baud_rate => 9600,	-- with 50MHz clk: 9600@50MHz on altera and 38400@200MHz on xilinx

			txf_depth => 1,
			rxf_depth => 1
		)
		port map(
			clk     => clk,
			reset   => rst,
			address => io_reg.address(2), -- Edgar: Why (2) not (0)?
			wr_data => memdin_reg,      --memdin,--execute_dout.mem_write_data_out,
			-- Edgar: Can we use VHDL 2008? 
			-- rd => io_reg.rd and io_reg.uart_en,
			-- wr => io_reg.wd and io_reg.uart_en,
			rd      => uart_rd,
			wr      => uart_wr,
			rd_data => mem_data_out_uart,
			txd     => txd,
			rxd     => out_rxd
		);
--	-- SDRAM: the I/O device uses word addressess, so we shift io_reg.address by 2 (x4)
--	dma_addr_special_i  <= io_reg.address(dma_addr_i'high+3);
--	dma_addr_i  <= io_reg.address(dma_addr_i'high+2 downto dma_addr_i'low+2);
--	dma_rd_i  <= io_reg.rd and io_reg.sdram_en;
--	dma_wr_i  <= io_reg.wr and io_reg.sdram_en;
--	dma_wr_data_i  <= memdin_reg;


	-- MS: what is this?
	--	out_rxd <= not out_rxd after 100 ns;

	-- TODO: the memory code belongs into the memory stage component

	mem_din.STT_instruction_type_out <= decode_dout.STT_instruction_type_out;
	mem_din.LDT_instruction_type_out <= decode_dout.LDT_instruction_type_out;
	mem_din.alu_result               <= execute_dout.alu_result;
	mem_din.mem_write                <= io_next.wr and io_next.mem_en;
	--	mem_din.alu_src2   <= memdin;
	data_mem_data_out                <= mem_dout.data_mem_data_out;
	--------------------------
	mem_din.data_in                  <= mux_mem_reg;
	-- forward
	mem_din.reg_write_in             <= execute_dout.reg_write_out or execute_dout.mem_to_reg_out; --execute_dout.mem_to_reg_out or execute_dout.mem_write_out;
	mem_din.write_back_reg_in        <= execute_dout.write_back_reg_out;
	mem_din.mem_write_data_in        <= memdin; --execute_dout.mem_write_data_out;
	memory_stage : entity work.patmos_mem_stage(arch)
		port map(clk, rst, mem_din, mem_dout);

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






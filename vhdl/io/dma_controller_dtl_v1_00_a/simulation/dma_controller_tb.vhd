library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dma_controller_tb is
end entity dma_controller_tb;

use work.dma_controller_dtl_cmp_pkg.all;

architecture RTL of dma_controller_tb is
	constant CLOCK_PERIOD                      : time    := 10 ns;
	constant MEM_SIZE                          : integer := 2 ** 8;
	constant DEBUG_SHOW_MEMORY_TRANSACTIONS    : boolean := true;
	constant DEBUG_SHOW_CACHELINE_TRANSACTIONS : boolean := true;

	constant DQ_WIDTH         : integer := 8;
	constant MTL_MASK_WIDTH   : integer := 4;
	constant MTL_SIZE_WIDTH   : integer := 5;
	constant MTL_ADDR_WIDTH   : integer := 32;
	-- Request size in bytes
	constant GEN_REQUEST_SIZE : integer := 64;
	-- DMA control interface
	constant DMA_ADDR_WIDTH   : integer := 5;
	constant DMA_DATA_WIDTH   : integer := 32;

	constant DATA_WIDTH_BITS : integer := 4 * DQ_WIDTH;
	constant NWORDS_PER_CMD  : integer := GEN_REQUEST_SIZE * 8 / DATA_WIDTH_BITS;

	subtype dma_word_t is std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);

	signal mtl_clk              : std_logic;
	signal mtl_rst_n            : std_logic;
	signal mtl_wr_valid_i       : std_logic;
	signal mtl_cmd_valid_i      : std_logic;
	signal mtl_cmd_accept_i     : std_logic;
	signal mtl_cmd_addr_i       : std_logic_vector(MTL_ADDR_WIDTH - 1 downto 0);
	signal mtl_cmd_read_i       : std_logic;
	signal mtl_cmd_block_size_i : std_logic_vector(MTL_SIZE_WIDTH - 1 downto 0);
	signal mtl_wr_last_i        : std_logic;
	signal mtl_flush_i          : std_logic;
	signal mtl_wr_accept_i      : std_logic;
	signal mtl_wr_data_i        : std_logic_vector(4 * DQ_WIDTH - 1 downto 0);
	signal mtl_wr_mask_i        : std_logic_vector(MTL_MASK_WIDTH - 1 downto 0);

	signal mtl_rd_last_i      : std_logic;
	signal mtl_rd_valid_i     : std_logic;
	signal mtl_rd_accept_i    : std_logic;
	signal mtl_rd_data_i      : std_logic_vector(4 * DQ_WIDTH - 1 downto 0);
	signal dma_addr_special_i : std_logic;
	signal dma_addr_i         : std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);
	signal dma_rd_i           : std_logic;
	signal dma_rd_data_i      : std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);
	signal dma_wr_i           : std_logic;
	signal dma_wr_data_i      : std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);

	type mem_t is array (0 to MEM_SIZE - 1) of dma_word_t;
	signal mem : mem_t := (others => (others => '0'));
	signal end_of_sim : std_logic := '0';
begin
	dut : work.dma_controller_dtl_cmp_pkg.dma_controller_dtl
		generic map(DQ_WIDTH         => DQ_WIDTH,
			        MTL_MASK_WIDTH   => MTL_MASK_WIDTH,
			        MTL_SIZE_WIDTH   => MTL_SIZE_WIDTH,
			        MTL_ADDR_WIDTH   => MTL_ADDR_WIDTH,
			        GEN_REQUEST_SIZE => GEN_REQUEST_SIZE,
			        DMA_ADDR_WIDTH   => DMA_ADDR_WIDTH,
			        DMA_DATA_WIDTH   => DMA_DATA_WIDTH)
		port map(mtl_clk              => mtl_clk,
			     mtl_rst_n            => mtl_rst_n,
			     mtl_cmd_valid_i      => mtl_cmd_valid_i,
			     mtl_cmd_accept_i     => mtl_cmd_accept_i,
			     mtl_cmd_addr_i       => mtl_cmd_addr_i,
			     mtl_cmd_read_i       => mtl_cmd_read_i,
			     mtl_cmd_block_size_i => mtl_cmd_block_size_i,
			     mtl_wr_last_i        => mtl_wr_last_i,
			     mtl_wr_valid_i       => mtl_wr_valid_i,
			     mtl_flush_i          => mtl_flush_i,
			     mtl_wr_accept_i      => mtl_wr_accept_i,
			     mtl_wr_data_i        => mtl_wr_data_i,
			     mtl_wr_mask_i        => mtl_wr_mask_i,
			     mtl_rd_last_i        => mtl_rd_last_i,
			     mtl_rd_valid_i       => mtl_rd_valid_i,
			     mtl_rd_accept_i      => mtl_rd_accept_i,
			     mtl_rd_data_i        => mtl_rd_data_i,
			     dma_addr_special_i   => dma_addr_special_i,
			     dma_addr_i           => dma_addr_i,
			     dma_rd_i             => dma_rd_i,
			     dma_rd_data_i        => dma_rd_data_i,
			     dma_wr_i             => dma_wr_i,
			     dma_wr_data_i        => dma_wr_data_i);

	clock_driver : process
	begin
		mtl_clk <= '0';
		wait for CLOCK_PERIOD / 2;
		mtl_clk <= '1';
		wait for CLOCK_PERIOD / 2;
	end process clock_driver;
	mtl_rst_n <= '0', '1' after CLOCK_PERIOD * 2;

	mtl_cmd_accept_i <= '1';
	memory_emulation : process is
		variable addr  : natural;
		variable count : integer;

	begin
		mtl_rd_last_i   <= '0';
		mtl_rd_valid_i  <= '0';
		mtl_wr_accept_i <= '0';

		wait until rising_edge(mtl_clk);
		if mtl_cmd_valid_i = '1' then   -- NOTE: cmd_accept always high
			addr  := to_integer(unsigned(mtl_cmd_addr_i)) / 4; -- the mtl_cmd_addr_i is byte addressable
			count := to_integer(unsigned(mtl_cmd_block_size_i));
			if mtl_cmd_read_i = '1' then
				loop
					mtl_rd_valid_i <= '1';
					mtl_rd_data_i  <= mem(addr);
					if (count = 0) then
						mtl_rd_last_i <= '1';
					end if;
					wait until rising_edge(mtl_clk);
					if mtl_rd_accept_i = '1' then
						addr := addr + 1;
						exit when count = 0;
						count := count - 1;
					end if;
				end loop;
			else
				mtl_wr_accept_i <= '1';
				for i in 0 to count loop
					loop
						exit when mtl_wr_valid_i = '1';
						wait until rising_edge(mtl_clk);
					end loop;
					mem(addr) <= mtl_wr_data_i;
					addr      := addr + 1;
					assert (mtl_wr_last_i = '1' or i /= count) report "mtl_wd_last_i signal error: i=" & integer'image(i) & " (of " & integer'image(count) & ") last=" & std_logic'image(mtl_wr_last_i);
					wait until rising_edge(mtl_clk);
				end loop;
			end if;
		end if;
	end process memory_emulation;

	show_cache_line_transactions : if DEBUG_SHOW_CACHELINE_TRANSACTIONS generate
		cl_monitor : process
		begin
			wait until rising_edge(mtl_clk);
			if dma_rd_i = '1' and dma_addr_special_i = '0' then
				report "CL RD " & integer'image(to_integer(unsigned(dma_rd_data_i))) & " at " & integer'image(to_integer(unsigned(dma_addr_i)));
			end if;
			if dma_wr_i = '1' and dma_addr_special_i = '0' then
				report "CL WR " & integer'image(to_integer(unsigned(dma_wr_data_i))) & " at " & integer'image(to_integer(unsigned(dma_addr_i)));
			end if;
		end process cl_monitor;
	end generate show_cache_line_transactions;

	show_memory_transactions : if DEBUG_SHOW_MEMORY_TRANSACTIONS generate
		memory_monitor : process
			variable addr : integer;
		begin
			wait until rising_edge(mtl_clk);
			if mtl_cmd_accept_i = '1' and mtl_cmd_valid_i = '1' then
				addr := TO_INTEGER(unsigned(mtl_cmd_addr_i)) / 4; -- the mtl_cmd_addr_i is byte addressable
			end if;

			if mtl_rd_accept_i = '1' and mtl_rd_valid_i = '1' then
				report "MEM RD " & integer'image(to_integer(unsigned(mtl_rd_data_i))) & " at " & integer'image(addr) & " last=" & std_logic'image(mtl_rd_last_i);
				addr := addr + 1;
			end if;
			if mtl_wr_accept_i = '1' and mtl_wr_valid_i = '1' then
				report "MEM WR " & integer'image(to_integer(unsigned(mtl_wr_data_i))) & " at " & integer'image(addr) & " last=" & std_logic'image(mtl_wr_last_i);
				addr := addr + 1;
			end if;
		end process memory_monitor;
	end generate show_memory_transactions;

	control_test : process is
		type line_op_t is (loLoadLine, loStoreLine);

		procedure controllerRead(isSpecial : std_logic; addr : natural; result : out dma_word_t) is
		begin
			dma_addr_special_i <= isSpecial;
			dma_addr_i         <= std_logic_vector(to_unsigned(addr, dma_addr_i'length));
			dma_rd_i           <= '1';
			dma_wr_i           <= '0';
			wait until rising_edge(mtl_clk);

			result := dma_rd_data_i;
		end procedure controllerRead;
		procedure controllerRead(isSpecial : std_logic; addr : natural; result : out natural) is
			variable result_slv : dma_word_t;
		begin
			controllerRead(isSpecial, addr, result_slv);
			result := to_integer(unsigned(result_slv));
		end procedure controllerRead;

		procedure controllerWrite(isSpecial : std_logic; addr : natural; value : dma_word_t) is
		begin
			dma_addr_special_i <= isSpecial;
			dma_addr_i         <= std_logic_vector(to_unsigned(addr, dma_addr_i'length));
			dma_rd_i           <= '0';
			dma_wr_i           <= '1';
			dma_wr_data_i      <= value;
			wait until rising_edge(mtl_clk);
		end procedure controllerWrite;
		procedure controllerWrite(isSpecial : std_logic; addr : natural; value : natural) is
		begin
			controllerWrite(isSpecial, addr, std_logic_vector(to_unsigned(value, dma_wr_data_i'length)));
		end procedure controllerWrite;

		procedure waitReady is
			variable stat : natural;
		begin
			loop
				controllerRead('1', DMA_OFFSET_CMD_STAT, stat);
				exit when stat = DMA_STATUS_READY;
			end loop;
		end procedure waitReady;

		procedure memoryLineOp(lineAddr : natural; lineOp : line_op_t) is
		begin
			waitReady;
			controllerWrite('1', DMA_OFFSET_ADDR_REG, lineAddr * NWORDS_PER_CMD * 4);

			if lineOp = loLoadLine then
				controllerWrite('1', DMA_OFFSET_CMD_STAT, DMA_CMD_LOAD_LINE);
			else
				controllerWrite('1', DMA_OFFSET_CMD_STAT, DMA_CMD_STORE_LINE);
			end if;
			waitReady;
		end procedure memoryLineOp;

		variable isReady : boolean;
		variable value   : natural;

	begin
		wait until mtl_rst_n = '1';
		waitReady;

		controllerWrite('0', 0, 1);
		controllerWrite('0', 5, 6);
		controllerWrite('0', 15, 16);
		controllerRead('0', 0, value);
		assert value = 1 report "T1 cache word write/read error at 0";
		controllerRead('0', 5, value);
		assert value = 6 report "T1 cache word write/read error at 5";
		controllerRead('0', 15, value);
		assert value = 16 report "T1 cache word write/read error at 5";

		memoryLineOp(0, loStoreLine);
		memoryLineOp(1, loLoadLine);
		controllerRead('0', 0, value);
		assert value = 0 report "T2 cache word write/read error at 0";
		controllerRead('0', 5, value);
		assert value = 0 report "T2 cache word write/read error at 5";
		controllerRead('0', 15, value);
		assert value = 0 report "T2 cache word write/read error at 5";

		memoryLineOp(0, loLoadLine);
		controllerRead('0', 0, value);
		assert value = 1 report "T3 cache word write/read error at 0";
		controllerRead('0', 5, value);
		assert value = 6 report "T3 cache word write/read error at 5";
		controllerRead('0', 15, value);
		assert value = 16 report "T3 cache word write/read error at 5";

		end_of_sim <= '1';
		-- report "OK.  Test Finished!" severity failure;
	end process control_test;

end architecture RTL;

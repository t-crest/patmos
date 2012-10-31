library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

--library rdt_util_misclib;
--use rdt_util_misclib.rdt_util_misc_pkg.all;

use work.dma_controller_dtl_cmp_pkg.all;

architecture arch of dma_controller_dtl is
	constant DATA_WIDTH_BITS : integer := 4 * DQ_WIDTH;
	constant NWORDS_PER_CMD  : integer := GEN_REQUEST_SIZE * 8 / DATA_WIDTH_BITS;
	type state_t is (ready, read_cmd, read_data, write_cmd, write_data);
	signal state_r, state_nxt : state_t;

	type mem_line_t is array (0 to DMA_ADDR_WIDTH ** 2 - 1) of std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);
	signal mem_line : mem_line_t := (others => (others  => '0'));

	signal words_transferred_r, words_transferred_nxt : std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);

	signal addr_r, addr_nxt       : std_logic_vector(MTL_ADDR_WIDTH - 1 downto 0);
	signal cl_addr                : std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);
	signal cl_wr                  : std_logic;
	signal cl_rd_data, cl_wr_data : std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);
	signal dma_cmd_received       : std_logic;

	signal mtl_rd_accept_ii : std_logic;
	signal dma_rd_data_ii   : std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);

begin
	assert NWORDS_PER_CMD <= 2**MTL_SIZE_WIDTH report "log2(NWORDS_PER_CMD) > MTL_SIZE_WIDTH. The number of required words per commands exceeds the block-size output capacity." severity failure;
	assert DMA_DATA_WIDTH = DATA_WIDTH_BITS report "DMA_DATA_WIDTH /= DATA_WIDTH_BITS. Current code assumes same word size on CPU and memory interfaces" severity failure;

	mtl_rd_accept_i <= mtl_rd_accept_ii;
	dma_rd_data_i   <= dma_rd_data_ii;

	-- registers
	state : process(mtl_clk, mtl_rst_n)
	begin
		if RISING_EDGE(mtl_clk) then
			if (mtl_rst_n = '0') then
				state_r             <= ready;
				addr_r              <= (others => '0');
				words_transferred_r <= (others => '0');
			else
				addr_r              <= addr_nxt;
				state_r             <= state_nxt;
				words_transferred_r <= words_transferred_nxt;
			end if;
		end if;
	end process;

	-- cache line
	process(mtl_clk)
	begin
		if RISING_EDGE(mtl_clk) then
			if cl_wr = '1' then
				mem_line(to_integer(unsigned(cl_addr))) <= cl_wr_data;
			--	 TODO: Needs assymetric memory if DMA_DATA_WIDTH /= DATA_WIDTH_BITS:	
			--	for byte in 0 to 3 loop
			--		mem_line(to_integer(unsigned(words_transferred_r) & to_unsigned(i, 2))) <= mtl_rd_data_i((i+1) * DQ_WIDTH - 1 downto i * DQ_WIDTH);
			--	end loop;
			end if;
		end if;
	end process;
	cl_rd_data <= mem_line(to_integer(unsigned(cl_addr)));

	-- cache line port sharing: accessed by controll interface in ready state and by memory interface in other states
	cl_port_sharing : process(state_r, dma_addr_i, dma_wr_i, dma_wr_data_i, words_transferred_r, mtl_rd_accept_ii, mtl_rd_valid_i, mtl_rd_data_i) is
	begin
		if state_r = ready then
			cl_addr    <= dma_addr_i;
			cl_wr      <= dma_wr_i and not dma_addr_special_i;
			cl_wr_data <= dma_wr_data_i;
		else
			cl_addr    <= words_transferred_r;
			cl_wr      <= mtl_rd_accept_ii and mtl_rd_valid_i;
			cl_wr_data <= mtl_rd_data_i;
		end if;
	end process;

	-- memory map: controll interface memory map
	memory_map : process(cl_rd_data, dma_addr_special_i, state_r, addr_r, dma_wr_i, dma_wr_data_i) is
	begin
		-- Input ports: the address is ignorred since we have only single input_port 
		dma_rd_data_ii <= cl_rd_data;   -- defaults
		if dma_addr_special_i = '1' then
			if state_r = ready then
				dma_rd_data_ii <= const2slv(DMA_STATUS_READY, dma_rd_data_ii);
			else
				dma_rd_data_ii <= const2slv(DMA_STATUS_BUSY, dma_rd_data_ii);
			end if;
		end if;

		-- Output ports: write to cache line is specified in cl_port_sharing 
		addr_nxt         <= addr_r;     -- defaults
		dma_cmd_received <= '0';
		if dma_wr_i = '1' and state_r = ready and dma_addr_special_i = '1' then
			if dma_addr_i = const2slv(DMA_OFFSET_ADDR_REG, dma_addr_i) then
				addr_nxt <= dma_wr_data_i(addr_nxt'range);
			elsif dma_addr_i = const2slv(DMA_OFFSET_CMD_STAT, dma_addr_i) then
				dma_cmd_received <= '1';
			end if;
		end if;
	end process;

	mtl_flush_i          <= '0';
	mtl_wr_mask_i        <= (others => '1');
	mtl_cmd_block_size_i <= std_logic_vector(to_unsigned(NWORDS_PER_CMD - 1, MTL_SIZE_WIDTH));
	mtl_cmd_addr_i       <= addr_r;
	mtl_wr_data_i        <= cl_rd_data;

	cmd : process (state_r, words_transferred_r, dma_cmd_received, dma_wr_data_i, mtl_cmd_accept_i, mtl_rd_valid_i, mtl_rd_last_i, mtl_wr_accept_i) is
	begin
		-- default output
		state_nxt             <= state_r;
		words_transferred_nxt <= words_transferred_r;
		mtl_cmd_valid_i       <= '0';
		mtl_cmd_read_i        <= '1';
		mtl_wr_last_i         <= '0';
		mtl_wr_valid_i        <= '0';
		mtl_rd_accept_ii      <= '0';

		case state_r is
			-- Ready to load/store line or perform word read/write
			when ready =>
				if dma_cmd_received = '1' then
					if dma_wr_data_i = const2slv(DMA_CMD_LOAD_LINE, dma_wr_data_i) then
						state_nxt <= read_cmd;
					else
						state_nxt <= write_cmd;
					end if;
				end if;
				-- reset the counter used during transfer
				words_transferred_nxt <= (others => '0');
			-- Request the line read from memory
			when read_cmd =>
				mtl_cmd_valid_i <= '1';
				mtl_cmd_read_i  <= '1';
				if mtl_cmd_accept_i = '1' then
					state_nxt <= read_data;
				end if;
			-- Save the received data in cache line
			when read_data =>
				mtl_rd_accept_ii <= '1';
				-- cl write logic is in cl_port_sharing, so we just update counter
				if mtl_rd_valid_i = '1' then
					words_transferred_nxt <= words_transferred_r + 1;
					if mtl_rd_last_i = '1' then
						state_nxt <= ready;
					end if;
				end if;
			-- Request the line write to memory
			when write_cmd =>
				mtl_cmd_valid_i <= '1';
				mtl_cmd_read_i  <= '0';
				if mtl_cmd_accept_i = '1' then
					state_nxt <= write_data;
				end if;
			-- Send the cache line data
			when write_data =>
				mtl_wr_valid_i <= '1';
				if mtl_wr_accept_i = '1' then
					words_transferred_nxt <= words_transferred_r + 1;
				end if;
				if words_transferred_r = (NWORDS_PER_CMD - 1) then
					mtl_wr_last_i <= '1';
					if mtl_wr_accept_i = '1' then
						state_nxt <= ready;
					end if;
				end if;
		end case;
	end process cmd;

end architecture arch;

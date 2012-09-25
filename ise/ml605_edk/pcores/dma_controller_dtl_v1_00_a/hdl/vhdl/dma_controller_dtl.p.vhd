library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package dma_controller_dtl_cmp_pkg is
	component dma_controller_dtl is
		generic(
			DQ_WIDTH            : integer := 8;
			MTL_MASK_WIDTH      : integer := 4;
			MTL_SIZE_WIDTH      : integer := 5; -- TODO: make sure this automatically wraps to 0 when the message is complete!
			MTL_ADDR_WIDTH      : integer := 32;
			-- Request size in bytes
			GEN_REQUEST_SIZE    : integer := 64;
			-- DMA control interface
			DMA_ADDR_WIDTH      : integer := 4;
			DMA_DATA_WIDTH      : integer := 32
		);
		port(
			mtl_clk              : in  std_logic;
			mtl_rst_n            : in  std_logic;

			mtl_cmd_valid_i      : out std_logic;
			mtl_cmd_accept_i     : in  std_logic;
			mtl_cmd_addr_i       : out std_logic_vector(MTL_ADDR_WIDTH - 1 downto 0);
			mtl_cmd_read_i       : out std_logic;
			mtl_cmd_block_size_i : out std_logic_vector(MTL_SIZE_WIDTH - 1 downto 0);
			mtl_wr_last_i        : out std_logic;
			mtl_wr_valid_i       : out std_logic;
			mtl_flush_i          : out std_logic;
			mtl_wr_accept_i      : in  std_logic;
			mtl_wr_data_i        : out std_logic_vector(4 * DQ_WIDTH - 1 downto 0);
			mtl_wr_mask_i        : out std_logic_vector(MTL_MASK_WIDTH - 1 downto 0);
			mtl_rd_last_i        : in  std_logic;
			mtl_rd_valid_i       : in  std_logic;
			mtl_rd_accept_i      : out std_logic;
			mtl_rd_data_i        : in  std_logic_vector(4 * DQ_WIDTH - 1 downto 0);
			-- dma controll interface
			dma_addr_special_i   : in  std_logic;
			dma_addr_i           : in  std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);
			dma_rd_i             : in  std_logic;
			dma_rd_data_i        : out  std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);
			dma_wr_i             : in  std_logic;
			dma_wr_data_i        : in  std_logic_vector(DMA_DATA_WIDTH - 1 downto 0)
		);
	end component;

	-- dma controll address space (N=Request_size_words)
	-- dma_addr_special_i=0:
	-- 	0 to N-1: cache_line - N words recently loaded from memory
	-- dma_addr_special=1:
	-- N       : addr_reg    - Address for memory op (must be 64 byte aligned)
	-- N+1     : cmd/status  - Command (0:Load line, 1:Store line) using the addr_reg
	--						 - Status (0:Ready, 1:Busy) after last issued command
	constant DMA_OFFSET_ADDR_REG : integer := 0;
	constant DMA_OFFSET_CMD_STAT : integer := 1;

	constant DMA_CMD_LOAD_LINE  : integer := 0;
	constant DMA_CMD_STORE_LINE : integer := 1;

	constant DMA_STATUS_READY : integer := 0;
	constant DMA_STATUS_BUSY  : integer := 1;

	-- Convert constant to std_logic_vector
	function const2slv(const : Natural; reference : std_logic_vector) return std_logic_vector;

	function const2slv(const : Natural; length : Natural) return std_logic_vector;

end dma_controller_dtl_cmp_pkg;

package body dma_controller_dtl_cmp_pkg is

	-- Convert constant to std_logic_vector
	function const2slv(const : Natural; reference : std_logic_vector) return std_logic_vector is
	begin
		return std_logic_vector(to_unsigned(const, reference'length));
	end function const2slv;

	function const2slv(const : Natural; length : Natural) return std_logic_vector is
	begin
		return std_logic_vector(to_unsigned(const, length));
	end function const2slv;
end package body dma_controller_dtl_cmp_pkg;

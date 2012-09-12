library ieee;
use ieee.std_logic_1164.all;

entity dma_controller_dtl is
	generic(
		DQ_WIDTH            : integer := 8;
		MTL_MASK_WIDTH      : integer := 4;
		MTL_SIZE_WIDTH      : integer := 5; -- TODO: make sure this automatically wraps to 0 when the message is complete!
		MTL_ADDR_WIDTH      : integer := 32;
		-- Request size in bytes
		GEN_REQUEST_SIZE    : integer := 64;
		-- DMA control interface
		DMA_ADDR_WIDTH      : integer := 5;
		DMA_DATA_WIDTH      : integer := 32
	);
	port(
		mtl_clk              : in  std_logic;
		mtl_rst_n            : in  std_logic;

		-- dtl interface
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

		dma_addr_special_i   : in  std_logic;
		dma_addr_i           : in  std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);
		dma_rd_i             : in  std_logic;
		dma_rd_data_i        : out std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);
		dma_wr_i             : in  std_logic;
		dma_wr_data_i        : in  std_logic_vector(DMA_DATA_WIDTH - 1 downto 0)
	);

end dma_controller_dtl;

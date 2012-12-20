library IEEE;
use IEEE.std_logic_1164.all;
use ieee.numeric_std.all;


use work.sc_pack.all;

entity sc_mem_if is
--  generic (addr_bits : integer);

  port (
    clk, rst : in std_logic;

    pll_locked         : in    std_logic; 
	 dram_clk_skewed : in std_logic;
	 
    sc_mem_out : in  sc_out_type;
    sc_mem_in  : out sc_in_type;

-- memory interface
        -- SDRAM interface lower chip
        dram0_CLK   : out   std_logic;  -- Clock
        dram0_CKE   : out   std_logic;  -- Clock Enable
        dram0_RAS_n : out   std_logic;  -- Row Address Strobe
        dram0_CAS_n : out   std_logic;  -- Column Address Strobe
        dram0_WE_n  : out   std_logic;  -- Write Enable
        dram0_CS_n  : out   std_logic; -- Chip Select
        dram0_BA_0  : out   std_logic;  -- Bank Address
        dram0_BA_1  : out   std_logic;  -- Bank Address
        dram0_ADDR  : out   std_logic_vector(12 downto 0); -- SDRAM Address
        dram0_UDQM  : out   std_logic;  -- Data mask Upper Byte
        dram0_LDQM  : out   std_logic;  -- Data mask Lower Byte
        -- SDRAM interface highier chip
        dram1_CLK   : out   std_logic;  -- Clock
        dram1_CKE   : out   std_logic;  -- Clock Enable
        dram1_RAS_n : out   std_logic;  -- Row Address Strobe
        dram1_CAS_n : out   std_logic;  -- Column Address Strobe
        dram1_WE_n  : out   std_logic;  -- Write Enable
        dram1_CS_n  : out   std_logic; -- Chip Select
        dram1_BA_0  : out   std_logic;  -- Bank Address
        dram1_BA_1  : out   std_logic;  -- Bank Address
        dram1_ADDR  : out   std_logic_vector(12 downto 0); -- SDRAM Address
        dram1_UDQM  : out   std_logic;  -- Data mask Upper Byte
        dram1_LDQM  : out   std_logic;  -- Data mask Lower Byte
        -- data bus from both chips
        dram_DQ     : inout std_logic_vector(31 downto 0) -- Data
    );
end sc_mem_if;

architecture rtl of sc_mem_if is
    constant BURST_LENGTH : natural := 1;

--    constant ADDR_WIDTH  : integer := 24; -- 16M (addr) x 4 (bytes) == 64MB 
    constant ADDR_WIDTH  : integer := 23; -- Can JOP support more?
    constant DATA_WIDTH  : integer := 32;
    -- Address Mapping
    constant COL_WIDTH   : integer := 9;
    constant ROW_WIDTH   : integer := 12; -- reduced from 13, due to reduced ADDR_WIDTH
    constant BA_WIDTH    : integer := 2;
    constant CS_WIDTH    : integer := 0;
    constant COL_LOW_BIT : integer := 0;
    constant ROW_LOW_BIT : integer := COL_LOW_BIT + COL_WIDTH; -- 9
    constant BA_LOW_BIT  : integer := ROW_LOW_BIT + ROW_WIDTH; -- 9+13=22
    constant CS_LOW_BIT  : integer := BA_LOW_BIT + BA_WIDTH; -- 22+2=24
    -- SDRAM configuration
    constant SA_WIDTH    : natural := dram0_ADDR'length;

    constant tCLK               : time    := 10 ns; --! Clock period
    constant tINIT_IDLE         : time    := 200 us; --! Inactivity perdiod required during initialization 
    constant INIT_REFRESH_COUNT : natural := 8; --! Number of Refresh commands required during initialization
    constant tCAC_CYCLES        : natural := 2; --! CAS latency
    constant tRRD               : time    := 14 ns; --! Row to Row Delay (ACT[0]-ACT[1])
    constant tRCD               : time    := 20 ns; --! Row to Column Delay (ACT-READ/WRITE)
    constant tRAS               : time    := 45 ns; --! Row Access Strobe (ACT-PRE)
    constant tRC                : time    := 67.5 ns; --! Row Cycle (REF-REF,ACT-ACT)
    constant tRP                : time    := 20 ns; --! Row Precharge (PRE-ACT)
    constant tCCD               : time    := tCLK * 1; --! Column Command Delay Time
    constant tDPL               : time    := 14 ns; --! Input Data to Precharge (DQ_WR-PRE)
    constant tDAL               : time    := 35 ns; --! Input Data to Activate (DQ_WR-ACT/PRE)
    constant tRBD               : time    := tCLK * tCAC_CYCLES; --! Burst Stop to High Impedance (Read)
    constant tWBD               : time    := 0 ns; --! Burst Stop to Input in Invalid (Write)
    constant tPQL               : time    := tCLK * (tCAC_CYCLES - 1); --! Last Output to Auto-Precharge Start (READ)
    constant tQMD               : time    := tCLK * 2; --! DQM to Output (Read)
    constant tDMD               : time    := 0 ns; --! DQM to Input (Write)
    constant tMRD               : time    := 15 ns; --! Mode Register Delay (program time)
    constant tMRD_CYCLES        : natural := 2; --! Mode Register Delay (program time) in Cycles
-- This was used to see how many errors would occur if refresh is performed way to rearly 
--    constant tREF               : time    := 1000*64 ms; --! Refresh Cycle (for each row)
    constant tREF               : time    := 64 ms; --! Refresh Cycle (for each row)

    constant DQ_WIDTH         : integer := 8;
    constant MTL_MASK_WIDTH   : integer := 4;
    --  constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_ADDR_WIDTH   : integer := ADDR_WIDTH;
    -- Request size in bytes
    --  constant GEN_REQUEST_SIZE : integer := 64;
    constant GEN_REQUEST_SIZE : integer := 4 * BURST_LENGTH;
    -- DMA control interface
    constant DMA_ADDR_WIDTH   : integer := 3;
    constant DMA_DATA_WIDTH   : integer := 32;

	 
    signal res_cnt    : std_logic_vector(2 downto 0) := "000";
    attribute altera_attribute : string;
    attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";
    signal rst_n : std_logic;
    
    signal dram_DQM   : std_logic_vector(3 downto 0);
    signal dram_BA    : std_logic_vector(1 downto 0);
    signal dram_ADDR  : std_logic_vector(12 downto 0);
    signal dram_CAS_N : std_logic;
    signal dram_RAS_N : std_logic;
    signal dram_CKE   : std_logic;
    signal dram_CS_N  : std_logic_vector(2**CS_WIDTH-1 downto 0);
    signal dram_WE_N  : std_logic;

    signal dma_addr_special_i : std_logic;
    signal dma_addr_i         : std_logic_vector(DMA_ADDR_WIDTH-1 downto 0);
    signal dma_rd_i           : std_logic;
    signal dma_rd_data_i      : std_logic_vector(31 downto 0);
    signal dma_wr_i           : std_logic;
    signal dma_wr_data_i      : std_logic_vector(31 downto 0);

    signal ocp_MCmd           : std_logic_vector(2 downto 0);
    signal ocp_MCmd_doRefresh : std_logic;
    signal ocp_MAddr          : std_logic_vector(ADDR_WIDTH - 1 downto 0);
    signal ocp_SCmdAccept     : std_logic;
    signal ocp_MData          : std_logic_vector(DATA_WIDTH - 1 downto 0);
    signal ocp_MDataByteEn    : std_logic_vector(DATA_WIDTH / 8 - 1 downto 0);
    signal ocp_MDataValid     : std_logic;
    signal ocp_MDataLast      : std_logic;
    signal ocp_SDataAccept    : std_logic;
    signal ocp_SData          : std_logic_vector(DATA_WIDTH - 1 downto 0);
    signal ocp_SResp          : std_logic;
    signal ocp_SRespLast      : std_logic;

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

	 
	 -- attribute ALTERA_ATTRIBUTE : string;
    attribute ALTERA_ATTRIBUTE of dram0_BA_0 : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_BA_1 : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_CAS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_RAS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_WE_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_CS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_CKE : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_ADDR : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_UDQM : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram0_LDQM : signal is "FAST_OUTPUT_REGISTER=ON";

    attribute ALTERA_ATTRIBUTE of dram1_BA_0 : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_BA_1 : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_CAS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_RAS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_WE_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_CS_n : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_CKE : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_ADDR : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_UDQM : signal is "FAST_OUTPUT_REGISTER=ON";
    attribute ALTERA_ATTRIBUTE of dram1_LDQM : signal is "FAST_OUTPUT_REGISTER=ON";

    attribute ALTERA_ATTRIBUTE of dram_DQ : signal is "FAST_INPUT_REGISTER=ON;FAST_OUTPUT_REGISTER=ON";
    --attribute ALTERA_ATTRIBUTE of sdram_DQoe_r: port is "FAST_OUTPUT_REGISTER=ON";
  
begin
	 
	 
--    process(sys_clk, pll_locked)
--    begin
--        if pll_locked = '0' then
--            res_cnt  <= "000";
--            rst  <= '1';
--        elsif rising_edge(sys_clk) then
--            if (res_cnt /= "111") then
--                res_cnt <= std_logic_vector(unsigned(res_cnt) + 1);
--            end if;
--            rst <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
--        end if;
--    end process;
--    rst_n  <= not rst;

--    pll : entity work.de2_70_sdram_pll
--        port map(
--            inclk0 => clk,
--            c0     => sys_clk,
--            c1     => dram_clk,
--            c2     => dram_clk_skew,
--            locked => pll_locked);

   sdr_sdram_inst : entity work.sdr_sdram
        generic map(
            ADDR_WIDTH         => ADDR_WIDTH,
            DATA_WIDTH         => DATA_WIDTH,
            BURST_LENGTH       => BURST_LENGTH,
            SA_WIDTH           => SA_WIDTH,
            CS_WIDTH           => CS_WIDTH,
            CS_LOW_BIT         => CS_LOW_BIT,
            BA_WIDTH           => BA_WIDTH,
            BA_LOW_BIT         => BA_LOW_BIT,
            ROW_WIDTH          => ROW_WIDTH,
            ROW_LOW_BIT        => ROW_LOW_BIT,
            COL_WIDTH          => COL_WIDTH,
            COL_LOW_BIT        => COL_LOW_BIT,
            tCLK               => tCLK,
            tINIT_IDLE         => tINIT_IDLE,
            INIT_REFRESH_COUNT => INIT_REFRESH_COUNT,
            tCAC_CYCLES        => tCAC_CYCLES,
            tRRD               => tRRD,
            tRCD               => tRCD,
            tRAS               => tRAS,
            tRC                => tRC,
            tRP                => tRP,
            tCCD               => tCCD,
            tDPL               => tDPL,
            tDAL               => tDAL,
            tRBD               => tRBD,
            tWBD               => tWBD,
            tPQL               => tPQL,
            tQMD               => tQMD,
            tDMD               => tDMD,
            tMRD_CYCLES        => tMRD_CYCLES,
            tREF               => tREF)
        port map(
            rst                => rst,
            clk                => clk,
            pll_locked         => pll_locked,
            ocp_MCmd           => ocp_MCmd,
--            ocp_MCmd_doRefresh => ocp_MCmd_doRefresh,
            ocp_MAddr          => ocp_MAddr,
            ocp_SCmdAccept     => ocp_SCmdAccept,
            ocp_MData          => ocp_MData,
            ocp_MDataByteEn    => ocp_MDataByteEn,
            ocp_MDataValid     => ocp_MDataValid,
            ocp_MDataLast      => ocp_MDataLast,
            ocp_SDataAccept    => ocp_SDataAccept,
            ocp_SData          => ocp_SData,
            ocp_SResp          => ocp_SResp,
            ocp_SRespLast      => ocp_SRespLast,
            sdram_CKE          => dram_CKE,
            sdram_RAS_n        => dram_RAS_n,
            sdram_CAS_n        => dram_CAS_n,
            sdram_WE_n         => dram_WE_n,
            sdram_CS_n         => dram_CS_n,
            sdram_BA           => dram_BA,
            sdram_SA           => dram_ADDR,
            sdram_DQ           => dram_DQ,
            sdram_DQM          => dram_DQM);

--  process (rst, clk)
--  begin  -- process
--    if rst = '1' then
--      rdy_cnt <= "0000";
--    elsif clk'event and clk = '1' then  -- rising clock edge
--      if sc_mem_out.wr = '1' then
--         rdy_cnt <= c_RAS+c_CL;		
--		elsif sc_mem_out.rd = '1' then
--         rdy_cnt <= c_RAS+c_RAS2WRITE;				
--		elsif rdy_cnt <> "0000" then
--			rdy_cnt <= std_logic_vector(unsigned(rdy_cnt)-1);
--		end if;
--    end if;
--  end process;
--  sc_mem_in.rdy_cnt <= "11" when rdy_cnt(3 downto 2) <> "00" else
--		rdy_cnt(1 downto 0);
  

				
    -- CMD
    ocp_MCmd_doRefresh  <= '0';

  process (clk, rst)
  begin  -- process
    if rst = '1' then 
		sc_mem_in.rdy_cnt <= "11";
    elsif clk'event and clk = '1' then  -- rising clock edge
      -- Hold Cmd and Address
		if sc_mem_out.wr = '1' or sc_mem_out.rd = '1' then
			 ocp_MCmd         <= '0' & sc_mem_out.wr & sc_mem_out.rd;
			 ocp_MAddr        <= sc_mem_out.address(ocp_MAddr'range);
		end if;
      -- Hold Write data
		if sc_mem_out.wr = '1' then
			 ocp_MData       <= sc_mem_out.wr_data;
		end if;
      -- Hold Read result
		if ocp_SResp = '1' then
			sc_mem_in.rd_data <= ocp_SData;
		end if;
		if ocp_MCmd /= "000" and ocp_SCmdAccept = '1' then
   		ocp_MCmd         <= "000";
		end if;
		-- Simple late rdy signalling for now.
		if sc_mem_out.wr = '1' or sc_mem_out.rd = '1' then
			sc_mem_in.rdy_cnt <= "11";
		elsif ocp_SDataAccept = '1' or ocp_SResp = '1' then
			sc_mem_in.rdy_cnt <= "00";
		end if;
    end if;
  end process;

    -- Write 
    ocp_MDataValid  <= '1';
    ocp_MDataLast   <= '1';
    ocp_MDataByteEn <= (others => '1');
    
    dram0_BA_0  <= dram_BA(0);
    dram0_BA_1  <= dram_BA(1);
    dram1_BA_0  <= dram_BA(0);
    dram1_BA_1  <= dram_BA(1);
    dram0_UDQM  <= dram_DQM(1);
    dram0_LDQM  <= dram_DQM(0);
    dram1_UDQM  <= dram_DQM(3);
    dram1_LDQM  <= dram_DQM(2);
    dram0_ADDR  <= dram_ADDR;
    dram1_ADDR  <= dram_ADDR;
    dram0_CAS_N <= dram_CAS_N;
    dram1_CAS_N <= dram_CAS_N;
    dram0_CKE   <= dram_CKE;
    dram1_CKE   <= dram_CKE;
    dram0_CLK   <= dram_clk_skewed;
    dram1_CLK   <= dram_clk_skewed;
    dram0_CS_N  <= dram_CS_N(0);
    dram1_CS_N  <= dram_CS_N(0);
    dram0_RAS_N <= dram_RAS_N;
    dram1_RAS_N <= dram_RAS_N;
    dram0_WE_N  <= dram_WE_N;
    dram1_WE_N  <= dram_WE_N;
	 
	 
end rtl;

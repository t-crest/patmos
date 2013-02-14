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
-- Top level for patmos with sdram
--------------------------------------------------------------------------------
library ieee;

use ieee.std_logic_1164.all;

entity patmos_top is
    port(
        clk         : in    std_logic;
        led         : out   std_logic;
        txd         : out   std_logic;
        rxd         : in    std_logic;
        -- SDRAM interface lower chip
        dram0_CLK   : out   std_logic;  -- Clock
        dram0_CKE   : out   std_logic;  -- Clock Enable
        dram0_RAS_n : out   std_logic;  -- Row Address Strobe
        dram0_CAS_n : out   std_logic;  -- Column Address Strobe
        dram0_WE_n  : out   std_logic;  -- Write Enable
        dram0_CS_n  : out   std_logic;  -- Chip Select
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
        dram1_CS_n  : out   std_logic;  -- Chip Select
        dram1_BA_0  : out   std_logic;  -- Bank Address
        dram1_BA_1  : out   std_logic;  -- Bank Address
        dram1_ADDR  : out   std_logic_vector(12 downto 0); -- SDRAM Address
        dram1_UDQM  : out   std_logic;  -- Data mask Upper Byte
        dram1_LDQM  : out   std_logic;  -- Data mask Lower Byte
        -- data bus from both chips
        dram_DQ     : inout std_logic_vector(31 downto 0) -- Data

    --      oSRAM_A      : out   std_logic_vector(18 downto 0); -- edit
    --      SRAM_DQ      : inout std_logic_vector(31 downto 0); -- edit
    --      oSRAM_CE1_N  : out   std_logic;
    --      oSRAM_OE_N   : out   std_logic;
    --      oSRAM_BE_N   : out   std_logic_vector(3 downto 0);
    --      oSRAM_WE_N   : out   std_logic;
    --      oSRAM_GW_N   : out   std_logic;
    --      oSRAM_CLK    : out   std_logic;
    --      oSRAM_ADSC_N : out   std_logic;
    --      oSRAM_ADSP_N : out   std_logic;
    --      oSRAM_ADV_N  : out   std_logic;
    --      oSRAM_CE2    : out   std_logic;
    --      oSRAM_CE3_N  : out   std_logic
    );
end entity patmos_top;

library ieee;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

use work.patmos_config.all;

use work.sdram_config.all;
use work.sdram_controller_interface.all;

architecture RTL of patmos_top is
    constant BURST_LENGTH : natural           := 8;
    -- 100MHz, 2 cycles read data latency
    constant tCLK_PERIOD  : time              := 10 ns;
    constant CAS_LATENCY  : natural           := 2;
    constant SDRAM        : sdram_config_type := GetSDRAMParameters(tCLK_PERIOD, CAS_LATENCY);

    -- Address Mapping => (bank & row & column)
    constant CS_WIDTH    : integer := 0; -- 1 rank
    constant COL_LOW_BIT : integer := 0;
    constant ROW_LOW_BIT : integer := COL_LOW_BIT + SDRAM.COL_WIDTH; -- 9
    constant BA_LOW_BIT  : integer := ROW_LOW_BIT + SDRAM.ROW_WIDTH; -- 9+13=22
    constant CS_LOW_BIT  : integer := BA_LOW_BIT + SDRAM.BA_WIDTH; -- 22+2=24


    constant DQ_WIDTH         : integer := 8;
    constant MTL_MASK_WIDTH   : integer := 4;
    --  constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_ADDR_WIDTH   : integer := SDRAM_ADDR_WIDTH;
    -- Request size in bytes
    --  constant GEN_REQUEST_SIZE : integer := 64;
    constant GEN_REQUEST_SIZE : integer := 4 * BURST_LENGTH;
    -- DMA control interface
    constant DMA_ADDR_WIDTH   : integer := 3;
    constant DMA_DATA_WIDTH   : integer := 32;

    signal res_cnt : std_logic_vector(2 downto 0) := "000";
    attribute altera_attribute : string;
    attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";
    signal rst, rst_n : std_logic;
    signal pll_locked : std_logic;

    signal sys_clk       : std_logic;
    signal dram_clk_skew : std_logic;
    signal dram_clk      : std_logic;

    signal dram_DQM   : std_logic_vector(3 downto 0);
    signal dram_BA    : std_logic_vector(1 downto 0);
    signal dram_ADDR  : std_logic_vector(12 downto 0);
    signal dram_CAS_N : std_logic;
    signal dram_RAS_N : std_logic;
    signal dram_CKE   : std_logic;
    signal dram_CS_N  : std_logic_vector(2 ** CS_WIDTH - 1 downto 0);
    signal dram_WE_N  : std_logic;

    signal dma_addr_special_i : std_logic;
    signal dma_addr_i         : std_logic_vector(DMA_ADDR_WIDTH - 1 downto 0);
    signal dma_rd_i           : std_logic;
    signal dma_rd_data_i      : std_logic_vector(31 downto 0);
    signal dma_wr_i           : std_logic;
    signal dma_wr_data_i      : std_logic_vector(31 downto 0);

    -- Edgar: have both interfaces for now selectable thourough USE_SDRAM config param

    -- SDRAM through I/O device
    signal ocpMaster : SDRAM_controller_master_type;
    signal ocpSlave  : SDRAM_controller_slave_type;

    -- SDRAM thruogh direct connection 
    signal gm_master : SDRAM_controller_master_type;
    signal gm_slave  : SDRAM_controller_slave_type;

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
    signal mtl_rd_last_i        : std_logic;
    signal mtl_rd_valid_i       : std_logic;
    signal mtl_rd_accept_i      : std_logic;
    signal mtl_rd_data_i        : std_logic_vector(4 * DQ_WIDTH - 1 downto 0);

    --    attribute ALTERA_ATTRIBUTE : string;
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
    assert 2 ** DMA_ADDR_WIDTH = BURST_LENGTH report "BURST_LENGTH should be == 2**DMA_ADDR_WIDTH" severity failure;

    process(sys_clk, pll_locked)
    begin
        if pll_locked = '0' then
            res_cnt <= "000";
            rst     <= '1';
        elsif rising_edge(sys_clk) then
            if (res_cnt /= "111") then
                res_cnt <= std_logic_vector(unsigned(res_cnt) + 1);
            end if;
            rst <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
        end if;
    end process;
    rst_n <= not rst;

    pll : entity work.de2_70_sdram_pll
        port map(
            inclk0 => clk,
            c0     => sys_clk,
            c1     => dram_clk,
            c2     => dram_clk_skew,
            locked => pll_locked);

    patmos_sdram_inst : entity work.patmos_sdram
        port map(clk                          => dram_clk,
                 --                 rst                => rst,

                 led                          => led,
                 txd                          => txd,
                 rxd                          => rxd,
                 dma_addr_special_i           => dma_addr_special_i,
                 dma_addr_i(dma_addr_i'range) => dma_addr_i,
                 dma_rd_i                     => dma_rd_i,
                 dma_rd_data_i                => dma_rd_data_i,
                 dma_wr_i                     => dma_wr_i,
                 dma_wr_data_i                => dma_wr_data_i,
                 gm_slave                     => gm_slave,
                 gm_master                    => gm_master
        );

    sdr_sdram_inst : entity work.sdr_sdram
        generic map(
            USE_AUTOMATIC_REFRESH => SDRAM_USE_AUTOMATIC_REFRESH,
            BURST_LENGTH          => BURST_LENGTH,
            SDRAM                 => SDRAM,
            CS_WIDTH              => CS_WIDTH,
            CS_LOW_BIT            => CS_LOW_BIT,
            BA_LOW_BIT            => BA_LOW_BIT,
            ROW_LOW_BIT           => ROW_LOW_BIT,
            COL_LOW_BIT           => COL_LOW_BIT)
        port map(
            rst         => rst,
            clk         => dram_clk,
            pll_locked  => pll_locked,
            ocpSlave    => ocpSlave,
            ocpMaster   => ocpMaster,
            sdram_CKE   => dram_CKE,
            sdram_RAS_n => dram_RAS_n,
            sdram_CAS_n => dram_CAS_n,
            sdram_WE_n  => dram_WE_n,
            sdram_CS_n  => dram_CS_n,
            sdram_BA    => dram_BA,
            sdram_SA    => dram_ADDR,
            sdram_DQ    => dram_DQ,
            sdram_DQM   => dram_DQM);

    GM_SDRAM : if USE_GLOBAL_MEMORY_SDRAM generate
        gm_slave  <= ocpSlave;
        ocpMaster <= gm_master;

        dma_rd_data_i <= (others => '0');
    end generate GM_SDRAM;

    IO_SDRAM : if not USE_GLOBAL_MEMORY_SDRAM generate
        sdram_io_controller : work.dma_controller_dtl_cmp_pkg.dma_controller_dtl
            generic map(
                DQ_WIDTH         => DQ_WIDTH,
                MTL_MASK_WIDTH   => MTL_MASK_WIDTH,
                MTL_SIZE_WIDTH   => MTL_SIZE_WIDTH,
                MTL_ADDR_WIDTH   => MTL_ADDR_WIDTH,
                GEN_REQUEST_SIZE => GEN_REQUEST_SIZE,
                DMA_ADDR_WIDTH   => DMA_ADDR_WIDTH,
                DMA_DATA_WIDTH   => DMA_DATA_WIDTH)
            port map(
                mtl_clk              => dram_clk,
                mtl_rst_n            => rst_n,
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

        -- CMD
        ocpMaster.MFlag_CmdRefresh <= '0'; -- Use automatic refresh
        ocpMaster.MCmd             <= '0' & (mtl_cmd_valid_i and (not mtl_cmd_read_i)) & (mtl_cmd_valid_i and mtl_cmd_read_i);
        ocpMaster.MAddr            <= mtl_cmd_addr_i;
        mtl_cmd_accept_i           <= ocpSlave.SCmdAccept;
        assert (to_integer(unsigned(mtl_cmd_block_size_i)) + 1) = BURST_LENGTH or mtl_cmd_valid_i /= '1' report "Unsupported block size" severity failure;
        -- Write 
        ocpMaster.MData       <= mtl_wr_data_i;
        -- ocpMaster.MDataValid  <= mtl_wr_valid_i;
        -- ocpMaster.MDataLast   <= mtl_wr_last_i;
        ocpMaster.MDataByteEn <= mtl_wr_mask_i;
        mtl_wr_accept_i       <= ocpSlave.SDataAccept;
        -- Read 
        mtl_rd_data_i         <= ocpSlave.SData;
        mtl_rd_valid_i        <= ocpSlave.SResp;
        mtl_rd_last_i         <= ocpSlave.SRespLast;
        assert mtl_rd_accept_i = '1' or mtl_rd_valid_i /= '1' report "Protocol error: wrapper not ready to accept data" severity failure;
    -- ocpMaster.MRespAccept not used
    end generate IO_SDRAM;

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
    dram0_CLK   <= dram_clk_skew;
    dram1_CLK   <= dram_clk_skew;
    dram0_CS_N  <= dram_CS_N(0);
    dram1_CS_N  <= dram_CS_N(0);
    dram0_RAS_N <= dram_RAS_N;
    dram1_RAS_N <= dram_RAS_N;
    dram0_WE_N  <= dram_WE_N;
    dram1_WE_N  <= dram_WE_N;

end architecture RTL;



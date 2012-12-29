library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity sdr_sdram_dma_controller_tb is
end entity sdr_sdram_dma_controller_tb;

use std.textio.all;
use work.dma_controller_dtl_cmp_pkg.all;

architecture RTL of sdr_sdram_dma_controller_tb is
    constant BURST_LENGTH : natural := 8;

    constant ADDR_WIDTH  : integer := 23;
    constant DATA_WIDTH  : integer := 32;
    -- Address Mapping
    constant COL_WIDTH   : integer := 9;
    constant ROW_WIDTH   : integer := 12;
    constant BA_WIDTH    : integer := 2;
    constant CS_WIDTH    : integer := 0;
    constant COL_LOW_BIT : integer := 0;
    constant ROW_LOW_BIT : integer := COL_WIDTH; -- 9
    constant BA_LOW_BIT  : integer := ROW_LOW_BIT + ROW_WIDTH; -- 9+13=22
    constant CS_LOW_BIT  : integer := BA_LOW_BIT + BA_WIDTH; -- 22+2=24
    -- SDRAM configuration
    constant SA_WIDTH    : natural := ROW_WIDTH;

    constant tCLK               : time    := 10 ns; --! Clock period
    constant tINIT_IDLE         : time    := 50 ns; -- 200 us; --! Inactivity perdiod required during initialization 
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
    constant tREF               : time    := tCLK*(2**ROW_WIDTH)*10; --! Refresh Cycle (for each row)

    constant DQ_WIDTH         : integer := 8;
    constant MTL_MASK_WIDTH   : integer := 4;
    --	constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_SIZE_WIDTH   : integer := 5;
    constant MTL_ADDR_WIDTH   : integer := ADDR_WIDTH;
    -- Request size in bytes
    --	constant GEN_REQUEST_SIZE : integer := 64;
    constant GEN_REQUEST_SIZE : integer := 4 * BURST_LENGTH;
    -- DMA control interface
    constant DMA_ADDR_WIDTH   : integer := 3;
    constant DMA_DATA_WIDTH   : integer := 32;

    constant DATA_WIDTH_BITS : integer := 4 * DQ_WIDTH;
    constant NWORDS_PER_CMD  : integer := GEN_REQUEST_SIZE * 8 / DATA_WIDTH_BITS;

    constant DEBUG_SHOW_MEMORY_TRANSACTIONS    : boolean := true;
    constant DEBUG_SHOW_CACHELINE_TRANSACTIONS : boolean := true;

    --===========================================================
    -- Default timing parameters for mt48lc2m32b2_2
    --===========================================================
    --    -- Timing Parameters for -6 and CAS Latency = 2
    --    constant tOH  : TIME    := 2.0 ns;
    --    constant tMRD : INTEGER := 2;       -- 2 Clk Cycles
    --    constant tRAS : TIME    := 42.0 ns;
    --    constant tRC  : TIME    := 60.0 ns;
    --    constant tRCD : TIME    := 18.0 ns;
    --    constant tRP  : TIME    := 18.0 ns;
    --    constant tRRD : TIME    := 12.0 ns;
    --    constant tWRa : TIME    := 6.0 ns;  -- A2 Version - Auto precharge mode only (1 Clk + 6 ns)
    --    constant tWRp : TIME    := 12.0 ns; -- A2 Version - Precharge mode only (12 ns)
    --
    --    constant tAS  : TIME := 1.5 ns;
    --    constant tCH  : TIME := 2.5 ns;
    --    constant tCL  : TIME := 2.5 ns;
    --    constant tCK  : TIME := 10.5 ns;
    --    constant tDH  : TIME := 1.0 ns;
    --    constant tDS  : TIME := 1.5 ns;
    --    constant tCKH : TIME := 1.0 ns;
    --    constant tCKS : TIME := 1.5 ns;
    --    constant tCMH : TIME := 0 ns;
    --    constant tCMS : TIME := 0 ns;

    --===========================================================
    -- Timing parameters for IS42S16160B: speed grade -7, tCL=3
    --===========================================================
    constant tOH  : TIME := 3.0 ns;
    --    constant tMRD : INTEGER := 2;       -- 2 Clk Cycles
    --    constant tRAS : TIME    := 45.0 ns;
    --    constant tRC  : TIME    := 67.5 ns;
    --    constant tRCD : TIME    := 20.0 ns;
    --    constant tRP  : TIME    := 20.0 ns;
    --    constant tRRD : TIME    := 14.0 ns;
    constant tWRa : TIME := 6.0 ns - 6 ns; -- A2 Version - Auto precharge mode only (1 Clk + 6 ns)
    constant tWRp : TIME := 20 ns + 14.0 ns; -- A2 Version - Precharge mode only (12 ns)

    constant tCH  : TIME := 2.5 ns;
    constant tCL  : TIME := 2.5 ns;
    constant tCK  : TIME := 10 ns;
    constant tAS  : TIME := 2 ns;
    constant tDS  : TIME := 2 ns;
    constant tCKS : TIME := tDS;
    constant tCMS : TIME := tDS;
    constant tDH  : TIME := 0 ns;       --1 ns; -- We use 0 delay behavioural model, so Hold violation checks are disabled
    constant tCKH : TIME := tDH;
    constant tCMH : TIME := tDH;

    subtype dma_word_t is std_logic_vector(DMA_DATA_WIDTH - 1 downto 0);

    signal clk                  : std_logic;
    signal rst_n                : std_logic;
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

    signal end_of_sim : std_logic := '0';

    component mt48lc8m16a2
        PORT(
            Dq    : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0) := (OTHERS => 'Z');
            Addr  : IN    STD_LOGIC_VECTOR(11 DOWNTO 0) := (OTHERS => '0');
            Ba    : IN    STD_LOGIC_VECTOR              := "00";
            Clk   : IN    STD_LOGIC                     := '0';
            Cke   : IN    STD_LOGIC                     := '0';
            Cs_n  : IN    STD_LOGIC                     := '1';
            Ras_n : IN    STD_LOGIC                     := '0';
            Cas_n : IN    STD_LOGIC                     := '0';
            We_n  : IN    STD_LOGIC                     := '0';
            Dqm   : IN    STD_LOGIC_VECTOR(1 DOWNTO 0)  := (OTHERS => '0')
        );
    END component;

    shared variable L : line;

    signal clk_ram     : std_logic := '0';
    signal rst         : std_logic;
    signal sdram_sa    : std_logic_vector(SA_WIDTH - 1 downto 0);
    signal sdram_ba    : std_logic_vector(BA_WIDTH - 1 downto 0);
    signal sdram_cs_n  : std_logic_vector(2 ** CS_WIDTH - 1 downto 0);
    signal sdram_cke   : std_logic;
    signal sdram_ras_n : std_logic;
    signal sdram_cas_n : std_logic;
    signal sdram_we_n  : std_logic;
    signal sdram_dq    : std_logic_vector(DATA_WIDTH - 1 downto 0);
    signal sdram_dqm   : std_logic_vector(DATA_WIDTH / 8 - 1 downto 0);

    constant USE_DIMM_MODEL   : boolean := true;
    constant USE_AUTOMATIC_REFRESH : boolean := true;
    signal ocp_MCmd           : std_logic_vector(2 downto 0);
    signal ocp_SFlag_CmdRefresh : std_logic := '0';
    signal ocp_MFlag_RefreshAccept : std_logic;
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
    signal sdram_dq_out : STD_LOGIC_VECTOR(DATA_WIDTH - 1 DOWNTO 0);
    signal sdram_dq_dir : STD_LOGIC_VECTOR(3 downto 0);
begin
    -- Wrapper
    dut : work.dma_controller_dtl_cmp_pkg.dma_controller_dtl
        generic map(
            DQ_WIDTH         => DQ_WIDTH,
            MTL_MASK_WIDTH   => MTL_MASK_WIDTH,
            MTL_SIZE_WIDTH   => MTL_SIZE_WIDTH,
            MTL_ADDR_WIDTH   => MTL_ADDR_WIDTH,
            GEN_REQUEST_SIZE => GEN_REQUEST_SIZE,
            DMA_ADDR_WIDTH   => DMA_ADDR_WIDTH,
            DMA_DATA_WIDTH   => DMA_DATA_WIDTH)
        port map(
            mtl_clk              => clk,
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
    -- The Controller
    sdr_sdram_inst : entity work.sdr_sdram
        generic map(
	    USE_AUTOMATIC_REFRESH => USE_AUTOMATIC_REFRESH,
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
            pll_locked         => '1',
            ocp_MCmd           => ocp_MCmd,
            ocp_SFlag_CmdRefresh => ocp_SFlag_CmdRefresh,
            ocp_MFlag_RefreshAccept => ocp_MFlag_RefreshAccept,
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
            sdram_CKE          => sdram_CKE,
            sdram_RAS_n        => sdram_RAS_n,
            sdram_CAS_n        => sdram_CAS_n,
            sdram_WE_n         => sdram_WE_n,
            sdram_CS_n         => sdram_CS_n,
            sdram_BA           => sdram_BA,
            sdram_SA           => sdram_SA,
            sdram_DQ           => sdram_DQ,
            sdram_DQM          => sdram_DQM);

    -- CMD
    ocp_SFlag_CmdRefresh  <= '0';
    ocp_MCmd         <= '0' & (mtl_cmd_valid_i and (not mtl_cmd_read_i)) & (mtl_cmd_valid_i and mtl_cmd_read_i);
    ocp_MAddr        <= mtl_cmd_addr_i;
    mtl_cmd_accept_i <= ocp_SCmdAccept;
    assert (to_integer(unsigned(mtl_cmd_block_size_i)) + 1) = BURST_LENGTH or mtl_cmd_valid_i /= '1' report "Unsupported block size" severity failure;
    -- Write 
    ocp_MData       <= mtl_wr_data_i;
    ocp_MDataValid  <= mtl_wr_valid_i;
    ocp_MDataLast   <= mtl_wr_last_i;
    ocp_MDataByteEn <= mtl_wr_mask_i;
    mtl_wr_accept_i <= ocp_SDataAccept;
    -- Read 
    mtl_rd_data_i   <= ocp_SData;
    mtl_rd_valid_i  <= ocp_SResp;
    mtl_rd_last_i   <= ocp_SRespLast;
    assert mtl_rd_accept_i = '1' or mtl_rd_valid_i /= '1' report "Protocol error: wrapper not ready to accept data" severity failure;
    -- ocp_MRespAccept not used

    gen_delay_dq_out : for i in sdram_dq_dir'range generate
        sdram_dq((i+1)*8-1 downto i*8)  <= sdram_dq_out((i+1)*8-1 downto i*8)'delayed(tCLK/10) when sdram_dq_dir'delayed(tCLK/10)(i)='1' else (others => 'Z');
    end generate gen_delay_dq_out;

    
    -- The SDRAMs
    dimm : if USE_DIMM_MODEL generate
        chips : for i in 0 to 2 ** CS_WIDTH - 1 generate
            B0 : entity work.mt48lc2m32b2
                generic map(
                    tOH       => tOH,
                    tMRD      => tMRD_CYCLES,
                    tRAS      => tRAS,
                    tRC       => tRC,
                    tRCD      => tRCD,
                    tRP       => tRP,
                    tRRD      => tRRD,
                    tWRa      => tWRa,
                    tWRp      => tWRp,
                    tAS       => tAS,
                    tCH       => tCH,
                    tCL       => tCL,
                    tCK       => tCK,
                    tDH       => tDH,
                    tDS       => tDS,
                    tCKH      => tCKH,
                    tCKS      => tCKS,
                    tCMH      => tCMH,
                    tCMS      => tCMS,
                    addr_bits => ROW_WIDTH,
                    data_bits => DATA_WIDTH,
                    col_bits  => COL_WIDTH)
                port map(
                    Dq_in  => sdram_dq'delayed(tCLK/10),
                    Dq_out => sdram_dq_out,
                    Dq_dir => sdram_dq_dir,
                    Addr   => sdram_sa'delayed(tCLK/10),
                    Ba     => sdram_Ba'delayed(tCLK/10),
--                    Clk    => clk_ram'delayed(tCLK/2),
                    Clk    => clk_ram,
                    Cke    => sdram_cke'delayed(tCLK/10),
                    Cs_n   => sdram_cs_n(i)'delayed(tCLK/10),
                    Ras_n  => sdram_ras_n'delayed(tCLK/10),
                    Cas_n  => sdram_cas_n'delayed(tCLK/10),
                    We_n   => sdram_We_n'delayed(tCLK/10),
                    Dqm    => sdram_Dqm'delayed(tCLK/10)  
                );
        end generate chips;
    end generate dimm;
    not_dimm : if not USE_DIMM_MODEL generate
        chips : for i in 0 to 2 ** CS_WIDTH - 1 generate
            B00 : mt48lc8m16a2 port map(
                    Dq    => sdram_dq(15 downto 0),
                    Addr  => sdram_sa(11 downto 0),
                    Ba    => sdram_ba,
                    CLK   => clk_ram,
                    Cke   => sdram_cke,
                    Cs_n  => sdram_cs_n(i),
                    Cas_n => sdram_cas_n,
                    Ras_n => sdram_ras_n,
                    We_n  => sdram_we_n,
                    Dqm   => sdram_dqm(1 downto 0)
                );
            B01 : mt48lc8m16a2 port map(
                    Dq    => sdram_dq(31 downto 16),
                    Addr  => sdram_sa(11 downto 0),
                    Ba    => sdram_ba,
                    CLK   => clk_ram,
                    Cke   => sdram_cke,
                    Cs_n  => sdram_cs_n(i),
                    Cas_n => sdram_cas_n,
                    Ras_n => sdram_ras_n,
                    We_n  => sdram_we_n,
                    Dqm   => sdram_dqm(3 downto 2)
                );
        end generate chips;
    end generate not_dimm;

    clock_driver : process
    begin
        if end_of_sim = '0' then
            -- generate both clocks to have 0 delta_delay seperation
            clk <= '1';
            clk_ram  <= '1';
            wait for tCLK / 2;
            clk <= '0';
            clk_ram  <= '0';
            wait for tCLK / 2;
        end if;
    end process clock_driver;
    rst_n <= '0', '1' after tCLK * 2.2;
    rst  <= not rst_n;

    show_cache_line_transactions : if DEBUG_SHOW_CACHELINE_TRANSACTIONS generate
        cl_monitor : process
        begin
            wait until rising_edge(clk);
            if dma_rd_i = '1' and dma_addr_special_i = '0' then
                report "CL RD " & integer'image(to_integer(signed(dma_rd_data_i))) & " at " & integer'image(to_integer(unsigned(dma_addr_i)));
            end if;
            if dma_wr_i = '1' and dma_addr_special_i = '0' then
                report "CL WR " & integer'image(to_integer(signed(dma_wr_data_i))) & " at " & integer'image(to_integer(unsigned(dma_addr_i)));
            end if;
        end process cl_monitor;
    end generate show_cache_line_transactions;

    show_memory_transactions : if DEBUG_SHOW_MEMORY_TRANSACTIONS generate
        memory_monitor : process
            variable addr : integer;
        begin
            wait until rising_edge(clk);
            if mtl_cmd_accept_i = '1' and mtl_cmd_valid_i = '1' then
                addr := TO_INTEGER(unsigned(mtl_cmd_addr_i)) / 4; -- the mtl_cmd_addr_i is byte addressable
            end if;

            if mtl_rd_accept_i = '1' and mtl_rd_valid_i = '1' then
                report "MEM RD " & integer'image(to_integer(signed(mtl_rd_data_i))) & " at " & integer'image(addr) & " last=" & std_logic'image(mtl_rd_last_i);
                addr := addr + 1;
            end if;
            if mtl_wr_accept_i = '1' and mtl_wr_valid_i = '1' then
                report "MEM WR " & integer'image(to_integer(signed(mtl_wr_data_i))) & " at " & integer'image(addr) & " last=" & std_logic'image(mtl_wr_last_i);
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
            wait until rising_edge(clk);

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
            wait until rising_edge(clk);
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

        variable value : natural;

        constant A2 : integer := NWORDS_PER_CMD / 2 - 1;
        constant A3 : integer := NWORDS_PER_CMD - 1;

    begin
        wait until rst_n = '1';
        waitReady;

	if (NWORDS_PER_CMD >=4) then
		controllerWrite('0', 0, 1);
		controllerWrite('0', A2, 6);
		controllerWrite('0', A3, 16);
		controllerRead('0', 0, value);
		assert value = 1 report "T1 cache word write/read error at 0" severity error;
		controllerRead('0', A2, value);
		assert value = 6 report "T1 cache word write/read error at middle" severity error;
		controllerRead('0', A3, value);
		assert value = 16 report "T1 cache word write/read error at last" severity error;

		memoryLineOp(0, loStoreLine);
		memoryLineOp(1, loLoadLine);
		controllerRead('0', 0, value);
		assert value = 0 report "T2 cache word write/read error at 0" severity error;
		controllerRead('0', A2, value);
		assert value = 0 report "T2 cache word write/read error at middle" severity error;
		controllerRead('0', A3, value);
		assert value = 0 report "T2 cache word write/read error at last" severity error;

		memoryLineOp(0, loLoadLine);
		controllerRead('0', 0, value);
		assert value = 1 report "T3 cache word write/read error at 0" severity error;
		controllerRead('0', A2, value);
		assert value = 6 report "T3 cache word write/read error at middle" severity error;
		controllerRead('0', A3, value);
		assert value = 16 report "T3 cache word write/read error at last" severity error;
	end if;
        
        report "Writing inc pattern";
        for i in 0 to NWORDS_PER_CMD*2-1 loop
            if (not USE_AUTOMATIC_REFRESH and (i mod 10 = 0)) then
                ocp_SFlag_CmdRefresh <= '1';
                wait until rising_edge(clk) and ocp_MFlag_RefreshAccept = '1';
            end if;
            controllerWrite('0', (i mod NWORDS_PER_CMD), i);
            if (i mod NWORDS_PER_CMD) = NWORDS_PER_CMD-1 then
                memoryLineOp(i/NWORDS_PER_CMD, loStoreLine);
            end if;
        end loop;
        
        report "Reading inc pattern";
        for i in 0 to NWORDS_PER_CMD*2-1 loop
            if (not USE_AUTOMATIC_REFRESH and (i mod 10 = 0)) then
                ocp_SFlag_CmdRefresh <= '1';
                wait until rising_edge(clk) and ocp_MFlag_RefreshAccept = '1';
            end if;
            if (i mod NWORDS_PER_CMD) = 0 then
                memoryLineOp(i/NWORDS_PER_CMD, loLoadLine);
            end if;
            controllerRead('0', i mod NWORDS_PER_CMD, value);
            assert value = i report "Word Read failed at "& natural'image(i) severity error;
        end loop;

        report "OK.  Test Finished!";
        end_of_sim <= '1';
        report "OK.  Test Finished!" severity failure;
    end process control_test;

end architecture RTL;

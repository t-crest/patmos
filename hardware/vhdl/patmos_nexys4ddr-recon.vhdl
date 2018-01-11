--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--

--
-- VHDL top level for Patmos on the Digilent/Xilinx Nexys4DDR board with off-chip memory and reconfigration
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.ocp.all;
use work.icap_ctrl_defs.all;
use work.icap_ctrl_config.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity patmos_top is
	port(
		clk_in               : in    std_logic;
		cpu_reset_btn        : in    std_logic;

		green_leds           : out   std_logic_vector(15 downto 0); -- (15) -> LD15 ... LD0 <- (0)
		rgb_leds             : out   std_logic_vector(5 downto 0); -- (5) -> LD17_R LD17_G LD17_B | LD16_R LD16_G LD16_B <- (0)
		seven_segments       : out   std_logic_vector(7 downto 0); -- (7) -> DP CG CF CE CD CC CB CA <- (0)
		seven_segments_drive : out   std_logic_vector(7 downto 0); -- (7) -> AN7 ... AN0 <- (0)
		buttons              : in    std_logic_vector(4 downto 0); -- (4) -> BTNT BTNR BTND BTNL BTNC <- (0)
		switches             : in    std_logic_vector(15 downto 0); -- (15) -> SW15 ... SW0 <- (0)

		--TXD, RXD naming uses terminal-centric naming convention
		uart_txd             : in    std_logic;
		uart_rxd             : out   std_logic;

		--DDR2 pins
		ddr2_dq              : inout std_logic_vector(15 downto 0);
		ddr2_dqs_n           : inout std_logic_vector(1 downto 0);
		ddr2_dqs_p           : inout std_logic_vector(1 downto 0);
		ddr2_addr            : out   std_logic_vector(12 downto 0);
		ddr2_ba              : out   std_logic_vector(2 downto 0);
		ddr2_ras_n           : out   std_logic;
		ddr2_cas_n           : out   std_logic;
		ddr2_we_n            : out   std_logic;
		ddr2_ck_p            : out   std_logic_vector(0 to 0);
		ddr2_ck_n            : out   std_logic_vector(0 to 0);
		ddr2_cke             : out   std_logic_vector(0 to 0);
		ddr2_cs_n            : out   std_logic_vector(0 to 0);
		ddr2_dm              : out   std_logic_vector(1 downto 0);
		ddr2_odt             : out   std_logic_vector(0 to 0)
	);
end entity patmos_top;

architecture rtl of patmos_top is
	component Patmos is
		port(
			clk                           : in  std_logic;
			reset                         : in  std_logic;

			io_comConf_M_Cmd              : out std_logic_vector(2 downto 0);
			io_comConf_M_Addr             : out std_logic_vector(31 downto 0);
			io_comConf_M_Data             : out std_logic_vector(31 downto 0);
			io_comConf_M_ByteEn           : out std_logic_vector(3 downto 0);
			io_comConf_M_RespAccept       : out std_logic;
			io_comConf_S_Resp             : in  std_logic_vector(1 downto 0);
			io_comConf_S_Data             : in  std_logic_vector(31 downto 0);
			io_comConf_S_CmdAccept        : in  std_logic;

			io_comSpm_M_Cmd               : out std_logic_vector(2 downto 0);
			io_comSpm_M_Addr              : out std_logic_vector(31 downto 0);
			io_comSpm_M_Data              : out std_logic_vector(31 downto 0);
			io_comSpm_M_ByteEn            : out std_logic_vector(3 downto 0);
			io_comSpm_S_Resp              : in  std_logic_vector(1 downto 0);
			io_comSpm_S_Data              : in  std_logic_vector(31 downto 0);

			io_memBridgePins_M_Cmd        : out std_logic_vector(2 downto 0);
			io_memBridgePins_M_Addr       : out std_logic_vector(31 downto 0);
			io_memBridgePins_M_Data       : out std_logic_vector(31 downto 0);
			io_memBridgePins_M_DataValid  : out std_logic;
			io_memBridgePins_M_DataByteEn : out std_logic_vector(3 downto 0);
			io_memBridgePins_S_Resp       : in  std_logic_vector(1 downto 0);
			io_memBridgePins_S_Data       : in  std_logic_vector(31 downto 0);
			io_memBridgePins_S_CmdAccept  : in  std_logic;
			io_memBridgePins_S_DataAccept : in  std_logic;

			io_uartPins_tx                : out std_logic;
			io_uartPins_rx                : in  std_logic;

			io_nexys4DDRIOPins_MCmd       : out std_logic_vector(2 downto 0);
			io_nexys4DDRIOPins_MAddr      : out std_logic_vector(15 downto 0);
			io_nexys4DDRIOPins_MData      : out std_logic_vector(31 downto 0);
			io_nexys4DDRIOPins_MByteEn    : out std_logic_vector(3 downto 0);
			io_nexys4DDRIOPins_SResp      : in  std_logic_vector(1 downto 0);
			io_nexys4DDRIOPins_SData      : in  std_logic_vector(31 downto 0);
		
			io_bRamCtrlPins_MCmd    : out std_logic_vector(2 downto 0);
            io_bRamCtrlPins_MAddr   : out std_logic_vector(15 downto 0);
            io_bRamCtrlPins_MData   : out std_logic_vector(31 downto 0);
            io_bRamCtrlPins_MByteEn : out std_logic_vector(3 downto 0);
            io_bRamCtrlPins_SResp   : in  std_logic_vector(1 downto 0);
            io_bRamCtrlPins_SData   : in  std_logic_vector(31 downto 0);

            io_icapCtrlPins_MCmd    : out std_logic_vector(2 downto 0);
            io_icapCtrlPins_MAddr   : out std_logic_vector(15 downto 0);
            io_icapCtrlPins_MData   : out std_logic_vector(31 downto 0);
            io_icapCtrlPins_MByteEn : out std_logic_vector(3 downto 0);
            io_icapCtrlPins_SResp   : in  std_logic_vector(1 downto 0);
            io_icapCtrlPins_SData   : in  std_logic_vector(31 downto 0);
            
            io_fpuCtrlPins_MCmd        : out std_logic_vector(2 downto 0); 
            io_fpuCtrlPins_MAddr        :    out std_logic_vector(15 downto 0);
            io_fpuCtrlPins_MData        :    out std_logic_vector(31 downto 0);
            io_fpuCtrlPins_MByteEn    :    out std_logic_vector(3 downto 0);
            io_fpuCtrlPins_SResp        : in  std_logic_vector(1 downto 0);
            io_fpuCtrlPins_SData        : in  std_logic_vector(31 downto 0)
			
			
		);
	end component;

	component nexys4ddr_io is
		port(
			clk                  : in  std_logic;
			clk_pwm              : in  std_logic;
			reset                : in  std_logic;

			MCmd                 : in  std_logic_vector(2 downto 0);
			MAddr                : in  std_logic_vector(15 downto 0);
			MData                : in  std_logic_vector(31 downto 0);
			MByteEn              : in  std_logic_vector(3 downto 0);
			SResp                : out std_logic_vector(1 downto 0);
			SData                : out std_logic_vector(31 downto 0);

			green_leds           : out std_logic_vector(15 downto 0); -- (15) -> LD15 ... LD0 <- (0)
			rgb_leds             : out std_logic_vector(5 downto 0); -- (5) -> LD17_R LD17_G LD17_B | LD16_R LD16_G LD16_B <- (0)
			seven_segments       : out std_logic_vector(7 downto 0); -- (7) -> DP CG CF CE CD CC CB CA <- (0)
			seven_segments_drive : out std_logic_vector(7 downto 0); -- (7) -> AN7 ... AN0 <- (0)
			buttons              : in  std_logic_vector(4 downto 0); -- (4) -> BTNT BTNR BTND BTNL BTNC <- (0)
			switches             : in  std_logic_vector(15 downto 0)); -- (15) -> SW15 ... SW0 <- (0)
	end component;

	component ddr2_ctrl is
		port(
			ddr2_dq             : inout std_logic_vector(15 downto 0);
			ddr2_dqs_n          : inout std_logic_vector(1 downto 0);
			ddr2_dqs_p          : inout std_logic_vector(1 downto 0);
			ddr2_addr           : out   std_logic_vector(12 downto 0);
			ddr2_ba             : out   std_logic_vector(2 downto 0);
			ddr2_ras_n          : out   std_logic;
			ddr2_cas_n          : out   std_logic;
			ddr2_we_n           : out   std_logic;
			ddr2_ck_p           : out   std_logic_vector(0 to 0);
			ddr2_ck_n           : out   std_logic_vector(0 to 0);
			ddr2_cke            : out   std_logic_vector(0 to 0);
			ddr2_cs_n           : out   std_logic_vector(0 to 0);
			ddr2_dm             : out   std_logic_vector(1 downto 0);
			ddr2_odt            : out   std_logic_vector(0 to 0);

			sys_clk_i           : in    std_logic;
			app_addr            : in    std_logic_vector(26 downto 0);
			app_cmd             : in    std_logic_vector(2 downto 0);
			app_en              : in    std_logic;
			app_wdf_data        : in    std_logic_vector(127 downto 0);
			app_wdf_end         : in    std_logic;
			app_wdf_mask        : in    std_logic_vector(15 downto 0);
			app_wdf_wren        : in    std_logic;
			app_rd_data         : out   std_logic_vector(127 downto 0);
			app_rd_data_end     : out   std_logic;
			app_rd_data_valid   : out   std_logic;
			app_rdy             : out   std_logic;
			app_wdf_rdy         : out   std_logic;
			app_sr_req          : in    std_logic;
			app_ref_req         : in    std_logic;
			app_zq_req          : in    std_logic;
			app_sr_active       : out   std_logic;
			app_ref_ack         : out   std_logic;
			app_zq_ack          : out   std_logic;
			ui_clk              : out   std_logic;
			ui_clk_sync_rst     : out   std_logic;
			init_calib_complete : out   std_logic;
			sys_rst             : in    std_logic
		);
	end component;

	component ocp_burst_to_ddr2_ctrl is
		port(
			-- Common
			clk               : in  std_logic;
			rst               : in  std_logic;

			-- OCPburst IN (slave)
			MCmd              : in  std_logic_vector(2 downto 0);
			MAddr             : in  std_logic_vector(31 downto 0);
			MData             : in  std_logic_vector(31 downto 0);
			MDataValid        : in  std_logic;
			MDataByteEn       : in  std_logic_vector(3 downto 0);
			SResp             : out std_logic_vector(1 downto 0);
			SData             : out std_logic_vector(31 downto 0);
			SCmdAccept        : out std_logic;
			SDataAccept       : out std_logic;

			-- Xilinx interface
			app_addr          : out std_logic_vector(26 downto 0);
			app_cmd           : out std_logic_vector(2 downto 0);
			app_en            : out std_logic;
			app_wdf_data      : out std_logic_vector(127 downto 0);
			app_wdf_end       : out std_logic;
			app_wdf_mask      : out std_logic_vector(15 downto 0);
			app_wdf_wren      : out std_logic;
			app_rd_data       : in  std_logic_vector(127 downto 0);
			app_rd_data_end   : in  std_logic;
			app_rd_data_valid : in  std_logic;
			app_rdy           : in  std_logic;
			app_wdf_rdy       : in  std_logic
		);
	end component;

	component clk_manager is
		port(
			clk_in    : in  std_logic;
			clk_out_1 : out std_logic;
			clk_out_2 : out std_logic;
			locked    : out std_logic
		);
	end component;
	
component recon_buffer is
        generic(
            OCP_ADDR_WIDTH  : natural;      -- must be 16 (the 2 LSB are not used) the MSB is always the bank_select enable bit
            BRAM_ADDR_WIDTH : natural;      -- this detemines the size of each bank (must be < or = than OCP_ADDR_WIDTH-1)
            BANK_ADDR_WIDTH : natural       -- this detemines the number of banks
        );
        port(
            clk         : in  std_logic;
            rst         : in  std_logic;
    
            -- OCP interface (slave) for Patmos
            MCmd        : in  std_logic_vector(2 downto 0);
            MAddr       : in  std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
            MData       : in  std_logic_vector(31 downto 0);
            MByteEn     : in  std_logic_vector(3 downto 0);
            SResp       : out std_logic_vector(1 downto 0);
            SData       : out std_logic_vector(31 downto 0);
    
            -- Bram interface for ICAP controller 
            bram_addr   : in  std_logic_vector((BANK_ADDR_WIDTH + BRAM_ADDR_WIDTH - 1) downto 0);
            bram_data_o : out std_logic_vector(31 downto 0);
            bram_we     : in  std_logic_vector(3 downto 0);
            bram_data_i : in  std_logic_vector(31 downto 0)
        );
    end component;

component icap_ctrl is
	port(
		clk   : in  std_logic;
		reset      : in  std_logic;

		-- DMA Configuration Port - OCP
		config_m   : in  ocp_core_m;
		config_s   : out ocp_core_s;

		-- Bram interface for the BRAM buffer 
		ram_addr   : out std_logic_vector(RAM_ADDR_WIDTH - 1 downto 0);
		ram_data_i : in  std_logic_vector(31 downto 0);
		ram_re     : out std_logic;

		-- ICAP interface, the signals of this interface, despite their direction, have the name of the signals of the FPGA interface
		icap_BUSY  : in  std_logic;
		icap_O     : in  std_logic_vector(31 downto 0); -- 32-bit data output
		icap_CE    : out std_logic;     -- Clock enable input
		icap_CLK   : out std_logic;     -- Clock input
		icap_I     : out std_logic_vector(31 downto 0); -- 32-bit data input
		icap_WRITE : out std_logic      -- Write input
	);
end component;

component ocp_rw_reg is
	port(
		clk        : in  std_logic;
		rst        : in  std_logic;

		-- OCP IN (slave)
		MCmd       : in  std_logic_vector(2 downto 0);
		MAddr      : in  std_logic_vector((16 - 1) downto 0);
		MData      : in  std_logic_vector((32 - 1) downto 0);
		MByteEn    : in  std_logic_vector(3 downto 0);
		SResp      : out std_logic_vector(1 downto 0);
		SData      : out std_logic_vector((32 - 1) downto 0)
	);
end component;


	signal clk_int : std_logic;
	signal clk_200 : std_logic;
	signal clk_pwm : std_logic;

	-- for generation of internal reset
	signal reset_int, reset_ddr         : std_logic;
	signal res_reg1, res_reg2, res_reg3, res_reg4 : std_logic;
	signal locked                                 : std_logic;
	signal cpu_reset_btn_debounced, cpu_reset_btn_prev, cpu_reset_btn_sync_int, cpu_reset_btn_sync : std_logic;
    signal debounce_count  : unsigned(12 downto 0);
    constant DEBOUNCE_TIME : integer := 8000;


	signal nexys4DDRIO_MCmd    : std_logic_vector(2 downto 0);
	signal nexys4DDRIO_MAddr   : std_logic_vector(15 downto 0);
	signal nexys4DDRIO_MData   : std_logic_vector(31 downto 0);
	signal nexys4DDRIO_MByteEn : std_logic_vector(3 downto 0);
	signal nexys4DDRIO_SResp   : std_logic_vector(1 downto 0);
	signal nexys4DDRIO_SData   : std_logic_vector(31 downto 0);

	-- signals for the bridge
	signal MCmd_bridge        : std_logic_vector(2 downto 0);
	signal MAddr_bridge       : std_logic_vector(31 downto 0);
	signal MData_bridge       : std_logic_vector(31 downto 0);
	signal MDataValid_bridge  : std_logic;
	signal MDataByteEn_bridge : std_logic_vector(3 downto 0);
	signal SResp_bridge       : std_logic_vector(1 downto 0);
	signal SData_bridge       : std_logic_vector(31 downto 0);
	signal SCmdAccept_bridge  : std_logic;
	signal SDataAccept_bridge : std_logic;

	signal app_addr_bridge          : std_logic_vector(26 downto 0); --
	signal app_cmd_bridge           : std_logic_vector(2 downto 0); --
	signal app_en_bridge            : std_logic;
	signal app_wdf_data_bridge      : std_logic_vector(127 downto 0);
	signal app_wdf_end_bridge       : std_logic;
	signal app_wdf_mask_bridge      : std_logic_vector(15 downto 0);
	signal app_wdf_wren_bridge      : std_logic;
	signal app_rd_data_bridge       : std_logic_vector(127 downto 0); --
	signal app_rd_data_end_bridge   : std_logic; --
	signal app_rd_data_valid_bridge : std_logic;
	signal app_rdy_bridge           : std_logic;
	signal app_wdf_rdy_bridge       : std_logic;

--  attribute mark_debug : string;
--  attribute mark_debug of app_addr_bridge             : signal is "true";
--  attribute mark_debug of app_cmd_bridge              : signal is "true";
--  attribute mark_debug of app_en_bridge               : signal is "true";
--  attribute mark_debug of app_wdf_data_bridge         : signal is "true";
--  attribute mark_debug of app_wdf_end_bridge          : signal is "true";
--  attribute mark_debug of app_wdf_mask_bridge         : signal is "true";
--  attribute mark_debug of app_wdf_wren_bridge         : signal is "true";
--  attribute mark_debug of app_rd_data_bridge          : signal is "true";
--  attribute mark_debug of app_rd_data_end_bridge      : signal is "true";
--  attribute mark_debug of app_rd_data_valid_bridge    : signal is "true";
--  attribute mark_debug of app_rdy_bridge              : signal is "true";
--  attribute mark_debug of app_wdf_rdy_bridge          : signal is "true";

signal bRam_MCmd    : std_logic_vector(2 downto 0);
	signal bRam_MAddr   : std_logic_vector(15 downto 0);
	signal bRam_MData   : std_logic_vector(31 downto 0);
	signal bRam_MByteEn : std_logic_vector(3 downto 0);
	signal bRam_SResp   : std_logic_vector(1 downto 0);
	signal bRam_SData   : std_logic_vector(31 downto 0);

	signal icapCtrl_MCmd    : std_logic_vector(2 downto 0);
    signal icapCtrl_MAddr   : std_logic_vector(15 downto 0);
    signal icapCtrl_MData   : std_logic_vector(31 downto 0);
    signal icapCtrl_MByteEn : std_logic_vector(3 downto 0);
    signal icapCtrl_SResp   : std_logic_vector(1 downto 0);
    signal icapCtrl_SData   : std_logic_vector(31 downto 0);
	
	signal bram_addr_int : std_logic_vector(17 downto 0);
	signal bram_data_o_int :  std_logic_vector(31 downto 0);

	signal icap_BUSY_int : std_logic;
	signal icap_O_int : std_logic_vector(31 downto 0); -- 32-bit data output
	signal icap_CE_int : std_logic;     -- Clock enable input
	signal icap_CLK_int : std_logic;     -- Clock input
	signal icap_I_int : std_logic_vector(31 downto 0); -- 32-bit data input
	signal icap_WRITE_int : std_logic;      -- Write input

	signal fpuCtrlPins_MCmd			: std_logic_vector(2 downto 0); 
	signal fpuCtrlPins_MAddr		:	std_logic_vector(15 downto 0);
	signal fpuCtrlPins_MData		:	std_logic_vector(31 downto 0);
	signal fpuCtrlPins_MByteEn	:	std_logic_vector(3 downto 0);
	signal fpuCtrlPins_SResp		: std_logic_vector(1 downto 0);
	signal fpuCtrlPins_SData		: std_logic_vector(31 downto 0);

begin
	clk_manager_inst_0 : clk_manager port map(
			clk_in    => clk_in,
			clk_out_1 => clk_200,
			clk_out_2 => clk_pwm,
			locked    => locked
		);

    --  reset button debouncer
	rst_btn_debouncer_PROC : process(clk_200)
	begin
		if rising_edge(clk_200) then
			if locked = '0' then
				debounce_count          <= (others => '0');
				cpu_reset_btn_debounced <= '0';
				cpu_reset_btn_prev      <= '0';
				cpu_reset_btn_sync      <= '0';
				cpu_reset_btn_sync_int  <= '0';
			else
				cpu_reset_btn_sync     <= cpu_reset_btn_sync_int;
				cpu_reset_btn_sync_int <= cpu_reset_btn;
				if (cpu_reset_btn_sync = cpu_reset_btn_prev) then
					if (debounce_count = DEBOUNCE_TIME) then
						cpu_reset_btn_debounced <= cpu_reset_btn_prev;
					else
						debounce_count <= debounce_count + 1;
					end if;
				else
					debounce_count     <= (others => '0');
					cpu_reset_btn_prev <= cpu_reset_btn_sync;
				end if;
			end if;
		end if;
	end process;
	
	--  internal reset generation
	process(clk_200)
	begin
		if rising_edge(clk_200) then
			res_reg1 <= locked and cpu_reset_btn_debounced;
			res_reg2 <= res_reg1;
			reset_ddr  <= res_reg2;
		end if;
	end process;

	--  internal reset generation
--	process(clk_int)
--	begin
--		if rising_edge(clk_int) then
--			res_reg3  <= ddr_rst;--int_res;
--			res_reg4  <= res_reg3;
--			int_res_n <= res_reg4;  --reset active high (when 0 patmos is running)
		--int_res <= res_reg2;
--		end if;
--	end process;

	ocp_burst_to_ddr2_ctrl_inst_0 : ocp_burst_to_ddr2_ctrl port map(
			clk               => clk_int,
			rst               => reset_int, -- --            : in std_logic; -- (=1 is reset)

			-- OCPburst IN (slave)
			MCmd              => MCmd_bridge, --              : in  std_logic_vector(2 downto 0);
			MAddr             => MAddr_bridge, --             : in  std_logic_vector(31 downto 0);
			MData             => MData_bridge, --             : in  std_logic_vector(31 downto 0);
			MDataValid        => MDataValid_bridge, --        : in  std_logic;
			MDataByteEn       => MDataByteEn_bridge, --       : in  std_logic_vector(3 downto 0);
			SResp             => SResp_bridge, --             : out std_logic_vector(1 downto 0);
			SData             => SData_bridge, --             : out std_logic_vector(31 downto 0);
			SCmdAccept        => SCmdAccept_bridge, --        : out std_logic;
			SDataAccept       => SDataAccept_bridge, --       : out std_logic;

			-- Xilinx interface
			app_addr          => app_addr_bridge, --             : out    std_logic_vector(26 downto 0); --
			app_cmd           => app_cmd_bridge, --              : out    std_logic_vector(2 downto 0); --
			app_en            => app_en_bridge, --               : out    std_logic;
			app_wdf_data      => app_wdf_data_bridge, --         : out    std_logic_vector(127 downto 0);
			app_wdf_end       => app_wdf_end_bridge, --          : out    std_logic;
			app_wdf_mask      => app_wdf_mask_bridge, --         : out    std_logic_vector(15 downto 0);
			app_wdf_wren      => app_wdf_wren_bridge, --         : out    std_logic;
			app_rd_data       => app_rd_data_bridge, --          : in   std_logic_vector(127 downto 0);--
			app_rd_data_end   => app_rd_data_end_bridge, --      : in   std_logic;--
			app_rd_data_valid => app_rd_data_valid_bridge, --    : in   std_logic;
			app_rdy           => app_rdy_bridge, --              : in   std_logic;
			app_wdf_rdy       => app_wdf_rdy_bridge --         : in   std_logic
		);

	ddr2_ctrl_inst_0 : ddr2_ctrl port map(
			ddr2_dq             => ddr2_dq, --: inout std_logic_vector ( 15 downto 0 );
			ddr2_dqs_n          => ddr2_dqs_n, --: inout std_logic_vector ( 1 downto 0 );
			ddr2_dqs_p          => ddr2_dqs_p, --: inout std_logic_vector ( 1 downto 0 );
			ddr2_addr           => ddr2_addr, --: out std_logic_vector ( 12 downto 0 );
			ddr2_ba             => ddr2_ba, --: out std_logic_vector ( 2 downto 0 );
			ddr2_ras_n          => ddr2_ras_n, --: out std_logic;
			ddr2_cas_n          => ddr2_cas_n, --: out std_logic;
			ddr2_we_n           => ddr2_we_n, --: out std_logic;
			ddr2_ck_p           => ddr2_ck_p, --: out std_logic_vector ( 0 to 0 );
			ddr2_ck_n           => ddr2_ck_n, --: out std_logic_vector ( 0 to 0 );
			ddr2_cke            => ddr2_cke, --: out std_logic_vector ( 0 to 0 );
			ddr2_cs_n           => ddr2_cs_n, --: out std_logic_vector ( 0 to 0 );
			ddr2_dm             => ddr2_dm, --: out std_logic_vector ( 1 downto 0 );
			ddr2_odt            => ddr2_odt, --: out std_logic_vector ( 0 to 0 );
			sys_clk_i           => clk_200, --: in std_logic;
			app_addr            => app_addr_bridge, -- : in std_logic_vector ( 26 downto 0 );
			app_cmd             => app_cmd_bridge, -- : in std_logic_vector ( 2 downto 0 );
			app_en              => app_en_bridge, -- : in std_logic;
			app_wdf_data        => app_wdf_data_bridge, -- : in std_logic_vector ( 127 downto 0 );
			app_wdf_end         => app_wdf_end_bridge, -- : in std_logic;
			app_wdf_mask        => app_wdf_mask_bridge, -- : in std_logic_vector ( 15 downto 0 );
			app_wdf_wren        => app_wdf_wren_bridge, -- : in std_logic;
			app_rd_data         => app_rd_data_bridge, -- : out std_logic_vector ( 127 downto 0 );
			app_rd_data_end     => app_rd_data_end_bridge, -- : out std_logic;
			app_rd_data_valid   => app_rd_data_valid_bridge, -- : out std_logic;
			app_rdy             => app_rdy_bridge, -- : out std_logic;
			app_wdf_rdy         => app_wdf_rdy_bridge, -- : out std_logic;
			app_sr_req          => '0', -- : in std_logic;
			app_ref_req         => '0', -- : in std_logic;
			app_zq_req          => '0', -- : in std_logic;
			app_sr_active       => open, -- : out std_logic;
			app_ref_ack         => open, -- : out std_logic;
			app_zq_ack          => open, -- : out std_logic;
			ui_clk              => clk_int, -- : out std_logic;
			ui_clk_sync_rst     => reset_int, -- : out std_logic;
			init_calib_complete => open, -- : out std_logic;
			sys_rst             => reset_ddr -- : in std_logic
		);

	-- The instance of the patmos processor            
	patmos_inst_0 : Patmos port map(
			clk                           => clk_int,
			reset                         => reset_int,
			io_comConf_M_Cmd              => open,
			io_comConf_M_Addr             => open,
			io_comConf_M_Data             => open,
			io_comConf_M_ByteEn           => open,
			io_comConf_M_RespAccept       => open,
			io_comConf_S_Resp             => (others => '0'),
			io_comConf_S_Data             => (others => '0'),
			io_comConf_S_CmdAccept        => '0',
			io_comSpm_M_Cmd               => open,
			io_comSpm_M_Addr              => open,
			io_comSpm_M_Data              => open,
			io_comSpm_M_ByteEn            => open,
			io_comSpm_S_Resp              => (others => '0'),
			io_comSpm_S_Data              => (others => '0'),
			io_memBridgePins_M_Cmd        => MCmd_bridge,
			io_memBridgePins_M_Addr       => MAddr_bridge,
			io_memBridgePins_M_Data       => MData_bridge,
			io_memBridgePins_M_DataValid  => MDataValid_bridge,
			io_memBridgePins_M_DataByteEn => MDataByteEn_bridge,
			io_memBridgePins_S_Resp       => SResp_bridge,
			io_memBridgePins_S_Data       => SData_bridge,
			io_memBridgePins_S_CmdAccept  => SCmdAccept_bridge,
			io_memBridgePins_S_DataAccept => SDataAccept_bridge,

			
			-- TXD, RXD naming uses terminal-centric naming convention
			io_uartPins_tx                => uart_rxd,
			io_uartPins_rx                => uart_txd,

			io_nexys4DDRIOPins_MCmd       => nexys4DDRIO_MCmd,
			io_nexys4DDRIOPins_MAddr      => nexys4DDRIO_MAddr,
			io_nexys4DDRIOPins_MData      => nexys4DDRIO_MData,
			io_nexys4DDRIOPins_MByteEn    => nexys4DDRIO_MByteEn,
			io_nexys4DDRIOPins_SResp      => nexys4DDRIO_SResp,
			io_nexys4DDRIOPins_SData      => nexys4DDRIO_SData,
			
			io_bRamCtrlPins_MCmd     => bRam_MCmd,
            io_bRamCtrlPins_MAddr    => bRam_MAddr,
            io_bRamCtrlPins_MData    => bRam_MData,
            io_bRamCtrlPins_MByteEn  => bRam_MByteEn,
            io_bRamCtrlPins_SResp    => bRam_SResp,
            io_bRamCtrlPins_SData    => bRam_SData,
                                      
            io_icapCtrlPins_MCmd     => icapCtrl_MCmd,
            io_icapCtrlPins_MAddr    => icapCtrl_MAddr,
            io_icapCtrlPins_MData    => icapCtrl_MData,
            io_icapCtrlPins_MByteEn  => icapCtrl_MByteEn,
            io_icapCtrlPins_SResp    => icapCtrl_SResp,
            io_icapCtrlPins_SData    => icapCtrl_SData,
                                        
            io_fpuCtrlPins_MCmd      => fpuCtrlPins_MCmd,
            io_fpuCtrlPins_MAddr    =>  fpuCtrlPins_MAddr,
            io_fpuCtrlPins_MData     => fpuCtrlPins_MData,
            io_fpuCtrlPins_MByteEn   => fpuCtrlPins_MByteEn,
            io_fpuCtrlPins_SResp     => fpuCtrlPins_SResp,
            io_fpuCtrlPins_SData     => fpuCtrlPins_SData
		);

ocp_rw_reg_inst_0 : ocp_rw_reg 
	port map(
		clk => clk_int,
		rst => reset_int,

		-- OCP IN (slave)
		MCmd        => fpuCtrlPins_MCmd,   
		MAddr      =>  fpuCtrlPins_MAddr,  
		MData       => fpuCtrlPins_MData,  
		MByteEn     => fpuCtrlPins_MByteEn,
		SResp       => fpuCtrlPins_SResp,  
		SData       => fpuCtrlPins_SData   
	);

	nexys4ddr_io_inst_0 : nexys4ddr_io port map(
			clk                  => clk_int,
			clk_pwm              => clk_pwm,
			reset                => reset_int,

			MCmd                 => nexys4DDRIO_MCmd,
			MAddr                => nexys4DDRIO_MAddr,
			MData                => nexys4DDRIO_MData,
			MByteEn              => nexys4DDRIO_MByteEn,
			SResp                => nexys4DDRIO_SResp,
			SData                => nexys4DDRIO_SData,

			green_leds           => green_leds,
			rgb_leds             => rgb_leds,
			seven_segments       => seven_segments,
			seven_segments_drive => seven_segments_drive,
			buttons              => buttons,
			switches             => switches
		);
		
recon_buffer_comp : recon_buffer
            generic map(
                OCP_ADDR_WIDTH  => 16,       -- must be 16 (the 2 LSB are not used) the MSB is always the bank_select enable bit
                BRAM_ADDR_WIDTH => 15,      -- this detemines the size of each bank (must be < or = than OCP_ADDR_WIDTH-1)
                BANK_ADDR_WIDTH => 3       -- this detemines the number of banks (3 is 8 banks)
            )
            port map(
                clk => clk_int,
                rst => reset_int,
        
                -- OCP interface (slave) for Patmos
                MCmd => bRam_MCmd,
                MAddr => bRam_MAddr,
                MData => bRam_MData,
                MByteEn => bRam_MByteEn,
                SResp => bRam_SResp,
                SData => bRam_SData,
        
                -- Bram interface for ICAP controller 
                bram_addr => bram_addr_int,
                bram_data_o => bram_data_o_int,
                bram_we => "0000",
                bram_data_i => (others => '0')
            );
                
            icap_ctrl_comp : icap_ctrl
            port map(
                clk => clk_int,--  : in  std_logic;
                reset => reset_int,--     : in  std_logic;
        
                -- DMA Configuration Port - OCP
                config_m.MCmd => icapCtrl_MCmd,--  : in  ocp_core_m;
                config_m.MAddr => icapCtrl_MAddr,--  : in  ocp_core_m;
                config_m.MData => icapCtrl_MData,--  : in  ocp_core_m;
                config_m.MByteEn => icapCtrl_MByteEn,--  : in  ocp_core_m;
                config_s.SResp => icapCtrl_SResp,--  : out ocp_core_s;
                config_s.SData => icapCtrl_SData,--  : out ocp_core_s;
        
                -- Bram interface for the BRAM buffer 
                ram_addr => bram_addr_int,--  : out std_logic_vector(RAM_ADDR_WIDTH - 1 downto 0);
                ram_data_i => bram_data_o_int,--: in  std_logic_vector(31 downto 0);
                ram_re => open,--    : out std_logic;
        
                -- ICAP interface, the signals of this interface, despite their direction, have the name of the signals of the FPGA interface
                icap_BUSY => icap_BUSY_int,-- : in  std_logic;
                icap_O => icap_O_int,--  : in  std_logic_vector(31 downto 0); -- 32-bit data output
                icap_CE => icap_CE_int,--   : out std_logic;     -- Clock enable input
                icap_CLK => icap_CLK_int,--  : out std_logic;     -- Clock input
                icap_I => icap_I_int,--    : out std_logic_vector(31 downto 0); -- 32-bit data input
                icap_WRITE => icap_WRITE_int--: out std_logic      -- Write input
            );
        
           -- ICAPE2: Internal Configuration Access Port
           --         Artix-7
           -- Xilinx HDL Language Template, version 2016.4
        
           icap_BUSY_int <= '1';
        
           ICAPE2_inst : ICAPE2
           generic map (
              DEVICE_ID => X"3651093",     -- Specifies the pre-programmed Device ID value to be used for simulation
                                           -- purposes.
              ICAP_WIDTH => "X32",         -- Specifies the input and output data width.
              SIM_CFG_FILE_NAME => "None"  -- Specifies the Raw Bitstream (RBT) file to be parsed by the simulation
                                           -- model.
           )
           port map (
              O => icap_O_int,         -- 32-bit output: Configuration data output bus
              CLK => clk_int,     -- 1-bit input: Clock Input
              CSIB => icap_CE_int,   -- 1-bit input: Active-Low ICAP Enable
              I => icap_I_int,         -- 32-bit input: Configuration data input bus
              RDWRB => icap_WRITE_int  -- 1-bit input: Read/Write Select input
           );
        
           -- End of ICAPE2_inst instantiation

end architecture rtl;

            -- ICAP_VIRTEX6: Internal Configuration Access Port
            --               Virtex-6
            -- Xilinx HDL Language Template, version 14.7
        
--            ICAP_VIRTEX6_inst : ICAP_VIRTEX6
--                generic map(
--                    DEVICE_ID         => X"4244093", -- Specifies the pre-programmed Device ID value
--                    ICAP_WIDTH        => "X32",  -- Specifies the input and output data width to be used with the ICAP_VIRTEX6.
--                    SIM_CFG_FILE_NAME => "NONE" -- Specifies the Raw Bitstream (RBT) file to be parsed by the simulation model
--                )
--                port map(
--                    BUSY  => icap_BUSY_int,              -- 1-bit output: Busy/Ready output
--                    O     => icap_O_int,                 -- 32-bit output: Configuration data output bus
--                    CLK   => '0',--icap_CLK_int,           -- 1-bit input: Clock Input
--                    CSB   => icap_CE_int,               -- 1-bit input: Active-Low ICAP input Enable
--                    I     => icap_I_int,                 -- 32-bit input: Configuration data input bus
--                    RDWRB => icap_WRITE_int              -- 1-bit input: Read/Write Select input
--                );
        
        -- End of ICAP_VIRTEX6_inst instantiation
        

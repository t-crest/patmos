--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--

--
-- VHDL top level for Patmos on the Digilent/Xilinx Genesys 2 board with off-chip memory
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
	port(
		clk_in_p        : in    std_logic;
		clk_in_n        : in    std_logic;

		led             : out   std_logic_vector(7 downto 0);

		--TXD, RXD naming uses terminal-centric naming convention
		uart_txd        : in    std_logic;
		uart_rxd        : out   std_logic;

		--DDR3 pins
		ddr3_dq         : inout std_logic_vector(31 downto 0);
		ddr3_dqs_p      : inout std_logic_vector(3 downto 0);
		ddr3_dqs_n      : inout std_logic_vector(3 downto 0);

		ddr3_addr       : out   std_logic_vector(14 downto 0);
		ddr3_ba         : out   std_logic_vector(2 downto 0);
		ddr3_ras_n      : out   std_logic;
		ddr3_cas_n      : out   std_logic;
		ddr3_we_n       : out   std_logic;
		ddr3_reset_n    : out   std_logic;
		ddr3_ck_p       : out   std_logic_vector(0 downto 0);
		ddr3_ck_n       : out   std_logic_vector(0 downto 0);
		ddr3_cke        : out   std_logic_vector(0 downto 0);
		ddr3_cs_n       : out   std_logic_vector(0 downto 0);
		ddr3_dm         : out   std_logic_vector(3 downto 0);
		ddr3_odt        : out   std_logic_vector(0 downto 0);

		--Audio interface
		audio_adc_sdata : in    std_logic;
		audio_adr       : out   std_logic_vector(1 downto 0);
		audio_bclk      : in    std_logic; --out std_logic;
		audio_dac_sdata : out   std_logic;
		audio_lrclk     : in    std_logic; --out std_logic;
		audio_mclk      : out   std_logic;
		audio_scl       : inout std_logic; --serial data output of i2c bus
		audio_sda       : inout std_logic --serial clock output of i2c bus
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

			io_i2CSubAddrPins_MCmd        : out std_logic_vector(2 downto 0);
			io_i2CSubAddrPins_MAddr       : out std_logic_vector(15 downto 0);
			io_i2CSubAddrPins_MData       : out std_logic_vector(31 downto 0);
			io_i2CSubAddrPins_MByteEn     : out std_logic_vector(3 downto 0);
			io_i2CSubAddrPins_SResp       : in  std_logic_vector(1 downto 0);
			io_i2CSubAddrPins_SData       : in  std_logic_vector(31 downto 0);

			io_audioBufferPins_MCmd        : out std_logic_vector(2 downto 0);
            io_audioBufferPins_MAddr      : out std_logic_vector(15 downto 0);
            io_audioBufferPins_MData       : out std_logic_vector(31 downto 0);
            io_audioBufferPins_MByteEn     : out std_logic_vector(3 downto 0);
            io_audioBufferPins_SResp       : in std_logic_vector(1 downto 0);
            io_audioBufferPins_SData      : in std_logic_vector(31 downto 0);

			io_cpuInfoPins_id             : in  std_logic_vector(31 downto 0);
			io_cpuInfoPins_cnt            : in  std_logic_vector(31 downto 0);
			io_ledsPins_led               : out std_logic_vector(7 downto 0);
			io_uartPins_tx                : out std_logic;
			io_uartPins_rx                : in  std_logic
		);
	end component;

	component ddr3_ctrl is
		port(
			ddr3_dq             : inout std_logic_vector(31 downto 0);
			ddr3_dqs_p          : inout std_logic_vector(3 downto 0);
			ddr3_dqs_n          : inout std_logic_vector(3 downto 0);

			ddr3_addr           : out   std_logic_vector(14 downto 0);
			ddr3_ba             : out   std_logic_vector(2 downto 0);
			ddr3_ras_n          : out   std_logic;
			ddr3_cas_n          : out   std_logic;
			ddr3_we_n           : out   std_logic;
			ddr3_reset_n        : out   std_logic;
			ddr3_ck_p           : out   std_logic_vector(0 downto 0);
			ddr3_ck_n           : out   std_logic_vector(0 downto 0);
			ddr3_cke            : out   std_logic_vector(0 downto 0);
			ddr3_cs_n           : out   std_logic_vector(0 downto 0);
			ddr3_dm             : out   std_logic_vector(3 downto 0);
			ddr3_odt            : out   std_logic_vector(0 downto 0);
			app_addr            : in    std_logic_vector(28 downto 0);
			app_cmd             : in    std_logic_vector(2 downto 0);
			app_en              : in    std_logic;
			app_wdf_data        : in    std_logic_vector(255 downto 0);
			app_wdf_end         : in    std_logic;
			app_wdf_mask        : in    std_logic_vector(31 downto 0);
			app_wdf_wren        : in    std_logic;
			app_rd_data         : out   std_logic_vector(255 downto 0);
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
			device_temp         : out   std_logic_vector(11 downto 0);
			-- System Clock Ports
			sys_clk_i           : in    std_logic;
			--device_temp_o                    : out std_logic_vector(11 downto 0);
			sys_rst             : in    std_logic
		);
	end component;

	component ocp_burst_to_ddr3_ctrl is
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
			app_addr          : out std_logic_vector(28 downto 0); --
			app_cmd           : out std_logic_vector(2 downto 0); --
			app_en            : out std_logic;
			app_wdf_data      : out std_logic_vector(255 downto 0);
			app_wdf_end       : out std_logic;
			app_wdf_mask      : out std_logic_vector(31 downto 0);
			app_wdf_wren      : out std_logic;
			app_rd_data       : in  std_logic_vector(255 downto 0); --
			app_rd_data_end   : in  std_logic; --
			app_rd_data_valid : in  std_logic;
			app_rdy           : in  std_logic;
			app_wdf_rdy       : in  std_logic
		);
	end component;

	component clk_manager is
		port(
			clk_in_p  : in  std_logic;
			clk_in_n  : in  std_logic;
			clk_out_1 : out std_logic;
			clk_out_2 : out std_logic;
			locked    : out std_logic
		);
	end component;

	component ADAU1761_i2c_bridge is
		generic(
			OCP_DATA_WIDTH : integer := 32;
			OCP_ADDR_WIDTH : integer := 16;
			input_clk      : integer := 100_000_000; --input clock speed from user logic in Hz
			bus_clk        : integer := 400_000); --speed the i2c bus (scl) will run at in Hz
		port(
			clk     : in    std_logic;
			reset   : in    std_logic;

			-- OCP IN (slave)
			MCmd    : in    std_logic_vector(2 downto 0);
			MAddr   : in    std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
			MData   : in    std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
			MByteEn : in    std_logic_vector(3 downto 0);
			SResp   : out   std_logic_vector(1 downto 0);
			SData   : out   std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);

			sda     : inout std_logic;  --serial data output of i2c bus
			scl     : inout std_logic   --serial clock output of i2c bus
		);
	end component;

	component I2S_seri_deseri_cdc is
		generic(
			ADC_RESOLUTION : natural := 24;
			DAC_RESOLUTION : natural := 24
		);
		port(clk           : in  STD_LOGIC;
			 reset         : in  STD_LOGIC;

			 adc_valid     : out STD_LOGIC;
			 adc_data_l    : out STD_LOGIC_VECTOR(ADC_RESOLUTION - 1 downto 0);
			 adc_data_r    : out STD_LOGIC_VECTOR(ADC_RESOLUTION - 1 downto 0);

			 dac_accept    : out STD_LOGIC;
			 dac_data_l    : in  STD_LOGIC_VECTOR(DAC_RESOLUTION - 1 downto 0);
			 dac_data_r    : in  STD_LOGIC_VECTOR(DAC_RESOLUTION - 1 downto 0);

			 i2s_bclk      : in  STD_LOGIC;
			 i2s_lrclk     : in  STD_LOGIC;
			 i2s_adc_sdata : in  STD_LOGIC;
			 i2s_dac_sdata : out STD_LOGIC);
	end component;
	
	component audio_buffer is
        generic(
            BUFFER_SIZE_WIDTH : integer := 8; --IN BITS OF ADDRESSING
            BUFFER_DATA_WIDTH : integer := 16;
            OCP_DATA_WIDTH    : integer := 32;
            OCP_ADDR_WIDTH    : integer := 16
        );
        port(
            clk      : in  std_logic;
            reset    : in  std_logic;
    
            --ADC
            --write port
            data_in  : in  std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
            wr_en    : in  std_logic;
    
            --DAC
            --read port
            data_out : out std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
            rd_en    : in  std_logic;
    
            -- OCP IN (slave)
            MCmd     : in  std_logic_vector(2 downto 0);
            MAddr    : in  std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
            MData    : in  std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
            MByteEn  : in  std_logic_vector(3 downto 0);
            SResp    : out std_logic_vector(1 downto 0);
            SData    : out std_logic_vector((OCP_DATA_WIDTH - 1) downto 0)
        );
    end component;
	
	signal clk_int : std_logic;
	signal clk_200 : std_logic;
	signal clk_12  : std_logic;

	-- for generation of internal reset
	signal int_res, int_res_n                     : std_logic;
	signal res_reg1, res_reg2, res_reg3, res_reg4 : std_logic;
	signal locked                                 : std_logic;

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

	signal app_addr_bridge          : std_logic_vector(28 downto 0); --
	signal app_cmd_bridge           : std_logic_vector(2 downto 0); --
	signal app_en_bridge            : std_logic;
	signal app_wdf_data_bridge      : std_logic_vector(255 downto 0);
	signal app_wdf_end_bridge       : std_logic;
	signal app_wdf_mask_bridge      : std_logic_vector(31 downto 0);
	signal app_wdf_wren_bridge      : std_logic;
	signal app_rd_data_bridge       : std_logic_vector(255 downto 0); --
	signal app_rd_data_end_bridge   : std_logic; --
	signal app_rd_data_valid_bridge : std_logic;
	signal app_rdy_bridge           : std_logic;
	signal app_wdf_rdy_bridge       : std_logic;

	--attribute mark_debug : string;
	--attribute mark_debug of app_addr_bridge             : signal is "true"; --   std_logic_vector(28 downto 0); --
	--attribute mark_debug of app_cmd_bridge              : signal is "true"; --   std_logic_vector(2 downto 0); --
	--attribute mark_debug of app_en_bridge               : signal is "true"; --   std_logic;
	--attribute mark_debug of app_wdf_data_bridge         : signal is "true"; --   std_logic_vector(255 downto 0);
	--attribute mark_debug of app_wdf_end_bridge          : signal is "true"; --   std_logic;
	--attribute mark_debug of app_wdf_mask_bridge         : signal is "true"; --   std_logic_vector(31 downto 0);
	--attribute mark_debug of app_wdf_wren_bridge         : signal is "true"; --   std_logic;
	--attribute mark_debug of app_rd_data_bridge          : signal is "true"; --   std_logic_vector(255 downto 0);--
	--attribute mark_debug of app_rd_data_end_bridge      : signal is "true"; --   std_logic;--
	--attribute mark_debug of app_rd_data_valid_bridge    : signal is "true"; --   std_logic;
	--attribute mark_debug of app_rdy_bridge              : signal is "true"; --   std_logic;
	--attribute mark_debug of app_wdf_rdy_bridge          : signal is "true"; --   std_logic;

	signal I2CSubAddr_MCmd_bridge    : std_logic_vector(2 downto 0);
	signal I2CSubAddr_MAddr_bridge   : std_logic_vector(15 downto 0);
	signal I2CSubAddr_MData_bridge   : std_logic_vector(31 downto 0);
	signal I2CSubAddr_MByteEn_bridge : std_logic_vector(3 downto 0);
	signal I2CSubAddr_SResp_bridge   : std_logic_vector(1 downto 0);
	signal I2CSubAddr_SData_bridge   : std_logic_vector(31 downto 0);
	
    signal audioBuffer_MCmd_bridge :  std_logic_vector(2 downto 0);
    signal audioBuffer_MAddr_bridge :  std_logic_vector(15 downto 0);
    signal audioBuffer_MData_bridge :  std_logic_vector(31 downto 0);
    signal audioBuffer_MByteEn_bridge :  std_logic_vector(3 downto 0);
    signal audioBuffer_SResp_bridge :  std_logic_vector(1 downto 0);
    signal audioBuffer_SData_bridge : std_logic_vector(31 downto 0);

	----attribute mark_debug : string;
	--attribute mark_debug of I2CSubAddr_MCmd_bridge : signal is "true"; --: std_logic_vector(2 downto 0);
	--attribute mark_debug of I2CSubAddr_MAddr_bridge : signal is "true"; --: std_logic_vector(15 downto 0);
	--attribute mark_debug of I2CSubAddr_MData_bridge : signal is "true"; --: std_logic_vector(31 downto 0);
	--attribute mark_debug of I2CSubAddr_MByteEn_bridge : signal is "true"; --: std_logic_vector(3 downto 0);
	--attribute mark_debug of I2CSubAddr_SResp_bridge : signal is "true"; --: std_logic_vector(1 downto 0);
	--attribute mark_debug of I2CSubAddr_SData_bridge : signal is "true"; --: std_logic_vector(31 downto 0);

	signal audio_adc_sdata_int : std_logic;
	signal audio_bclk_int      : std_logic;
	signal audio_dac_sdata_int : std_logic;
	signal audio_lrclk_int     : std_logic;

	--attribute mark_debug of audio_adc_sdata_int : signal is "true"; --
	--attribute mark_debug of audio_bclk_int      : signal is "true"; --
	--attribute mark_debug of audio_dac_sdata_int : signal is "true"; --
	--attribute mark_debug of audio_lrclk_int     : signal is "true"; --

	signal adc_valid_int  : STD_LOGIC;
	signal dac_accept_int : STD_LOGIC;
	--    signal data_l_int, dac_data_l_int : STD_LOGIC_VECTOR (31 downto 0);
	--    signal data_r_int, dac_data_r_int : STD_LOGIC_VECTOR (31 downto 0); 

	--signal data_l_int : STD_LOGIC_VECTOR(15 downto 0);
	--signal data_r_int : STD_LOGIC_VECTOR(15 downto 0);

    signal adc_data_int : STD_LOGIC_VECTOR(31 downto 0);
    signal dac_data_int : STD_LOGIC_VECTOR(31 downto 0);
    
--attribute mark_debug of  adc_valid_int : signal is "true"; --: STD_LOGIC;
--attribute mark_debug of  dac_accept_int : signal is "true"; --: STD_LOGIC;
--attribute mark_debug of  data_l_int : signal is "true"; --: STD_LOGIC_VECTOR (31 downto 0);
--attribute mark_debug of  data_r_int : signal is "true"; --: STD_LOGIC_VECTOR (31 downto 0);          

begin

	--TMP Audio interface
	audio_adr  <= "00";                 --       : out std_logic_vector(1 downto 0);
	audio_mclk <= clk_12;               --   : out std_logic;


--------------------------------------------
-- Working audio

--	audio_adc_sdata_int <= audio_adc_sdata;
--	audio_bclk_int      <= audio_bclk;
--	audio_dac_sdata     <= audio_dac_sdata_int;
--	audio_lrclk_int     <= audio_lrclk;



--	I2S_seri_deseri_cdc_inst_0 : I2S_seri_deseri_cdc
--		generic map(
--			ADC_RESOLUTION => 16,       --: natural := 24;
--			DAC_RESOLUTION => 16        --: natural := 24
--		)
--		port map(
--			clk           => clk_int,
--			reset         => int_res_n,
--			adc_valid     => adc_valid_int, -- : out STD_LOGIC;
--			adc_data_l    => data_l_int, -- : out STD_LOGIC_VECTOR (31 downto 0);
--			adc_data_r    => data_r_int, -- : out STD_LOGIC_VECTOR (31 downto 0);

--			dac_accept    => dac_accept_int, -- : out STD_LOGIC;
--			dac_data_l    => data_l_int, -- : in STD_LOGIC_VECTOR (31 downto 0);
--			dac_data_r    => data_r_int, -- : in STD_LOGIC_VECTOR (31 downto 0);

--			i2s_bclk      => audio_bclk_int, -- : in STD_LOGIC;
--			i2s_lrclk     => audio_lrclk_int, -- : in STD_LOGIC;
--			i2s_adc_sdata => audio_adc_sdata_int, -- : in STD_LOGIC;
--			i2s_dac_sdata => audio_dac_sdata_int -- : out STD_LOGIC
--		);
-------------------------------------------------

	--audio_adc_sdata_int <= audio_adc_sdata;
	--audio_bclk_int      <= audio_bclk;
	--audio_dac_sdata     <= audio_dac_sdata_int;
	--audio_lrclk_int     <= audio_lrclk;



	I2S_seri_deseri_cdc_inst_0 : I2S_seri_deseri_cdc
		generic map(
			ADC_RESOLUTION => 16,       --: natural := 24;
			DAC_RESOLUTION => 16        --: natural := 24
		)
		port map(
			clk           => clk_int,
			reset         => int_res_n,
			adc_valid     => adc_valid_int, -- : out STD_LOGIC;
			adc_data_l    => adc_data_int(31 downto 16), -- : out STD_LOGIC_VECTOR (31 downto 0);
			adc_data_r    => adc_data_int(15 downto 0), -- : out STD_LOGIC_VECTOR (31 downto 0);

			dac_accept    => dac_accept_int, -- : out STD_LOGIC;
			dac_data_l    => dac_data_int(31 downto 16), -- : in STD_LOGIC_VECTOR (31 downto 0);
			dac_data_r    => dac_data_int(15 downto 0), -- : in STD_LOGIC_VECTOR (31 downto 0);

			i2s_bclk      => audio_bclk, -- : in STD_LOGIC;
			i2s_lrclk     => audio_lrclk, -- : in STD_LOGIC;
			i2s_adc_sdata => audio_adc_sdata, -- : in STD_LOGIC;
			i2s_dac_sdata => audio_dac_sdata -- : out STD_LOGIC
		);

	audio_buffer_inst_0 : audio_buffer 
        generic map(
            BUFFER_SIZE_WIDTH => 7, --IN BITS OF ADDRESSING -- 128 entries
            BUFFER_DATA_WIDTH => 32,
            OCP_DATA_WIDTH    => 32,
            OCP_ADDR_WIDTH    => 16
        )
        port map(
            clk       => clk_int,  
            reset     => int_res_n,
    
            --ADC
            --write port
            data_in  => adc_data_int,--: in  std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
            wr_en   => adc_valid_int, -- : in  std_logic;
    
            --DAC
            --read port
            data_out => dac_data_int, --: out std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
            rd_en   =>  dac_accept_int, --: in  std_logic;
    
            -- OCP IN (slave)
            MCmd  =>  audioBuffer_MCmd_bridge, --      : in  std_logic_vector(2 downto 0);
            MAddr         => audioBuffer_MAddr_bridge, --    : in  std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
            MData         => audioBuffer_MData_bridge, --    : in  std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
            MByteEn     => audioBuffer_MByteEn_bridge, --   : in  std_logic_vector(3 downto 0);
            SResp         => audioBuffer_SResp_bridge, -- --    : out std_logic_vector(1 downto 0);
            SData         => audioBuffer_SData_bridge -- --    : out std_logic_vector((OCP_DATA_WIDTH - 1) downto 0)
        );


	clk_manager_inst_0 : clk_manager port map(
			clk_in_p  => clk_in_p,
			clk_in_n  => clk_in_n,
			clk_out_1 => clk_200,
			clk_out_2 => clk_12,
			locked    => locked
		);

	--
	--  internal reset generation
	process(clk_200)
	begin
		if rising_edge(clk_200) then
			res_reg1 <= locked;
			res_reg2 <= res_reg1;
			--int_res_n <= not res_reg2; --reset active high (when 0 patmos is running)
			int_res  <= res_reg2;
		end if;
	end process;

	--
	--  internal reset generation
	process(clk_int)
	begin
		if rising_edge(clk_int) then
			res_reg3  <= int_res;
			res_reg4  <= res_reg3;
			int_res_n <= not res_reg4;  --reset active high (when 0 patmos is running)
		--int_res <= res_reg2;
		end if;
	end process;

	ocp_burst_to_ddr3_ctrl_inst_0 : ocp_burst_to_ddr3_ctrl port map(
			clk               => clk_int,
			rst               => int_res_n, -- --            : in std_logic; -- (=1 is reset)

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
			app_addr          => app_addr_bridge, --             : out    std_logic_vector(28 downto 0); --
			app_cmd           => app_cmd_bridge, --              : out    std_logic_vector(2 downto 0); --
			app_en            => app_en_bridge, --               : out    std_logic;
			app_wdf_data      => app_wdf_data_bridge, --         : out    std_logic_vector(255 downto 0);
			app_wdf_end       => app_wdf_end_bridge, --          : out    std_logic;
			app_wdf_mask      => app_wdf_mask_bridge, --         : out    std_logic_vector(31 downto 0);
			app_wdf_wren      => app_wdf_wren_bridge, --         : out    std_logic;
			app_rd_data       => app_rd_data_bridge, --          : in   std_logic_vector(255 downto 0);--
			app_rd_data_end   => app_rd_data_end_bridge, --      : in   std_logic;--
			app_rd_data_valid => app_rd_data_valid_bridge, --    : in   std_logic;
			app_rdy           => app_rdy_bridge, --              : in   std_logic;
			app_wdf_rdy       => app_wdf_rdy_bridge --         : in   std_logic
		);

	ddr3_ctrl_inst_0 : ddr3_ctrl port map(
			ddr3_dq             => ddr3_dq, --: inout std_logic_vector(31 downto 0);
			ddr3_dqs_p          => ddr3_dqs_p, --    --: inout std_logic_vector(3 downto 0);
			ddr3_dqs_n          => ddr3_dqs_n, --    --: inout std_logic_vector(3 downto 0);

			ddr3_addr           => ddr3_addr, --     : out   std_logic_vector(14 downto 0);
			ddr3_ba             => ddr3_ba, --       : out   std_logic_vector(2 downto 0);
			ddr3_ras_n          => ddr3_ras_n, --    : out   std_logic;
			ddr3_cas_n          => ddr3_cas_n, --    : out   std_logic;
			ddr3_we_n           => ddr3_we_n, --     : out   std_logic;
			ddr3_reset_n        => ddr3_reset_n, --  : out   std_logic;
			ddr3_ck_p           => ddr3_ck_p, --     : out   std_logic_vector(0 downto 0);
			ddr3_ck_n           => ddr3_ck_n, --     : out   std_logic_vector(0 downto 0);
			ddr3_cke            => ddr3_cke, --      : out   std_logic_vector(0 downto 0);
			ddr3_cs_n           => ddr3_cs_n, --     : out   std_logic_vector(0 downto 0);
			ddr3_dm             => ddr3_dm, --       : out   std_logic_vector(3 downto 0);
			ddr3_odt            => ddr3_odt, --      : out   std_logic_vector(0 downto 0);

			app_addr            => app_addr_bridge, --                  : in    std_logic_vector(28 downto 0);
			app_cmd             => app_cmd_bridge, --                   : in    std_logic_vector(2 downto 0);
			app_en              => app_en_bridge, --                    : in    std_logic;
			app_wdf_data        => app_wdf_data_bridge, --              : in    std_logic_vector(255 downto 0);
			app_wdf_end         => app_wdf_end_bridge, --               : in    std_logic;
			app_wdf_mask        => app_wdf_mask_bridge, --         : in    std_logic_vector(31 downto 0);
			app_wdf_wren        => app_wdf_wren_bridge, --              : in    std_logic;
			app_rd_data         => app_rd_data_bridge, --              : out   std_logic_vector(255 downto 0);
			app_rd_data_end     => app_rd_data_end_bridge, --           : out   std_logic;
			app_rd_data_valid   => app_rd_data_valid_bridge, --         : out   std_logic;
			app_rdy             => app_rdy_bridge, --                   : out   std_logic;
			app_wdf_rdy         => app_wdf_rdy_bridge, --               : out   std_logic;

			app_sr_req          => '0', --                : in    std_logic;
			app_ref_req         => '0', --               : in    std_logic;
			app_zq_req          => '0', --                : in    std_logic;
			app_sr_active       => open, --             : out   std_logic;
			app_ref_ack         => open, --               : out   std_logic;
			app_zq_ack          => open, --                : out   std_logic;

			ui_clk              => clk_int, --                   : out   std_logic;
			ui_clk_sync_rst     => open, --           : out   std_logic;
			init_calib_complete => open, --       : out   std_logic;
			device_temp         => open,
			-- System Clock Ports
			sys_clk_i           => clk_200, --                      : in    std_logic;
			--device_temp_o => open,    --                    : out std_logic_vector(11 downto 0);
			sys_rst             => int_res -- reset active low                    : in    std_logic
		);

	ADAU1761_i2c_bridge_int_0 : ADAU1761_i2c_bridge
		generic map(
			OCP_DATA_WIDTH => 32,       --: integer := 32;
			OCP_ADDR_WIDTH => 16,       -- : integer := 16;
			input_clk      => 100_000_000, --      : integer := 100_000_000; --input clock speed from user logic in Hz
			bus_clk        => 400_000)  --        : integer := 400_000); --speed the i2c bus (scl) will run at in Hz
		port map(
			clk     => clk_int,         --     : in    std_logic;
			reset   => int_res_n,       --   : in    std_logic;

			-- OCP IN (slave)
			MCmd    => I2CSubAddr_MCmd_bridge, --    : in    std_logic_vector(2 downto 0);
			MAddr   => I2CSubAddr_MAddr_bridge, --   : in    std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
			MData   => I2CSubAddr_MData_bridge, --   : in    std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
			MByteEn => I2CSubAddr_MByteEn_bridge, -- : in    std_logic_vector(3 downto 0);
			SResp   => I2CSubAddr_SResp_bridge, --   : out   std_logic_vector(1 downto 0);
			SData   => I2CSubAddr_SData_bridge, --   : out   std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);

			sda     => audio_sda,       --     : inout std_logic;      --serial data output of i2c bus
			scl     => audio_scl        --     : inout std_logic       --serial clock output of i2c bus
		);

	-- The instance of the patmos processor            
	patmos_inst_0 : Patmos port map(
			clk                           => clk_int,
			reset                         => int_res_n,
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
			io_memBridgePins_M_Cmd        => MCmd_bridge, --: out std_logic_vector(2 downto 0);
			io_memBridgePins_M_Addr       => MAddr_bridge, --: out std_logic_vector(31 downto 0);
			io_memBridgePins_M_Data       => MData_bridge, --: out std_logic_vector(31 downto 0);
			io_memBridgePins_M_DataValid  => MDataValid_bridge, --: out std_logic;
			io_memBridgePins_M_DataByteEn => MDataByteEn_bridge, --: out std_logic_vector(3 downto 0);
			io_memBridgePins_S_Resp       => SResp_bridge, --: in std_logic_vector(1 downto 0);
			io_memBridgePins_S_Data       => SData_bridge, --: in std_logic_vector(31 downto 0);
			io_memBridgePins_S_CmdAccept  => SCmdAccept_bridge, --: in std_logic;
			io_memBridgePins_S_DataAccept => SDataAccept_bridge, --: in std_logic;

			io_i2CSubAddrPins_MCmd        => I2CSubAddr_MCmd_bridge, -- : out std_logic_vector(2 downto 0);
			io_i2CSubAddrPins_MAddr       => I2CSubAddr_MAddr_bridge, -- : out std_logic_vector(15 downto 0);
			io_i2CSubAddrPins_MData       => I2CSubAddr_MData_bridge, -- : out std_logic_vector(31 downto 0);
			io_i2CSubAddrPins_MByteEn     => I2CSubAddr_MByteEn_bridge, -- : out std_logic_vector(3 downto 0);
			io_i2CSubAddrPins_SResp       => I2CSubAddr_SResp_bridge, -- : in std_logic_vector(1 downto 0);
			io_i2CSubAddrPins_SData       => I2CSubAddr_SData_bridge, -- : in std_logic_vector(31 downto 0);
			
			io_audioBufferPins_MCmd        => audioBuffer_MCmd_bridge, -- : out std_logic_vector(2 downto 0);
            io_audioBufferPins_MAddr       => audioBuffer_MAddr_bridge, -- : out std_logic_vector(15 downto 0);
            io_audioBufferPins_MData       => audioBuffer_MData_bridge, -- : out std_logic_vector(31 downto 0);
            io_audioBufferPins_MByteEn     => audioBuffer_MByteEn_bridge, -- : out std_logic_vector(3 downto 0);
            io_audioBufferPins_SResp       => audioBuffer_SResp_bridge, -- : in std_logic_vector(1 downto 0);
            io_audioBufferPins_SData       => audioBuffer_SData_bridge, -- : in std_logic_vector(31 downto 0);

			io_cpuInfoPins_id             => X"00000000",
			io_cpuInfoPins_cnt            => X"00000000",
			io_ledsPins_led               => led,
			io_uartPins_tx                => uart_rxd, --TXD, RXD naming uses terminal-centric naming convention
			io_uartPins_rx                => uart_txd --TXD, RXD naming uses terminal-centric naming convention
		);

end architecture rtl;

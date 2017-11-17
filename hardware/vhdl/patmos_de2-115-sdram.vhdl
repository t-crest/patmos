--
-- Copyright: 2017, Technical University of Denmark, DTU Compute
-- Author: Luca Pezzarossa (lpez@dtu.dk)
-- License: Simplified BSD License
--

--
-- VHDL top level for Patmos using the SDRAM memory
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
	port(
		clk        : in    std_logic;
		led        : out   std_logic_vector(8 downto 0);
		key        : in    std_logic_vector(3 downto 0);
		txd        : out   std_logic;
		rxd        : in    std_logic;

		-- memory interface
		dram_CLK   : out   std_logic;   -- Clock
		dram_CKE   : out   std_logic;   -- Clock Enable
		dram_RAS_n : out   std_logic;   -- Row Address Strobe
		dram_CAS_n : out   std_logic;   -- Column Address Strobe
		dram_WE_n  : out   std_logic;   -- Write Enable
		dram_CS_n  : out   std_logic;   -- Chip Select
		dram_BA_0  : out   std_logic;   -- Bank Address
		dram_BA_1  : out   std_logic;   -- Bank Address
		dram_ADDR  : out   std_logic_vector(12 downto 0); -- SDRAM Address
		-- SDRAM interface lower chip
		dram0_UDQM : out   std_logic;   -- Data mask Upper Byte
		dram0_LDQM : out   std_logic;   -- Data mask Lower Byte
		-- SDRAM interface highier chip
		dram1_UDQM : out   std_logic;   -- Data mask Upper Byte
		dram1_LDQM : out   std_logic;   -- Data mask Lower Byte
		-- data bus from both chips
		dram_DQ    : inout std_logic_vector(31 downto 0) -- Data
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
			io_memBridgePins_M_Addr       : out std_logic_vector(26 downto 0);
			io_memBridgePins_M_Data       : out std_logic_vector(31 downto 0);
			io_memBridgePins_M_DataValid  : out std_logic;
			io_memBridgePins_M_DataByteEn : out std_logic_vector(3 downto 0);
			io_memBridgePins_S_Resp       : in  std_logic_vector(1 downto 0);
			io_memBridgePins_S_Data       : in  std_logic_vector(31 downto 0);
			io_memBridgePins_S_CmdAccept  : in  std_logic;
			io_memBridgePins_S_DataAccept : in  std_logic;

			io_ledsPins_led               : out std_logic_vector(8 downto 0);
			io_keysPins_key               : in  std_logic_vector(3 downto 0);
			io_uartPins_tx                : out std_logic;
			io_uartPins_rx                : in  std_logic
		);
	end component;

	component sc_sdram_top is
		port(
			clk, rst        : in    std_logic;

			pll_locked      : in    std_logic;
			dram_clk_skewed : in    std_logic;

			-- User interface
			M_Cmd           : in    std_logic_vector(2 downto 0);
			M_Addr          : in    std_logic_vector(26 downto 0);
			M_Data          : in    std_logic_vector(31 downto 0);
			M_DataValid     : in    std_logic;
			M_DataByteEn    : in    std_logic_vector(3 downto 0);
			S_Resp          : out   std_logic_vector(1 downto 0);
			S_Data          : out   std_logic_vector(31 downto 0);
			S_CmdAccept     : out   std_logic;
			S_DataAccept    : out   std_logic;

			-- memory interface
			dram_CLK        : out   std_logic; -- Clock
			dram_CKE        : out   std_logic; -- Clock Enable
			dram_RAS_n      : out   std_logic; -- Row Address Strobe
			dram_CAS_n      : out   std_logic; -- Column Address Strobe
			dram_WE_n       : out   std_logic; -- Write Enable
			dram_CS_n       : out   std_logic; -- Chip Select
			dram_BA_0       : out   std_logic; -- Bank Address
			dram_BA_1       : out   std_logic; -- Bank Address
			dram_ADDR       : out   std_logic_vector(12 downto 0); -- SDRAM Address
			-- SDRAM interface lower chip
			dram0_UDQM      : out   std_logic; -- Data mask Upper Byte
			dram0_LDQM      : out   std_logic; -- Data mask Lower Byte
			-- SDRAM interface highier chip
			dram1_UDQM      : out   std_logic; -- Data mask Upper Byte
			dram1_LDQM      : out   std_logic; -- Data mask Lower Byte
			-- data bus from both chips
			dram_DQ         : inout std_logic_vector(31 downto 0) -- Data
		);
	end component;

	component de2_115_sdram_pll is
		PORT(
			inclk0 : IN  STD_LOGIC := '0';
			c0     : OUT STD_LOGIC;
			c1     : OUT STD_LOGIC;
			c2     : OUT STD_LOGIC;
			locked : OUT STD_LOGIC
		);
	END component;

	-- DE2-70: 50 MHz clock => 80 MHz
	-- BeMicro: 16 MHz clock => 25.6 MHz
	--	constant pll_infreq : real    := 50.0;
	--	constant pll_mult   : natural := 1;
	--	constant pll_div    : natural := 1;

	signal sys_clk, dram_clk_int, dram_clk_skew : std_logic;

	-- for generation of internal reset
	signal pll_locked : std_logic;
	signal rst, rst_n : std_logic;
	signal res_cnt    : unsigned(2 downto 0) := "000"; -- for the simulation

	signal MCmd_int        : std_logic_vector(2 downto 0);
	signal MAddr_int       : std_logic_vector(26 downto 0);
	signal MData_int       : std_logic_vector(31 downto 0);
	signal MDataByteEn_int : std_logic_vector(3 downto 0);
	signal MDataValid_int  : std_logic;
	signal SResp_int       : std_logic_vector(1 downto 0);
	signal SData_int       : std_logic_vector(31 downto 0);
	signal SCmdAccept_int  : std_logic;
	signal SDataAccept_int : std_logic;

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
	process(sys_clk, pll_locked)
	begin
		if pll_locked = '0' then
			res_cnt <= "000";
			rst     <= '1';
		elsif rising_edge(sys_clk) then
			if (res_cnt /= "111") then
				res_cnt <= res_cnt + 1;
			end if;
			rst <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
		end if;
	end process;
	--rst_n  <= not rst;

	pll : de2_115_sdram_pll
		port map(
			inclk0 => clk,
			c0     => sys_clk,
			c1     => open,             --dram_clk_int,
			c2     => dram_clk_skew,
			locked => pll_locked);

	dram_clk_int <= sys_clk;

	comp : Patmos port map(
			clk                           => sys_clk, --             : in  std_logic;
			reset                         => rst, --           : in  std_logic;

			io_comConf_M_Cmd              => open, --        : out std_logic_vector(2 downto 0);
			io_comConf_M_Addr             => open, --       : out std_logic_vector(31 downto 0);
			io_comConf_M_Data             => open, --       : out std_logic_vector(31 downto 0);
			io_comConf_M_ByteEn           => open, --     : out std_logic_vector(3 downto 0);
			io_comConf_M_RespAccept       => open, -- : out std_logic;
			io_comConf_S_Resp             => (others => '0'), --       : in std_logic_vector(1 downto 0);
			io_comConf_S_Data             => (others => '0'), --       : in std_logic_vector(31 downto 0);
			io_comConf_S_CmdAccept        => '0', --  : in std_logic;

			io_comSpm_M_Cmd               => open, --         : out std_logic_vector(2 downto 0);
			io_comSpm_M_Addr              => open, --        : out std_logic_vector(31 downto 0);
			io_comSpm_M_Data              => open, --        : out std_logic_vector(31 downto 0);
			io_comSpm_M_ByteEn            => open, --      : out std_logic_vector(3 downto 0);
			io_comSpm_S_Resp              => (others => '0'), --        : in std_logic_vector(1 downto 0);
			io_comSpm_S_Data              => (others => '0'), --        : in std_logic_vector(31 downto 0);

			io_memBridgePins_M_Cmd        => MCmd_int, --: out  std_logic_vector(2 downto 0); 
			io_memBridgePins_M_Addr       => MAddr_int, -- : out  std_logic_vector(25 downto 0);
			io_memBridgePins_M_Data       => MData_int, -- : out  std_logic_vector(31 downto 0);
			io_memBridgePins_M_DataValid  => MDataValid_int, -- : out std_logic;
			io_memBridgePins_M_DataByteEn => MDataByteEn_int, --: out std_logic_vector(3 downto 0);
			io_memBridgePins_S_Resp       => SResp_int, -- : in  std_logic_vector(1 downto 0);
			io_memBridgePins_S_Data       => SData_int, -- : in  std_logic_vector(31 downto 0);
			io_memBridgePins_S_CmdAccept  => SCmdAccept_int, -- : in std_logic;
			io_memBridgePins_S_DataAccept => SDataAccept_int, -- : in std_logic

			io_ledsPins_led               => led, -- : out std_logic_vector(8 downto 0);
			io_keysPins_key               => key, -- : in std_logic_vector(3 downto 0);
			io_uartPins_tx                => txd, --  : out std_logic;
			io_uartPins_rx                => rxd --  : in  std_logic;

		);

	sc_sdram_top_inst0 : sc_sdram_top port map(
			clk             => dram_clk_int, --
			rst             => rst,     --        : in    std_logic;

			pll_locked      => pll_locked, --      : in    std_logic;
			dram_clk_skewed => dram_clk_skew, -- : in    std_logic;

			-- User interface
			M_Cmd           => MCmd_int, --   : in    SDRAM_controller_master_type;
			M_Addr          => MAddr_int, --
			M_Data          => MData_int, --
			M_DataByteEn    => MDataByteEn_int, --
			M_DataValid     => MDataValid_int,
			S_CmdAccept     => SCmdAccept_int, --    : out   SDRAM_controller_slave_type;
			S_DataAccept    => SDataAccept_int, --
			S_Data          => SData_int, --
			S_Resp          => SResp_int, --

			-- memory interface
			dram_CLK        => dram_CLK, --       : out   std_logic; -- Clock
			dram_CKE        => dram_CKE, --       : out   std_logic; -- Clock Enable
			dram_RAS_n      => dram_RAS_n, --     : out   std_logic; -- Row Address Strobe
			dram_CAS_n      => dram_CAS_n, --     : out   std_logic; -- Column Address Strobe
			dram_WE_n       => dram_WE_n, --      : out   std_logic; -- Write Enable
			dram_CS_n       => dram_CS_n, --      : out   std_logic; -- Chip Select
			dram_BA_0       => dram_BA_0, --      : out   std_logic; -- Bank Address
			dram_BA_1       => dram_BA_1, --      : out   std_logic; -- Bank Address
			dram_ADDR       => dram_ADDR, --      : out   std_logic_vector(12 downto 0); -- SDRAM Address
			-- SDRAM interface lower chip
			dram0_UDQM      => dram0_UDQM, --      : out   std_logic; -- Data mask Upper Byte
			dram0_LDQM      => dram0_LDQM, --      : out   std_logic; -- Data mask Lower Byte
			-- SDRAM interface highier chip
			dram1_UDQM      => dram1_UDQM, --      : out   std_logic; -- Data mask Upper Byte
			dram1_LDQM      => dram1_LDQM, --      : out   std_logic; -- Data mask Lower Byte
			-- data bus from both chips
			dram_DQ         => dram_DQ  --         : inout std_logic_vector(31 downto 0) -- Data
		);

end architecture rtl;

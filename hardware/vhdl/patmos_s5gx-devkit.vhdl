--
-- Copyright: 2014, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
--         Rasmus Bo Soerensen (rasmus@rbscloud.dk)
--         Wolfgang Puffitsch (wpuffitsch@gmail.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Altera Stratix V development board
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
	port(
		clk : in  std_logic;
		oLedsPins_led : out std_logic_vector(8 downto 0);
		--iKeysPins_key : in std_logic_vector(3 downto 0);
		oUartPins_txd : out std_logic;
		iUartPins_rxd : in  std_logic;

        qdrIIplusCtrlPins_clk_n : out std_logic;
        qdrIIplusCtrlPins_clk_p : out std_logic;
        qdrIIplusCtrlPins_clkq_n : in std_logic;
        qdrIIplusCtrlPins_clkq_p : in std_logic;
        qdrIIplusCtrlPins_k_n : out std_logic;
        qdrIIplusCtrlPins_k_p : out std_logic;

        qdrIIplusCtrlPins_addr  : out std_logic_vector(18 downto 0);
        qdrIIplusCtrlPins_nrps  : out std_logic;
        qdrIIplusCtrlPins_nwps  : out std_logic;
        qdrIIplusCtrlPins_nbws  : out std_logic_vector(1 downto 0);
        qdrIIplusCtrlPins_din   : in  std_logic_vector(15 downto 0);
        qdrIIplusCtrlPins_dout  : out std_logic_vector(15 downto 0);
        qdrIIplusCtrlPins_ndoff : out std_logic
	);
end entity patmos_top;

architecture rtl of patmos_top is
	component Patmos is
		port(
			clk             : in  std_logic;
			reset           : in  std_logic;

			io_comConf_M_Cmd        : out std_logic_vector(2 downto 0);
			io_comConf_M_Addr       : out std_logic_vector(31 downto 0);
			io_comConf_M_Data       : out std_logic_vector(31 downto 0);
			io_comConf_M_ByteEn     : out std_logic_vector(3 downto 0);
			io_comConf_M_RespAccept : out std_logic;
			io_comConf_S_Resp       : in std_logic_vector(1 downto 0);
			io_comConf_S_Data       : in std_logic_vector(31 downto 0);
			io_comConf_S_CmdAccept  : in std_logic;

			io_comSpm_M_Cmd         : out std_logic_vector(2 downto 0);
			io_comSpm_M_Addr        : out std_logic_vector(31 downto 0);
			io_comSpm_M_Data        : out std_logic_vector(31 downto 0);
			io_comSpm_M_ByteEn      : out std_logic_vector(3 downto 0);
			io_comSpm_S_Resp        : in std_logic_vector(1 downto 0);
			io_comSpm_S_Data        : in std_logic_vector(31 downto 0);

			io_cpuInfoPins_id   : in  std_logic_vector(31 downto 0);
			io_cpuInfoPins_cnt  : in  std_logic_vector(31 downto 0);
			io_ledsPins_led : out std_logic_vector(8 downto 0);
			--io_keysPins_key : in  std_logic_vector(3 downto 0);
			io_uartPins_tx  : out std_logic;
			io_uartPins_rx  : in  std_logic;

            io_qdrIIplusCtrlPins_addr   : out std_logic_vector(18 downto 0);
            io_qdrIIplusCtrlPins_nrps   : out std_logic;
            io_qdrIIplusCtrlPins_nwps   : out std_logic;
            io_qdrIIplusCtrlPins_nbws_0 : out std_logic_vector(1 downto 0);
            io_qdrIIplusCtrlPins_nbws_1 : out std_logic_vector(1 downto 0);
            io_qdrIIplusCtrlPins_din_0  : in  std_logic_vector(15 downto 0);
            io_qdrIIplusCtrlPins_din_1  : in  std_logic_vector(15 downto 0);
            io_qdrIIplusCtrlPins_dout_0 : out std_logic_vector(15 downto 0);
            io_qdrIIplusCtrlPins_dout_1 : out std_logic_vector(15 downto 0);
            io_qdrIIplusCtrlPins_ndoff  : out std_logic
		);
	end component;

	-- S5GX: 50 MHz clock => 166 MHz
	constant pll_infreq : real    := 50.0;
	constant pll_mult   : natural := 10;
	constant pll_div    : natural := 3;

	signal clk_int, nclk_int : std_logic;
	signal clk_ram, nclk_ram : std_logic;
	signal pll_locked : std_logic;

	-- for generation of internal reset
	signal int_res            : std_logic;
	signal res_reg1, res_reg2 : std_logic;
	signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    signal qdrIIplusCtrlPins_addr_int : std_logic_vector(18 downto 0);
    signal qdrIIplusCtrlPins_nrps_int : std_logic;
    signal qdrIIplusCtrlPins_nwps_int : std_logic;
    signal qdrIIplusCtrlPins_ndoff_int : std_logic;
    -- dual data-rate signals for QDR memory
    signal qdrIIplusCtrlPins_nbws_0    : std_logic_vector(1 downto 0);
    signal qdrIIplusCtrlPins_nbws_1    : std_logic_vector(1 downto 0);
    signal qdrIIplusCtrlPins_din_0     : std_logic_vector(15 downto 0);
    signal qdrIIplusCtrlPins_din_1     : std_logic_vector(15 downto 0);
    signal qdrIIplusCtrlPins_dout_0    : std_logic_vector(15 downto 0);
    signal qdrIIplusCtrlPins_dout_1    : std_logic_vector(15 downto 0);

	attribute altera_attribute : string;
	attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin

	pll_inst : entity work.pll generic map(
			input_freq  => pll_infreq,
			multiply_by => pll_mult,
			divide_by   => pll_div
		)
		port map(
			inclk0 => clk,
			c0     => clk_int,
            c1     => nclk_int,
            c2     => clk_ram,
            c3     => nclk_ram,
            locked => pll_locked
		);
	-- we use a PLL
	-- clk_int <= clk;

	--
	--	internal reset generation
	--	should include the PLL lock signal
	--
	process(pll_locked, clk_int)
	begin
        if pll_locked = '0' then
            res_cnt <= "000";
            res_reg1 <= '1';
            res_reg2 <= '1';
        elsif rising_edge(clk_int) then
			if (res_cnt /= "111") then
				res_cnt <= res_cnt + 1;
			end if;
			res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
			res_reg2 <= res_reg1;
			int_res  <= res_reg2;
		end if;
	end process;

    -- QDR clocks
    qdrIIplusCtrlPins_clk_n <= nclk_ram;
    qdrIIplusCtrlPins_clk_p <= clk_ram;
    qdrIIplusCtrlPins_k_n <= nclk_ram;
    qdrIIplusCtrlPins_k_p <= clk_ram;

    -- register QDR signals to match dual data rate signals
    qdr_buf: process (clk_int)
    begin
        if rising_edge(clk_int) then
            qdrIIplusCtrlPins_addr <= qdrIIplusCtrlPins_addr_int;
            qdrIIplusCtrlPins_nrps <= qdrIIplusCtrlPins_nrps_int;
            qdrIIplusCtrlPins_nwps <= qdrIIplusCtrlPins_nwps_int;
            qdrIIplusCtrlPins_ndoff <= qdrIIplusCtrlPins_ndoff_int;
        end if;
    end process qdr_buf;
    
    -- dual data rate I/O
    ddio_nbws : entity work.ddio_out generic map (
        width => 2)
        port map (
            datain_h => qdrIIplusCtrlPins_nbws_0,
            datain_l => qdrIIplusCtrlPins_nbws_1,
            outclock => clk_int,
            dataout  => qdrIIplusCtrlPins_nbws);
    ddio_dout : entity work.ddio_out generic map (
        width => 16)
        port map (
            datain_h => qdrIIplusCtrlPins_dout_0,
            datain_l => qdrIIplusCtrlPins_dout_1,
            outclock => clk_int,
            dataout  => qdrIIplusCtrlPins_dout);
    ddio_din : entity work.ddio_in generic map (
        width => 16)
        port map (
            datain => qdrIIplusCtrlPins_din,
            inclock => nclk_int,
            dataout_h  => qdrIIplusCtrlPins_din_1,
            dataout_l  => qdrIIplusCtrlPins_din_0);

    comp : Patmos port map(clk_int, int_res,
           open, open, open, open, open,
           (others => '0'), (others => '0'), '0',
           open, open, open, open,
           (others => '0'), (others => '0'),
           X"00000000", X"00000001",
           oLedsPins_led,
           --iKeysPins_key,
           oUartPins_txd, iUartPins_rxd,
           qdrIIplusCtrlPins_addr_int,
           qdrIIplusCtrlPins_nrps_int,
           qdrIIplusCtrlPins_nwps_int,
           qdrIIplusCtrlPins_nbws_0, qdrIIplusCtrlPins_nbws_1,
           qdrIIplusCtrlPins_din_0, qdrIIplusCtrlPins_din_1,
           qdrIIplusCtrlPins_dout_0, qdrIIplusCtrlPins_dout_1,
           qdrIIplusCtrlPins_ndoff_int);
            
end architecture rtl;

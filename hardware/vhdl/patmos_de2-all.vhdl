--
-- Copyright: 2016, Technical University of Denmark, DTU Compute
-- Authors: Martin Schoeberl (martin@jopdesign.com)
--          Rasmus Bo Soerensen (rasmus@rbscloud.dk)
--          Luca Pezzarossa (lpez@dtu.dk)
--          Wolfgang Puffitsch (wpuffitsch@gmail.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Altera de2-115 board with the EthMac ethernet controller
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use work.all;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
        clk           : in    std_logic;
        oEthPPS          : out   std_logic;
        oEth2PPS          : out   std_logic;
        oLedsPins_led : out   std_logic_vector(8 downto 0);
        oLedsPins_ledR : out  std_logic_vector(17 downto 0);
        iKeysPins_key : in    std_logic_vector(3 downto 0);
        oGpioPins_gpio_0 : out std_logic_vector(6 downto 0);
        oSegmentDisplayPins_hexDisp_7 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_6 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_5 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_4 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_3 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_2 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_1 : out std_logic_vector(6 downto 0);
		oSegmentDisplayPins_hexDisp_0 : out std_logic_vector(6 downto 0);
        oUartPins_txd : out   std_logic;
        iUartPins_rxd : in    std_logic;
        oUart2Pins_txd : out   std_logic;
        iUart2Pins_rxd : in    std_logic;
        oUart3Pins_txd : out   std_logic;
        iUart3Pins_rxd : in    std_logic;
        oSRAM_A       : out   std_logic_vector(19 downto 0);
        SRAM_DQ       : inout std_logic_vector(15 downto 0);
        oSRAM_CE_N    : out   std_logic;
        oSRAM_OE_N    : out   std_logic;
        oSRAM_WE_N    : out   std_logic;
        oSRAM_LB_N    : out   std_logic;
        oSRAM_UB_N    : out   std_logic;

        --first PHY interface
        -- Tx
        ENET0_TX_CLK  : in    std_logic; -- Transmit clock (from PHY)
        ENET0_TX_DATA : out   std_logic_vector(3 downto 0); -- Transmit nibble (to PHY)
        ENET0_TX_EN   : out   std_logic; -- Transmit enable (to PHY)
        ENET0_TX_ER   : out   std_logic; -- Transmit error (to PHY)

        -- Rx
        ENET0_RX_CLK  : in    std_logic; -- Receive clock (from PHY)
        ENET0_RX_DATA : in    std_logic_vector(3 downto 0); -- Receive nibble (from PHY)
        ENET0_RX_DV   : in    std_logic; -- Receive data valid (from PHY)
        ENET0_RX_ER   : in    std_logic; -- Receive data error (from PHY)

        -- Common Tx and Rx
        ENET0_RX_COL  : in    std_logic; -- Collision (from PHY)
        ENET0_RX_CRS  : in    std_logic; -- Carrier sense (from PHY)

        -- MII Management interface
        ENET0_MDC     : out   std_logic; -- MII Management data clock (to PHY)
        ENET0_MDIO    : inout std_logic;

        ENET0_RST_N   : out   std_logic;

        --second PHY interface
        -- Tx
        ENET1_TX_CLK  : in    std_logic; -- Transmit clock (from PHY)
        ENET1_TX_DATA : out   std_logic_vector(3 downto 0); -- Transmit nibble (to PHY)
        ENET1_TX_EN   : out   std_logic; -- Transmit enable (to PHY)
        ENET1_TX_ER   : out   std_logic; -- Transmit error (to PHY)

        -- Rx
        ENET1_RX_CLK  : in    std_logic; -- Receive clock (from PHY)
        ENET1_RX_DATA : in    std_logic_vector(3 downto 0); -- Receive nibble (from PHY)
        ENET1_RX_DV   : in    std_logic; -- Receive data valid (from PHY)
        ENET1_RX_ER   : in    std_logic; -- Receive data error (from PHY)

        -- Common Tx and Rx
        ENET1_RX_COL  : in    std_logic; -- Collision (from PHY)
        ENET1_RX_CRS  : in    std_logic; -- Carrier sense (from PHY)

        -- MII Management interface
        ENET1_MDC     : out   std_logic; -- MII Management data clock (to PHY)
        ENET1_MDIO    : inout std_logic;

        ENET1_RST_N   : out   std_logic
        );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
            clk                              : in  std_logic;
            reset                            : in  std_logic;
            io_Leds_led                      : out std_logic_vector(8 downto 0);
            io_Keys_key                      : in  std_logic_vector(3 downto 0);
            io_Gpio_gpios_0                  : out std_logic_vector(5 downto 0);
            io_UartCmp_tx                    : out std_logic;
            io_UartCmp_rx                    : in  std_logic;
            io_Uart_tx                       : out std_logic;
            io_Uart_rx                       : in  std_logic;
            io_Uart_1_tx                     : out std_logic;
            io_Uart_1_rx                     : in  std_logic;
            io_SegmentDisplay_hexDisp_7  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_6  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_5  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_4  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_3  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_2  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_1  : out std_logic_vector(6 downto 0);
            io_SegmentDisplay_hexDisp_0  : out std_logic_vector(6 downto 0);

            io_EthMac_mtx_clk_pad_i           : in    std_logic; -- Transmit clock (from PHY)
            io_EthMac_mtxd_pad_o              : out   std_logic_vector(3 downto 0); -- Transmit nibble (to PHY)
            io_EthMac_mtxen_pad_o             : out   std_logic; -- Transmit enable (to PHY)
            io_EthMac_mtxerr_pad_o            : out   std_logic; -- Transmit error (to PHY)
            io_EthMac_mrx_clk_pad_i           : in    std_logic; -- Receive clock (from PHY)
            io_EthMac_mrxd_pad_i              : in    std_logic_vector(3 downto 0); -- Receive nibble (from PHY)
            io_EthMac_mrxdv_pad_i             : in    std_logic; -- Receive data valid (from PHY)
            io_EthMac_mrxerr_pad_i            : in    std_logic; -- Receive data error (from PHY)
            io_EthMac_mcoll_pad_i             : in    std_logic; -- Collision (from PHY)
            io_EthMac_mcrs_pad_i              : in    std_logic; -- Carrier sense (from PHY)
            io_EthMac_md_pad_i                : in    std_logic; -- MII data input (from I/O cell)
            io_EthMac_mdc_pad_o               : out   std_logic; -- MII Management data clock (to PHY)
            io_EthMac_md_pad_o                : out   std_logic; -- MII data output (to I/O cell)
            io_EthMac_md_padoe_o              : out   std_logic; -- MII data output enable (to I/O cell)
            io_EthMac_ptpPPS                  : out   std_logic;
            io_EthMac_ledEOF                  : out   std_logic;
            io_EthMac_ledPHY                  : out   std_logic;
            io_EthMac_ledSOF                  : out   std_logic;

            -- io_EthMac_1_mtx_clk_pad_i           : in    std_logic; -- Transmit clock (from PHY)
            -- io_EthMac_1_mtxd_pad_o              : out   std_logic_vector(3 downto 0); -- Transmit nibble (to PHY)
            -- io_EthMac_1_mtxen_pad_o             : out   std_logic; -- Transmit enable (to PHY)
            -- io_EthMac_1_mtxerr_pad_o            : out   std_logic; -- Transmit error (to PHY)
            -- io_EthMac_1_mrx_clk_pad_i           : in    std_logic; -- Receive clock (from PHY)
            -- io_EthMac_1_mrxd_pad_i              : in    std_logic_vector(3 downto 0); -- Receive nibble (from PHY)
            -- io_EthMac_1_mrxdv_pad_i             : in    std_logic; -- Receive data valid (from PHY)
            -- io_EthMac_1_mrxerr_pad_i            : in    std_logic; -- Receive data error (from PHY)
            -- io_EthMac_1_mcoll_pad_i             : in    std_logic; -- Collision (from PHY)
            -- io_EthMac_1_mcrs_pad_i              : in    std_logic; -- Carrier sense (from PHY)
            -- io_EthMac_1_md_pad_i                : in    std_logic; -- MII data input (from I/O cell)
            -- io_EthMac_1_mdc_pad_o               : out   std_logic; -- MII Management data clock (to PHY)
            -- io_EthMac_1_md_pad_o                : out   std_logic; -- MII data output (to I/O cell)
            -- io_EthMac_1_md_padoe_o              : out   std_logic; -- MII data output enable (to I/O cell)
            -- io_EthMac_1_ptpPPS                  : out   std_logic;
            -- io_EthMac_1_ledEOF                  : out   std_logic;
            -- io_EthMac_1_ledPHY                  : out   std_logic;
            -- io_EthMac_1_ledSOF                  : out   std_logic;

            io_SRamCtrl_ramOut_addr           : out std_logic_vector(19 downto 0);
            io_SRamCtrl_ramOut_doutEna        : out std_logic;
            io_SRamCtrl_ramIn_din             : in  std_logic_vector(15 downto 0);
            io_SRamCtrl_ramOut_dout           : out std_logic_vector(15 downto 0);
            io_SRamCtrl_ramOut_nce            : out std_logic;
            io_SRamCtrl_ramOut_noe            : out std_logic;
            io_SRamCtrl_ramOut_nwe            : out std_logic;
            io_SRamCtrl_ramOut_nlb            : out std_logic;
            io_SRamCtrl_ramOut_nub            : out std_logic
        );
    end component;

    -- DE2-70: 50 MHz clock => 100 MHz
    -- BeMicro: 16 MHz clock => 25.6 MHz
    constant pll_infreq : real    := 50.0;
	constant pll_mult   : natural := 8;
	constant pll_div    : natural := 5;

    signal clk_int : std_logic;

    -- signals for converting i o in io (MII)
    signal md_pad_o_int   : std_logic;
    signal md_padoe_o_int : std_logic;
    signal md2_pad_o_int   : std_logic;
    signal md2_padoe_o_int : std_logic;

    -- for generation of internal reset
    signal int_res            : std_logic;
    signal res_reg1, res_reg2 : std_logic;
    signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    -- sram signals for tristate inout
    signal sram_out_dout_ena : std_logic;
    signal sram_out_dout     : std_logic_vector(15 downto 0);

    attribute altera_attribute : string;
    attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

    signal debug_timestamp_int : std_logic;
    signal debug_mii_tx_en_int : std_logic;

begin
    ENET0_MDIO  <= md_pad_o_int when (md_padoe_o_int = '1') else 'Z';
    ENET0_RST_N <= not int_res;
    ENET1_MDIO  <= md2_pad_o_int when (md2_padoe_o_int = '1') else 'Z';
    ENET1_RST_N <= not int_res;

    pll_inst : entity work.pll generic map(
        input_freq  => pll_infreq,
        multiply_by => pll_mult,
        divide_by   => pll_div
        )
        port map(
            inclk0 => clk,
            c0     => clk_int
            );
    -- we use a PLL
    -- clk_int <= clk;

    --
    --  internal reset generation
    --  should include the PLL lock signal
    --
    process(clk_int)
    begin
        if rising_edge(clk_int) then
            if (res_cnt /= "111") then
                res_cnt <= res_cnt + 1;
            end if;
            res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
            res_reg2 <= res_reg1;
            int_res  <= res_reg2;
        end if;
    end process;

    -- tristate output to ssram
    process(sram_out_dout_ena, sram_out_dout)
    begin
        if sram_out_dout_ena = '1' then
            SRAM_DQ <= sram_out_dout;
        else
            SRAM_DQ <= (others => 'Z');
        end if;
    end process;

    oLedsPins_ledR(17) <=  debug_timestamp_int;
    oGpioPins_gpio_0(4) <= ENET0_RX_DV;
    oGpioPins_gpio_0(5) <= debug_mii_tx_en_int;
    oGpioPins_gpio_0(6) <= debug_timestamp_int;
    ENET0_TX_EN <= debug_mii_tx_en_int;

    patmos_inst : Patmos port map(
        clk => clk_int, 
        reset => int_res,
        io_Leds_led => oLedsPins_led,
        io_Keys_key => iKeysPins_key,
        io_Gpio_gpios_0(3 downto 0) => oGpioPins_gpio_0(3 downto 0),
        io_UartCmp_tx => oUartPins_txd, 
        io_UartCmp_rx => iUartPins_rxd,
        io_Uart_tx => oUart2Pins_txd,
        io_Uart_rx => iUart2Pins_rxd,
        io_Uart_1_tx => oUart3Pins_txd,
        io_Uart_1_rx => iUart3Pins_rxd,                     
        io_EthMac_mtx_clk_pad_i => ENET0_TX_CLK,
        io_EthMac_mtxd_pad_o => ENET0_TX_DATA,
        io_EthMac_mtxen_pad_o => debug_mii_tx_en_int,
        io_EthMac_mtxerr_pad_o => ENET0_TX_ER,
        io_EthMac_mrx_clk_pad_i => ENET0_RX_CLK,
        io_EthMac_mrxd_pad_i => ENET0_RX_DATA,
        io_EthMac_mrxdv_pad_i => ENET0_RX_DV,
        io_EthMac_mrxerr_pad_i => ENET0_RX_ER,
        io_EthMac_mcoll_pad_i => ENET0_RX_COL,
        io_EthMac_mcrs_pad_i => ENET0_RX_CRS,
        io_EthMac_md_pad_i => ENET0_MDIO,
        io_EthMac_mdc_pad_o => ENET0_MDC,
        io_EthMac_md_pad_o => md_pad_o_int,
        io_EthMac_md_padoe_o => md_padoe_o_int,
        io_EthMac_ptpPPS => oEthPPS,
        io_EthMac_ledPHY => debug_timestamp_int,
        io_EthMac_ledSOF => oLedsPins_ledR(16),
        io_EthMac_ledEOF => oLedsPins_ledR(15),

        -- io_EthMac_1_mtx_clk_pad_i => ENET1_TX_CLK,
        -- io_EthMac_1_mtxd_pad_o => ENET1_TX_DATA,
        -- io_EthMac_1_mtxen_pad_o => ENET1_TX_EN,
        -- io_EthMac_1_mtxerr_pad_o => ENET1_TX_ER,
        -- io_EthMac_1_mrx_clk_pad_i => ENET1_RX_CLK,
        -- io_EthMac_1_mrxd_pad_i => ENET1_RX_DATA,
        -- io_EthMac_1_mrxdv_pad_i => ENET1_RX_DV,
        -- io_EthMac_1_mrxerr_pad_i => ENET1_RX_ER,
        -- io_EthMac_1_mcoll_pad_i => ENET1_RX_COL,
        -- io_EthMac_1_mcrs_pad_i => ENET1_RX_CRS,
        -- io_EthMac_1_md_pad_i => ENET1_MDIO,
        -- io_EthMac_1_mdc_pad_o => ENET1_MDC,
        -- io_EthMac_1_md_pad_o => md2_pad_o_int,
        -- io_EthMac_1_md_padoe_o => md2_padoe_o_int,
        -- io_EthMac_1_ptpPPS => oEth2PPS,

        io_SegmentDisplay_hexDisp_7 => oSegmentDisplayPins_hexDisp_7,
        io_SegmentDisplay_hexDisp_6 => oSegmentDisplayPins_hexDisp_6,
        io_SegmentDisplay_hexDisp_5 => oSegmentDisplayPins_hexDisp_5,
        io_SegmentDisplay_hexDisp_4 => oSegmentDisplayPins_hexDisp_4,
        io_SegmentDisplay_hexDisp_3 => oSegmentDisplayPins_hexDisp_3,
        io_SegmentDisplay_hexDisp_2 => oSegmentDisplayPins_hexDisp_2,
        io_SegmentDisplay_hexDisp_1 => oSegmentDisplayPins_hexDisp_1,
        io_SegmentDisplay_hexDisp_0 => oSegmentDisplayPins_hexDisp_0,

        io_SRamCtrl_ramOut_addr => oSRAM_A, 
        io_SRamCtrl_ramOut_doutEna => sram_out_dout_ena,
        io_SRamCtrl_ramIn_din => SRAM_DQ,
        io_SRamCtrl_ramOut_dout => sram_out_dout, 
        io_SRamCtrl_ramOut_nce => oSRAM_CE_N, 
        io_SRamCtrl_ramOut_noe => oSRAM_OE_N, 
        io_SRamCtrl_ramOut_nwe => oSRAM_WE_N, 
        io_SRamCtrl_ramOut_nlb => oSRAM_LB_N, 
        io_SRamCtrl_ramOut_nub => oSRAM_UB_N
    );

end architecture rtl;

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
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
        clk           : in    std_logic;
        oLedsPins_led : out   std_logic_vector(8 downto 0);
        iKeysPins_key : in    std_logic_vector(3 downto 0);
        oUartPins_txd : out   std_logic;
        iUartPins_rxd : in    std_logic;
        oSRAM_A       : out   std_logic_vector(19 downto 0);
        SRAM_DQ       : inout std_logic_vector(15 downto 0);
        oSRAM_CE_N    : out   std_logic;
        oSRAM_OE_N    : out   std_logic;
        oSRAM_WE_N    : out   std_logic;
        oSRAM_LB_N    : out   std_logic;
        oSRAM_UB_N    : out   std_logic;

        --PHY interface
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

        ENET0_RST_N   : out   std_logic
        );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
            clk                                   : in  std_logic;
            reset                                 : in  std_logic;

            io_comConf_M_Cmd                      : out std_logic_vector(2 downto 0);
            io_comConf_M_Addr                     : out std_logic_vector(31 downto 0);
            io_comConf_M_Data                     : out std_logic_vector(31 downto 0);
            io_comConf_M_ByteEn                   : out std_logic_vector(3 downto 0);
            io_comConf_M_RespAccept               : out std_logic;
            io_comConf_S_Resp                     : in  std_logic_vector(1 downto 0);
            io_comConf_S_Data                     : in  std_logic_vector(31 downto 0);
            io_comConf_S_CmdAccept                : in  std_logic;

            io_comSpm_M_Cmd                       : out std_logic_vector(2 downto 0);
            io_comSpm_M_Addr                      : out std_logic_vector(31 downto 0);
            io_comSpm_M_Data                      : out std_logic_vector(31 downto 0);
            io_comSpm_M_ByteEn                    : out std_logic_vector(3 downto 0);
            io_comSpm_S_Resp                      : in  std_logic_vector(1 downto 0);
            io_comSpm_S_Data                      : in  std_logic_vector(31 downto 0);

            io_ledsPins_led                       : out std_logic_vector(8 downto 0);
            io_keysPins_key                       : in  std_logic_vector(3 downto 0);
            io_uartPins_tx                        : out std_logic;
            io_uartPins_rx                        : in  std_logic;

            io_ethMacPins_mtx_clk_pad_i           : in    std_logic; -- Transmit clock (from PHY)
            io_ethMacPins_mtxd_pad_o              : out   std_logic_vector(3 downto 0); -- Transmit nibble (to PHY)
            io_ethMacPins_mtxen_pad_o             : out   std_logic; -- Transmit enable (to PHY)
            io_ethMacPins_mtxerr_pad_o            : out   std_logic; -- Transmit error (to PHY)
            io_ethMacPins_mrx_clk_pad_i           : in    std_logic; -- Receive clock (from PHY)
            io_ethMacPins_mrxd_pad_i              : in    std_logic_vector(3 downto 0); -- Receive nibble (from PHY)
            io_ethMacPins_mrxdv_pad_i             : in    std_logic; -- Receive data valid (from PHY)
            io_ethMacPins_mrxerr_pad_i            : in    std_logic; -- Receive data error (from PHY)
            io_ethMacPins_mcoll_pad_i             : in    std_logic; -- Collision (from PHY)
            io_ethMacPins_mcrs_pad_i              : in    std_logic; -- Carrier sense (from PHY)
            io_ethMacPins_md_pad_i                : in    std_logic; -- MII data input (from I/O cell)
            io_ethMacPins_mdc_pad_o               : out   std_logic; -- MII Management data clock (to PHY)
            io_ethMacPins_md_pad_o                : out   std_logic; -- MII data output (to I/O cell)
            io_ethMacPins_md_padoe_o              : out   std_logic; -- MII data output enable (to I/O cell)

            io_sramCtrlPins_ramOut_addr           : out std_logic_vector(19 downto 0);
            io_sramCtrlPins_ramOut_doutEna        : out std_logic;
            io_sramCtrlPins_ramIn_din             : in  std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_dout           : out std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_nce            : out std_logic;
            io_sramCtrlPins_ramOut_noe            : out std_logic;
            io_sramCtrlPins_ramOut_nwe            : out std_logic;
            io_sramCtrlPins_ramOut_nlb            : out std_logic;
            io_sramCtrlPins_ramOut_nub            : out std_logic
            );
    end component;

    -- DE2-70: 50 MHz clock => 80 MHz
    -- BeMicro: 16 MHz clock => 25.6 MHz
    constant pll_infreq : real    := 50.0;
    constant pll_mult   : natural := 8;
    constant pll_div    : natural := 5;

    signal clk_int : std_logic;

    -- signals for converting i o in io (MII)
    signal md_pad_o_int   : std_logic;
    signal md_padoe_o_int : std_logic;

    -- for generation of internal reset
    signal int_res            : std_logic;
    signal res_reg1, res_reg2 : std_logic;
    signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    -- sram signals for tristate inout
    signal sram_out_dout_ena : std_logic;
    signal sram_out_dout     : std_logic_vector(15 downto 0);

    attribute altera_attribute : string;
    attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
    ENET0_MDIO  <= md_pad_o_int when (md_padoe_o_int = '1') else 'Z';
    ENET0_RST_N <= not int_res;

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

    comp : Patmos port map(clk_int, int_res,
                           open, open, open, open, open,
                           (others => '0'), (others => '0'), '0',
                           open, open, open, open,
                           (others => '0'), (others => '0'),
                           oLedsPins_led,
                           iKeysPins_key,
                           oUartPins_txd, 
                           iUartPins_rxd,                           
                           ENET0_TX_CLK,
                           ENET0_TX_DATA,
                           ENET0_TX_EN,
                           ENET0_TX_ER,
                           ENET0_RX_CLK,
                           ENET0_RX_DATA,
                           ENET0_RX_DV,
                           ENET0_RX_ER,
                           ENET0_RX_COL,
                           ENET0_RX_CRS,
                           ENET0_MDIO,
                           ENET0_MDC,
                           md_pad_o_int,
                           md_padoe_o_int,
                           oSRAM_A, 
                           sram_out_dout_ena, SRAM_DQ, sram_out_dout, oSRAM_CE_N, oSRAM_OE_N, oSRAM_WE_N, oSRAM_LB_N, oSRAM_UB_N);

end architecture rtl;

--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
--         Rasmus Bo Soerensen (rasmus@rbscloud.dk)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel on Altera de2-115 board
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
  port(
    clk_in_p     : in    std_logic;
    clk_in_n     : in    std_logic;
    led : out std_logic_vector(3 downto 0);
    btn : in std_logic_vector(3 downto 0);
    oUartPins_txd : out std_logic;
    iUartPins_rxd : in  std_logic;
    oSRAM_A : out std_logic_vector(19 downto 0);
    SRAM_DQ : inout std_logic_vector(15 downto 0);
    oSRAM_CE_N : out std_logic;
    oSRAM_OE_N : out std_logic;
    oSRAM_WE_N : out std_logic;
    oSRAM_LB_N : out std_logic;
    oSRAM_UB_N : out std_logic
  );
end entity patmos_top;

architecture rtl of patmos_top is
	component Patmos is
		port(
			clock           : in  std_logic;
			reset           : in  std_logic;

      io_Leds_led : out std_logic_vector(3 downto 0);
      io_Keys_key : in  std_logic_vector(3 downto 0);
      io_UartCmp_tx  : out std_logic;
      io_UartCmp_rx  : in  std_logic

    );
  end component;
  
 component clk_manager is
    port(
      clk_in_p  : in  std_logic;
      clk_in_n  : in  std_logic;
      clk_out_1 : out std_logic;
      locked    : out std_logic
    );
  end component;

  -- DE2-70: 50 MHz clock => 80 MHz
  -- BeMicro: 16 MHz clock => 25.6 MHz
  constant pll_infreq : real    := 50.0;
  constant pll_mult   : natural := 8;
  constant pll_div    : natural := 5;
  
  
  signal clk_int : std_logic;
  signal clk_200 : std_logic;

  -- for generation of internal reset
  signal int_res, int_res_n                     : std_logic;
  signal res_reg1, res_reg2, res_reg3, res_reg4 : std_logic;
  signal locked                                 : std_logic;

--  signal clk_int : std_logic;

--  -- for generation of internal reset
--  signal int_res            : std_logic;
--  signal res_reg1, res_reg2 : std_logic;
  signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

    -- sram signals for tristate inout
    signal sram_out_dout_ena : std_logic;
    signal sram_out_dout : std_logic_vector(15 downto 0);

  attribute altera_attribute : string;
  attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

begin
  clk_manager_inst_0 : clk_manager port map(
      clk_in_p  => clk_in_p,
      clk_in_n  => clk_in_n,
      clk_out_1 => clk_200,
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



    comp : Patmos port map(clk_int, int_res,
           led,
           btn,
           oUartPins_txd, iUartPins_rxd);

end architecture rtl;

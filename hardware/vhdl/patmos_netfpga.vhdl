--
-- Copyright: 2020, Technical University of Denmark, DTU Compute
-- Author:
--         
-- License: Simplified BSD License
--

-- 
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
  port(
    --CLK
    clk_int     : in    std_logic;
    locked     : in    std_logic;
    --GPIO
    led : out std_logic_vector(3 downto 0);
    btn : in std_logic_vector(3 downto 0);
    --UART
    oUartPins_txd : out std_logic;
    iUartPins_rxd : in  std_logic;
    --AXI
    m_axi_awaddr : out std_logic_vector(11 downto 0);
    m_axi_bresp : in std_logic_vector(1 downto 0);
    m_axi_rresp : in std_logic_vector(1 downto 0);
    m_axi_wstrb : out std_logic_vector(3 downto 0);
    m_axi_wready : in std_logic;
    m_axi_rready : out std_logic;
    m_axi_bready : out std_logic;
    m_axi_arvalid : out std_logic;
    m_axi_araddr : out std_logic_vector(11 downto 0);
    m_axi_awready : in std_logic;
    m_axi_bvalid : in std_logic;
    m_axi_rvalid : in std_logic;
    m_axi_wvalid : out std_logic;
    m_axi_wdata : out std_logic_vector(31 downto 0);
    m_axi_rdata : in std_logic_vector(31 downto 0);
    m_axi_awvalid : out std_logic;
    m_axi_arready : in std_logic
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
      io_UartCmp_rx  : in  std_logic;
    io_AXI4LiteMMBridge_awaddr : out std_logic_vector(11 downto 0);
    io_AXI4LiteMMBridge_bresp : in std_logic_vector(1 downto 0);
    io_AXI4LiteMMBridge_rresp : in std_logic_vector(1 downto 0);
    io_AXI4LiteMMBridge_wstrb : out std_logic_vector(3 downto 0);
    io_AXI4LiteMMBridge_wready : in std_logic;
    io_AXI4LiteMMBridge_rready : out std_logic;
    io_AXI4LiteMMBridge_bready : out std_logic;
    io_AXI4LiteMMBridge_arvalid : out std_logic;
    io_AXI4LiteMMBridge_araddr : out std_logic_vector(11 downto 0);
    io_AXI4LiteMMBridge_awready : in std_logic;
    io_AXI4LiteMMBridge_bvalid : in std_logic;
    io_AXI4LiteMMBridge_rvalid : in std_logic;
    io_AXI4LiteMMBridge_wvalid : out std_logic;
    io_AXI4LiteMMBridge_wdata : out std_logic_vector(31 downto 0);
    io_AXI4LiteMMBridge_rdata : in std_logic_vector(31 downto 0);
    io_AXI4LiteMMBridge_awvalid : out std_logic;
    io_AXI4LiteMMBridge_arready : in std_logic

    );
  end component;
  


  -- for generation of internal reset
  signal int_res, int_res_n                     : std_logic;
  signal res_reg1, res_reg2, res_reg3, res_reg4 : std_logic;

  signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

begin
 

  
  --  internal reset generation
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

  --
  --  internal reset generation
--  process(clk_int)
--  begin
--    if rising_edge(clk_int) then
--      res_reg3  <= int_res;
--      res_reg4  <= res_reg3;
--      int_res_n <= not res_reg4;  --reset active high (when 0 patmos is running)
--    --int_res <= res_reg2;
--    end if;
--  end process;



    comp : Patmos port map(
                    clock => clk_int, 
                    reset => int_res,
                    io_Leds_led => led,
                    io_Keys_key => btn,
                    io_UartCmp_tx => oUartPins_txd, 
                    io_UartCmp_rx => iUartPins_rxd,
                    io_AXI4LiteMMBridge_awaddr => m_axi_awaddr,
                    io_AXI4LiteMMBridge_bresp => m_axi_bresp,
                    io_AXI4LiteMMBridge_rresp => m_axi_rresp,
                    io_AXI4LiteMMBridge_wstrb => m_axi_wstrb,
                    io_AXI4LiteMMBridge_wready => m_axi_wready,
                    io_AXI4LiteMMBridge_rready => m_axi_rready,
                    io_AXI4LiteMMBridge_bready => m_axi_bready,
                    io_AXI4LiteMMBridge_arvalid => m_axi_arvalid,
                    io_AXI4LiteMMBridge_araddr => m_axi_araddr,
                    io_AXI4LiteMMBridge_awready => m_axi_awready,
                    io_AXI4LiteMMBridge_bvalid => m_axi_bvalid,
                    io_AXI4LiteMMBridge_rvalid => m_axi_rvalid,
                    io_AXI4LiteMMBridge_wvalid => m_axi_wvalid,
                    io_AXI4LiteMMBridge_wdata => m_axi_wdata,
                    io_AXI4LiteMMBridge_rdata => m_axi_rdata,
                    io_AXI4LiteMMBridge_awvalid => m_axi_awvalid,
                    io_AXI4LiteMMBridge_arready => m_axi_arready);

end architecture rtl;

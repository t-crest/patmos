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
    clk_in_p     : in    std_logic;
    clk_in_n     : in    std_logic;

    led          : out   std_logic_vector(7 downto 0);

    --TXD, RXD naming uses terminal-centric naming convention
    uart_txd     : in    std_logic;
    uart_rxd     : out   std_logic;

    --DDR3 pins
    ddr3_dq      : inout std_logic_vector(31 downto 0);
    ddr3_dqs_p   : inout std_logic_vector(3 downto 0);
    ddr3_dqs_n   : inout std_logic_vector(3 downto 0);

    ddr3_addr    : out   std_logic_vector(14 downto 0);
    ddr3_ba      : out   std_logic_vector(2 downto 0);
    ddr3_ras_n   : out   std_logic;
    ddr3_cas_n   : out   std_logic;
    ddr3_we_n    : out   std_logic;
    ddr3_reset_n : out   std_logic;
    ddr3_ck_p    : out   std_logic_vector(0 downto 0);
    ddr3_ck_n    : out   std_logic_vector(0 downto 0);
    ddr3_cke     : out   std_logic_vector(0 downto 0);
    ddr3_cs_n    : out   std_logic_vector(0 downto 0);
    ddr3_dm      : out   std_logic_vector(3 downto 0);
    ddr3_odt     : out   std_logic_vector(0 downto 0)
  );
end entity patmos_top;

architecture rtl of patmos_top is
  component Patmos is
    port(
      clk                           : in  std_logic;
      reset                         : in  std_logic;

      io_MemBridge_M_Cmd        : out std_logic_vector(2 downto 0);
      io_MemBridge_M_Addr       : out std_logic_vector(31 downto 0);
      io_MemBridge_M_Data       : out std_logic_vector(31 downto 0);
      io_MemBridge_M_DataValid  : out std_logic;
      io_MemBridge_M_DataByteEn : out std_logic_vector(3 downto 0);
      io_MemBridge_S_Resp       : in  std_logic_vector(1 downto 0);
      io_MemBridge_S_Data       : in  std_logic_vector(31 downto 0);
      io_MemBridge_S_CmdAccept  : in  std_logic;
      io_MemBridge_S_DataAccept : in  std_logic;

      io_Leds_led               : out std_logic_vector(7 downto 0);
      io_UartCmp_tx                : out std_logic;
      io_UartCmp_rx                : in  std_logic
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
      locked    : out std_logic
    );
  end component;

  signal clk_int : std_logic;
  signal clk_200 : std_logic;

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

--          attribute mark_debug : string;
--          attribute mark_debug of app_addr_bridge             : signal is "true"; --   std_logic_vector(28 downto 0); --
--          attribute mark_debug of app_cmd_bridge              : signal is "true"; --   std_logic_vector(2 downto 0); --
--          attribute mark_debug of app_en_bridge               : signal is "true"; --   std_logic;
--          attribute mark_debug of app_wdf_data_bridge         : signal is "true"; --   std_logic_vector(255 downto 0);
--          attribute mark_debug of app_wdf_end_bridge          : signal is "true"; --   std_logic;
--          attribute mark_debug of app_wdf_mask_bridge         : signal is "true"; --   std_logic_vector(31 downto 0);
--          attribute mark_debug of app_wdf_wren_bridge         : signal is "true"; --   std_logic;
--          attribute mark_debug of app_rd_data_bridge          : signal is "true"; --   std_logic_vector(255 downto 0);--
--          attribute mark_debug of app_rd_data_end_bridge      : signal is "true"; --   std_logic;--
--          attribute mark_debug of app_rd_data_valid_bridge    : signal is "true"; --   std_logic;
--          attribute mark_debug of app_rdy_bridge              : signal is "true"; --   std_logic;
--          attribute mark_debug of app_wdf_rdy_bridge          : signal is "true"; --   std_logic;

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

  -- The instance of the patmos processor            
  patmos_inst_0 : Patmos port map(
      clk                           => clk_int,
      reset                         => int_res_n,
      
      io_MemBridge_M_Cmd        => MCmd_bridge, --: out std_logic_vector(2 downto 0);
      io_MemBridge_M_Addr       => MAddr_bridge, --: out std_logic_vector(31 downto 0);
      io_MemBridge_M_Data       => MData_bridge, --: out std_logic_vector(31 downto 0);
      io_MemBridge_M_DataValid  => MDataValid_bridge, --: out std_logic;
      io_MemBridge_M_DataByteEn => MDataByteEn_bridge, --: out std_logic_vector(3 downto 0);
      io_MemBridge_S_Resp       => SResp_bridge, --: in std_logic_vector(1 downto 0);
      io_MemBridge_S_Data       => SData_bridge, --: in std_logic_vector(31 downto 0);
      io_MemBridge_S_CmdAccept  => SCmdAccept_bridge, --: in std_logic;
      io_MemBridge_S_DataAccept => SDataAccept_bridge, --: in std_logic;
      
      io_Leds_led               => led,
      io_UartCmp_tx                => uart_rxd, --TXD, RXD naming uses terminal-centric naming convention
      io_UartCmp_rx                => uart_txd --TXD, RXD naming uses terminal-centric naming convention
    );

end architecture rtl;

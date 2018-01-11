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
			inclk0 : in std_logic;
			button : in std_logic_vector(3 downto 0);
			LED	: out std_logic_vector(24 downto 0);
	
		--Combined Resolver / Quadrature / Hall effect interface
		drive0_res_quad_hall : inout std_logic_vector(2 downto 0); -- 0 = resolver stim output, 1 = resolver sin input, 2 = resolver cos input
		drive0_resolver_feedback_clk : in std_logic;
		--ADC
		drive0_adc_Sync_Dat_U : in std_logic;
		drive0_adc_Sync_Dat_W : in std_logic;
		drive0_adc_Sync_Dat_ipow : in std_logic;
		-- IGBT Control
		drive0_pwm_u_h : out std_logic;
		drive0_pwm_u_l : out std_logic;
		drive0_pwm_v_h : out std_logic;
		drive0_pwm_v_l : out std_logic;
		drive0_pwm_w_h : out std_logic;
		drive0_pwm_w_l : out std_logic;
		drive0_sm_igbt_err : in std_logic;

		sys_braking : out std_logic;
		dc_link_Sync_Dat_VBUS : in std_logic;
		dc_link_Sync_Dat_ipow : in std_logic;
		--ADC Clks
		sys_adc_clk : out std_logic;
		sys_adc_feedback_clk : in std_logic;
		sys_pfc_ld_en : in std_logic;
		sys_pfc_pfw : in std_logic;
		sys_pfc_en : out std_logic;
	
		oUartPins_txd : out std_logic;
		iUartPins_rxd : in  std_logic;
		oSRAM_A : out std_logic_vector(19 downto 0);
		SRAM_DQ : inout std_logic_vector(15 downto 0);
		oSRAM_CE_N : out std_logic;
		oSRAM_OE_N : out std_logic;
		oSRAM_WE_N : out std_logic;
		oSRAM_LB_N : out std_logic;
		oSRAM_UB_N : out std_logic;

		-- Debug signals
		debug_sys_adc_clk : out std_logic;
		debug_sys_adc_feedback_clk : out std_logic;
		debug_dc_link_Sync_Dat_VBUS : out std_logic;
		debug_drive0_adc_Sync_Dat_U : out std_logic;
		debug_drive0_adc_Sync_Dat_W : out std_logic;
		debug_ground : out std_logic;
		debug_irg : out std_logic
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

			io_ledsPins_led : out std_logic_vector(8 downto 0);
			io_keysPins_key : in  std_logic_vector(3 downto 0);
			io_uartPins_tx  : out std_logic;
			io_uartPins_rx  : in  std_logic;
			
			io_avalonMMBridgePins_avs_waitrequest : in std_logic;
			io_avalonMMBridgePins_avs_readdata : in std_logic_vector(31 downto 0);
			io_avalonMMBridgePins_avs_readdatavalid : in std_logic;
			io_avalonMMBridgePins_avs_burstcount  : out std_logic;
			io_avalonMMBridgePins_avs_writedata : out std_logic_vector(31 downto 0);
			io_avalonMMBridgePins_avs_address : out std_logic_vector(15 downto 0);
			io_avalonMMBridgePins_avs_write : out std_logic;
			io_avalonMMBridgePins_avs_read : out std_logic;
			io_avalonMMBridgePins_avs_byteenable  : out std_logic_vector(3 downto 0);
			io_avalonMMBridgePins_avs_debugaccess : out std_logic;
			io_avalonMMBridgePins_avs_intr  : in std_logic;

            io_sramCtrlPins_ramOut_addr : out std_logic_vector(19 downto 0);
            io_sramCtrlPins_ramOut_doutEna : out std_logic;
            io_sramCtrlPins_ramIn_din : in std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_dout : out std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_nce : out std_logic;
            io_sramCtrlPins_ramOut_noe : out std_logic;
            io_sramCtrlPins_ramOut_nwe : out std_logic;
            io_sramCtrlPins_ramOut_nlb : out std_logic;
            io_sramCtrlPins_ramOut_nub : out std_logic

		);
	end component;
	
	component altpll_patmos is
		PORT	(
		inclk0		: IN STD_LOGIC  := '0';
		c0		: OUT STD_LOGIC ;
		c1		: OUT STD_LOGIC ;
		c2		: OUT STD_LOGIC ;
		locked		: OUT STD_LOGIC 
	);
	end component;
	
	component DOC_Axis_Periphs_patmos is
		port (
			drive0_adc_sync_dat_u           : in  std_logic                     := 'X';             -- sync_dat_u
			drive0_adc_sync_dat_w           : in  std_logic                     := 'X';             -- sync_dat_w
			drive0_adc_overcurrent          : out std_logic;                                        -- overcurrent
			drive0_pwm_carrier              : out std_logic_vector(15 downto 0);                    -- carrier
			drive0_pwm_carrier_latch        : out std_logic;                                        -- carrier_latch
			drive0_pwm_encoder_strobe_n     : out std_logic;                                        -- encoder_strobe_n
			drive0_pwm_u_h                  : out std_logic;                                        -- u_h
			drive0_pwm_u_l                  : out std_logic;                                        -- u_l
			drive0_pwm_v_h                  : out std_logic;                                        -- v_h
			drive0_pwm_v_l                  : out std_logic;                                        -- v_l
			drive0_pwm_w_h                  : out std_logic;                                        -- w_h
			drive0_pwm_w_l                  : out std_logic;                                        -- w_l
			drive0_sm_overcurrent           : in  std_logic                     := 'X';             -- overcurrent
			drive0_sm_overvoltage           : in  std_logic                     := 'X';             -- overvoltage
			drive0_sm_undervoltage          : in  std_logic                     := 'X';             -- undervoltage
			drive0_sm_chopper               : in  std_logic                     := 'X';             -- chopper
			drive0_sm_dc_link_clk_err       : in  std_logic                     := 'X';             -- dc_link_clk_err
			drive0_sm_igbt_err              : in  std_logic                     := 'X';             -- igbt_err
			drive0_sm_error_out             : out std_logic;                                        -- error_out
			drive0_sm_overcurrent_latch     : out std_logic;                                        -- overcurrent_latch
			drive0_sm_overvoltage_latch     : out std_logic;                                        -- overvoltage_latch
			drive0_sm_undervoltage_latch    : out std_logic;                                        -- undervoltage_latch
			drive0_sm_dc_link_clk_err_latch : out std_logic;                                        -- dc_link_clk_err_latch
			drive0_sm_igbt_err_latch        : out std_logic;                                        -- igbt_err_latch
			drive0_sm_chopper_latch         : out std_logic;                                        -- chopper_latch
			drive0_adc_pow_sync_dat_u       : in  std_logic                     := 'X';             -- sync_dat_u
			drive0_adc_pow_sync_dat_w       : in  std_logic                     := 'X';             -- sync_dat_w
			drive0_adc_pow_overcurrent      : out std_logic;                                        -- overcurrent
			clk_adc_in_clk                  : in  std_logic                     := 'X';             -- clk
			drive0_doc_pwm_sync_out_export  : out std_logic;                                        -- export
			drive0_doc_pwm_sync_in_export   : in  std_logic                     := 'X';             -- export
			drive0_doc_adc_irq_irq          : out std_logic;                                        -- irq
			drive0_doc_adc_pow_irq_irq      : out std_logic;                                        -- irq
			avs_periph_slave_waitrequest    : out std_logic;                                        -- waitrequest
			avs_periph_slave_readdata       : out std_logic_vector(31 downto 0);                    -- readdata
			avs_periph_slave_readdatavalid  : out std_logic;                                        -- readdatavalid
			avs_periph_slave_burstcount     : in  std_logic_vector(0 downto 0)  := (others => 'X'); -- burstcount
			avs_periph_slave_writedata      : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			avs_periph_slave_address        : in  std_logic_vector(11 downto 0) := (others => 'X'); -- address
			avs_periph_slave_write          : in  std_logic                     := 'X';             -- write
			avs_periph_slave_read           : in  std_logic                     := 'X';             -- read
			avs_periph_slave_byteenable     : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- byteenable
			avs_periph_slave_debugaccess    : in  std_logic                     := 'X';             -- debugaccess
			reset_reset_n                   : in  std_logic                     := 'X';             -- reset_n
			clk_50_clk                      : in  std_logic                     := 'X';             -- clk
			clk_80_clk                      : in  std_logic                     := 'X';             -- clk
			reset_80_reset_n                : in  std_logic                     := 'X'              -- reset_n
		);
	end component DOC_Axis_Periphs_patmos;
	
	component DOC_Monitor is
		port (
			in_port_to_the_IO_IN_Buttons   : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			pio_pfc_in_port                : in  std_logic_vector(1 downto 0)  := (others => 'X'); -- in_port
			pio_pfc_out_port               : out std_logic_vector(1 downto 0);                     -- out_port
			dc_link_sync_dat               : in  std_logic                     := 'X';             -- sync_dat
			dc_link_dc_link_enable         : in  std_logic                     := 'X';             -- dc_link_enable
			dc_link_overvoltage            : out std_logic;                                        -- overvoltage
			dc_link_undervoltage           : out std_logic;                                        -- undervoltage
			dc_link_chopper                : out std_logic;                                        -- chopper
			dc_link_p_sync_dat             : in  std_logic                     := 'X';             -- sync_dat
			dc_link_p_dc_link_enable       : in  std_logic                     := 'X';             -- dc_link_enable
			dc_link_p_overvoltage          : out std_logic;                                        -- overvoltage
			dc_link_p_undervoltage         : out std_logic;                                        -- undervoltage
			dc_link_p_chopper              : out std_logic;                                        -- chopper
			clk_adc_in_clk                 : in  std_logic                     := 'X';             -- clk
			avs_periph_slave_waitrequest   : out std_logic;                                        -- waitrequest
			avs_periph_slave_readdata      : out std_logic_vector(31 downto 0);                    -- readdata
			avs_periph_slave_readdatavalid : out std_logic;                                        -- readdatavalid
			avs_periph_slave_burstcount    : in  std_logic_vector(0 downto 0)  := (others => 'X'); -- burstcount
			avs_periph_slave_writedata     : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			avs_periph_slave_address       : in  std_logic_vector(11 downto 0) := (others => 'X'); -- address
			avs_periph_slave_write         : in  std_logic                     := 'X';             -- write
			avs_periph_slave_read          : in  std_logic                     := 'X';             -- read
			avs_periph_slave_byteenable    : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- byteenable
			avs_periph_slave_debugaccess   : in  std_logic                     := 'X';             -- debugaccess
			clk_80_clk                     : in  std_logic                     := 'X';             --                            clk_80.clk
			reset_80_reset_n               : in  std_logic                     := 'X';             --                          reset_80.reset_n
			clk_50_clk                     : in  std_logic                     := 'X';             --                            clk_50.clk
			reset_50_reset_n               : in  std_logic                     := 'X'               -- reset_n
		);
	end component DOC_Monitor;
	
	-- DE2-70: 50 MHz clock => 80 MHz
	-- BeMicro: 16 MHz clock => 25.6 MHz
	constant pll_infreq : real    := 50.0;
	constant pll_mult   : natural := 8;
	constant pll_div    : natural := 5;

	signal clk_int : std_logic;
	signal adc_clk : std_logic;

	-- for generation of internal reset
	signal res_50_reg1, res_50_reg2 : std_logic := '0';
	signal res_80_reg1, res_80_reg2 : std_logic := '0';
	signal res_50_cnt, res_80_cnt   : unsigned(2 downto 0) := "000"; -- for the simulation

	signal reset_50_n, reset_80_n		  : std_logic := '0';
	signal reset_50, reset_80			  : std_logic := '1';

    -- sram signals for tristate inout
    signal sram_out_dout_ena : std_logic;
    signal sram_out_dout : std_logic_vector(15 downto 0);

	attribute altera_attribute : string;
	attribute altera_attribute of res_50_cnt : signal is "POWER_UP_LEVEL=LOW";
	attribute altera_attribute of res_80_cnt : signal is "POWER_UP_LEVEL=LOW";
	
	signal drive0_avs_waitrequest 	:  std_logic;
	signal drive0_avs_readdata 	:  std_logic_vector(31 downto 0);
	signal drive0_avs_readdatavalid 	:  std_logic;
	signal drive0_avs_burstcount  	:  std_logic_vector(0 downto 0);
	signal drive0_avs_writedata 	:  std_logic_vector(31 downto 0);
	signal drive0_avs_address 	:  std_logic_vector(11 downto 0);
	signal drive0_avs_write 	:  std_logic;
	signal drive0_avs_read 	:  std_logic;
	signal drive0_avs_byteenable  	:  std_logic_vector(3 downto 0);
	signal drive0_avs_debugaccess 	:  std_logic;
	signal drive0_avs_intr  	:  std_logic  := '0';

	signal monitor_avs_waitrequest 	:  std_logic;
	signal monitor_avs_readdata 	:  std_logic_vector(31 downto 0);
	signal monitor_avs_readdatavalid 	:  std_logic;
	signal monitor_avs_burstcount  	:  std_logic_vector(0 downto 0);
	signal monitor_avs_writedata 	:  std_logic_vector(31 downto 0);
	signal monitor_avs_address 	:  std_logic_vector(11 downto 0);
	signal monitor_avs_write 	:  std_logic;
	signal monitor_avs_read 	:  std_logic;
	signal monitor_avs_byteenable  	:  std_logic_vector(3 downto 0);
	signal monitor_avs_debugaccess 	:  std_logic;
	signal monitor_avs_intr  	:  std_logic  := '0';

	signal avs_waitrequest 	:  std_logic;
	signal avs_readdata 	:  std_logic_vector(31 downto 0);
	signal avs_readdatavalid 	:  std_logic;
	signal avs_burstcount  	:  std_logic_vector(0 downto 0);
	signal avs_writedata 	:  std_logic_vector(31 downto 0);
	signal avs_address 	:  std_logic_vector(15 downto 0);
	signal avs_write 	:  std_logic;
	signal avs_read 	:  std_logic;
	signal avs_byteenable  	:  std_logic_vector(3 downto 0);
	signal avs_debugaccess 	:  std_logic;
	signal avs_intr  	:  std_logic := '0';


	signal next_addr_reg, addr_reg : std_logic_vector(3 downto 0);
	
	signal drive0_igbt_err_in : std_logic;
	signal locked : std_logic := '0';
	signal clk_50,clk_80 : std_logic;
	
	signal overcurrent0 : std_logic;
	signal overvoltage : std_logic;
	signal undervoltage : std_logic;
	signal chopper : std_logic;
	signal drive0_adc_irq : std_logic;

	signal pfc_control : std_logic_vector(1 downto 0);
begin
	pll_inst : component altpll_patmos port map	(
		inclk0	=> inclk0,
		c0			=> clk_50,
		c1			=> clk_80,
		c2			=> adc_clk,
		locked	=> locked
	);
	sys_adc_clk <= adc_clk;

	-- we use a PLL
	-- clk_int <= clk;

	--
	--	internal reset generation
	--	should include the PLL lock signal
	--
--	process(clk_50)
--	begin
--		if rising_edge(clk_50) then
--			if (res_50_cnt /= "111") then
--				res_50_cnt <= res_50_cnt + 1;
--			end if;
--			res_50_reg1 <= not res_50_cnt(0) or not res_50_cnt(1) or not res_50_cnt(2);
--			res_50_reg2 <= res_50_reg1;
--			reset_50  <= res_50_reg2;
--		end if;
--	end process;
--	reset_50_n <= not reset_50;
--
--	process(clk_80)
--	begin
--		if rising_edge(clk_80) then
--			if (res_80_cnt /= "111") then
--				res_80_cnt <= res_80_cnt + 1;
--			end if;
--			res_80_reg1 <= not res_80_cnt(0) or not res_80_cnt(1) or not res_80_cnt(2);
--			res_80_reg2 <= res_80_reg1;
--			reset_80  <= res_80_reg2;
--		end if;
--	end process;
--	reset_80_n <= not reset_80;

	process(clk_50)
	begin
		if rising_edge(clk_50) then
			res_50_reg1 <= locked;
			res_50_reg2 <= res_50_reg1;
			reset_50_n  <= res_50_reg2;
		end if;
	end process;
	reset_50 <= not reset_50_n;

	process(clk_80)
	begin
		if rising_edge(clk_80) then
			res_80_reg1 <= locked;
			res_80_reg2 <= res_80_reg1;
			reset_80_n  <= res_80_reg2;
		end if;
	end process;
	reset_80 <= not reset_80_n;
	
    -- tristate output to ssram
    process(sram_out_dout_ena, sram_out_dout)
    begin
      if sram_out_dout_ena='1' then
        SRAM_DQ <= sram_out_dout;
      else
        SRAM_DQ <= (others => 'Z');
      end if;
    end process;

    comp : Patmos port map(clk_80, reset_80,
           open, open, open, open, open,
           (others => '0'), (others => '0'), '0',
           open, open, open, open,
           (others => '0'), (others => '0'),
           open,
           (others => '0'),
           oUartPins_txd, iUartPins_rxd,
			  avs_waitrequest, avs_readdata, avs_readdatavalid, avs_burstcount(0), avs_writedata,
			  avs_address, avs_write, avs_read, avs_byteenable, avs_debugaccess, avs_intr,
           oSRAM_A, sram_out_dout_ena, SRAM_DQ, sram_out_dout, oSRAM_CE_N, oSRAM_OE_N, oSRAM_WE_N, oSRAM_LB_N, oSRAM_UB_N);
	 
	 drive0_igbt_err_in <= not drive0_sm_igbt_err;
	
	drive0 : component DOC_Axis_Periphs_patmos
	port map (
		drive0_adc_sync_dat_u           => drive0_adc_Sync_Dat_U,           --              drive0_adc.sync_dat_u
		drive0_adc_sync_dat_w           => drive0_adc_Sync_Dat_W,           --                        .sync_dat_w
		drive0_adc_overcurrent          => overcurrent0,          --                        .overcurrent
		drive0_pwm_carrier              => open,              --              drive0_pwm.carrier
		drive0_pwm_carrier_latch        => open,        --                        .carrier_latch
		drive0_pwm_encoder_strobe_n     => open,     --                        .encoder_strobe_n
		drive0_pwm_u_h                  => drive0_pwm_u_h,                  --                        .u_h
		drive0_pwm_u_l                  => drive0_pwm_u_l,                  --                        .u_l
		drive0_pwm_v_h                  => drive0_pwm_v_h,                  --                        .v_h
		drive0_pwm_v_l                  => drive0_pwm_v_l,                  --                        .v_l
		drive0_pwm_w_h                  => drive0_pwm_w_h,                  --                        .w_h
		drive0_pwm_w_l                  => drive0_pwm_w_l,                  --                        .w_l
		drive0_sm_overcurrent           => overcurrent0,           --               drive0_sm.overcurrent
		drive0_sm_overvoltage           => overvoltage,           --                        .overvoltage
		drive0_sm_undervoltage          => undervoltage,          --                        .undervoltage
		drive0_sm_chopper               => chopper,               --                        .chopper
		drive0_sm_dc_link_clk_err       => '0',       --                        .dc_link_clk_err
		drive0_sm_igbt_err              => drive0_igbt_err_in,              --                        .igbt_err
		drive0_sm_error_out             => LED(5),             --                        .error_out
		drive0_sm_overcurrent_latch     => LED(0),     --                        .overcurrent_latch
		drive0_sm_overvoltage_latch     => LED(1),     --                        .overvoltage_latch
		drive0_sm_undervoltage_latch    => LED(2),    --                        .undervoltage_latch
		drive0_sm_dc_link_clk_err_latch => open, --                        .dc_link_clk_err_latch
		drive0_sm_igbt_err_latch        => LED(3),        --                        .igbt_err_latch
		drive0_sm_chopper_latch         => LED(4),         --                        .chopper_latch
		drive0_adc_pow_sync_dat_u       => drive0_adc_Sync_Dat_ipow,       --          drive0_adc_pow.sync_dat_u
		drive0_adc_pow_sync_dat_w       => '0',       --                        .sync_dat_w
		drive0_adc_pow_overcurrent      => open,      --                        .overcurrent
		clk_adc_in_clk                  => sys_adc_feedback_clk,                  --              clk_adc_in.clk
		drive0_doc_pwm_sync_out_export  => open,  -- drive0_doc_pwm_sync_out.export
		drive0_doc_pwm_sync_in_export   => '0',   --  drive0_doc_pwm_sync_in.export
		drive0_doc_adc_irq_irq          => drive0_adc_irq,          --      drive0_doc_adc_irq.irq
		drive0_doc_adc_pow_irq_irq      => open,      --  drive0_doc_adc_pow_irq.irq
		avs_periph_slave_waitrequest    => drive0_avs_waitrequest,    --        avs_periph_slave.waitrequest
		avs_periph_slave_readdata       => drive0_avs_readdata,       --                        .readdata
		avs_periph_slave_readdatavalid  => drive0_avs_readdatavalid,  --                        .readdatavalid
		avs_periph_slave_burstcount     => drive0_avs_burstcount,     --                        .burstcount
		avs_periph_slave_writedata      => drive0_avs_writedata,      --                        .writedata
		avs_periph_slave_address        => drive0_avs_address,        --                        .address
		avs_periph_slave_write          => drive0_avs_write,          --                        .write
		avs_periph_slave_read           => drive0_avs_read,           --                        .read
		avs_periph_slave_byteenable     => drive0_avs_byteenable,     --                        .byteenable
		avs_periph_slave_debugaccess    => drive0_avs_debugaccess,    --                        .debugaccess
		reset_reset_n                   => reset_50_n,                   --                   reset.reset_n
		clk_50_clk                      => clk_50,                      --                  clk_50.clk
		clk_80_clk                      => clk_80,                      --                  clk_80.clk
		reset_80_reset_n                => reset_80_n                 --                reset_80.reset_n
	);

	sys_pfc_en <= pfc_control(0);
	
	monitor : component DOC_Monitor
	port map (
		in_port_to_the_IO_IN_Buttons   => button,   -- io_in_buttons_external_connection.export
		pio_pfc_in_port                => sys_pfc_ld_en & sys_pfc_pfw,                --                           pio_pfc.in_port
		pio_pfc_out_port               => pfc_control,               --                                  .out_port
		dc_link_sync_dat               => dc_link_Sync_Dat_VBUS,               --                           dc_link.sync_dat
		dc_link_dc_link_enable         => '1',         --                                  .dc_link_enable
		dc_link_overvoltage            => overvoltage,            --                                  .overvoltage
		dc_link_undervoltage           => undervoltage,           --                                  .undervoltage
		dc_link_chopper                => chopper,                --                                  .chopper
		dc_link_p_sync_dat             => dc_link_Sync_Dat_ipow,             --                         dc_link_p.sync_dat
		dc_link_p_dc_link_enable       => '1',       --                                  .dc_link_enable
		dc_link_p_overvoltage          => open,          --                                  .overvoltage
		dc_link_p_undervoltage         => open,         --                                  .undervoltage
		dc_link_p_chopper              => open,              --                                  .chopper
		clk_adc_in_clk                 => sys_adc_feedback_clk,                 --                        clk_adc_in.clk
		avs_periph_slave_waitrequest   => monitor_avs_waitrequest,   --                  avs_periph_slave.waitrequest
		avs_periph_slave_readdata      => monitor_avs_readdata,      --                                  .readdata
		avs_periph_slave_readdatavalid => monitor_avs_readdatavalid, --                                  .readdatavalid
		avs_periph_slave_burstcount    => monitor_avs_burstcount,    --                                  .burstcount
		avs_periph_slave_writedata     => monitor_avs_writedata,     --                                  .writedata
		avs_periph_slave_address       => monitor_avs_address,       --                                  .address
		avs_periph_slave_write         => monitor_avs_write,         --                                  .write
		avs_periph_slave_read          => monitor_avs_read,          --                                  .read
		avs_periph_slave_byteenable    => monitor_avs_byteenable,    --                                  .byteenable
		avs_periph_slave_debugaccess   => monitor_avs_debugaccess,   --                                  .debugaccess
		clk_80_clk                     => clk_80,                        --                               clk.clk
		reset_80_reset_n               => reset_80_n,                  --                             reset.reset_n
		clk_50_clk                     => clk_50,                      --                             clk_0.clk
		reset_50_reset_n               => reset_50_n                 --                           reset_0.reset_n
	);

	
	avs_intr <= drive0_adc_irq;

	addr_mux : process(avs_burstcount,avs_writedata,avs_byteenable,avs_debugaccess,avs_address,
		avs_read,avs_write,drive0_avs_waitrequest,drive0_avs_readdata,drive0_avs_readdatavalid,
		monitor_avs_waitrequest,monitor_avs_readdata,monitor_avs_readdatavalid)
	begin
		drive0_avs_burstcount <= avs_burstcount;
		monitor_avs_burstcount <= avs_burstcount;
		drive0_avs_writedata <= avs_writedata;
		monitor_avs_writedata <= avs_writedata;

		drive0_avs_byteenable <= avs_byteenable;
		monitor_avs_byteenable <= avs_byteenable;
		drive0_avs_debugaccess <= avs_debugaccess;
		monitor_avs_debugaccess <= avs_debugaccess;
		
		avs_waitrequest	<= '0';
		avs_readdata	<= (others => '0');
		avs_readdatavalid	<= '0';

		
		monitor_avs_address <= avs_address(11 downto 0);
		monitor_avs_write <= '0';
		monitor_avs_read <= '0';
		
		drive0_avs_address <= avs_address(11 downto 0);
		drive0_avs_write <= '0';
		drive0_avs_read <= '0';

		case( avs_address(15 downto 12) ) is
			when X"1" =>
				drive0_avs_write <= avs_write;
				drive0_avs_read <= avs_read;

			when X"0" =>
				monitor_avs_write <= avs_write;
				monitor_avs_read <= avs_read;

			when others =>

		end case ;

		case( addr_reg ) is
			when X"1" =>
				avs_waitrequest	<= drive0_avs_waitrequest;
				avs_readdata	<= drive0_avs_readdata;
				avs_readdatavalid	<= drive0_avs_readdatavalid;
			when X"0" =>
				avs_waitrequest	<= monitor_avs_waitrequest;
				avs_readdata	<= monitor_avs_readdata;
				avs_readdatavalid	<= monitor_avs_readdatavalid;
			when others =>

		end case ;

		next_addr_reg <= addr_reg;
		if avs_write = '1' or avs_read = '1' then
			next_addr_reg <= avs_address(15 downto 12);
		end if;
	end process;

	process(clk_80)
	begin
		if rising_edge(clk_80) then
			addr_reg <= next_addr_reg;
		end if ;
	end process;

	-- Debug signal assignments
	debug_sys_adc_clk <= adc_clk;
	debug_sys_adc_feedback_clk <= sys_adc_feedback_clk;
	debug_dc_link_Sync_Dat_VBUS <= dc_link_Sync_Dat_VBUS;
	debug_drive0_adc_Sync_Dat_U <= drive0_adc_Sync_Dat_U;
	debug_drive0_adc_Sync_Dat_W <= drive0_adc_Sync_Dat_W;
	debug_ground <= '0';
	debug_irg <= drive0_adc_irq;
		
end architecture rtl;

--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- License: Simplified BSD License
--

-- VHDL top level for Patmos in Chisel with on-chip memory.
--
-- Includes some 'magic' VHDL code to generate a reset after FPGA configuration.
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_top is
    port(
        -- Clock and reset
        clk           : in    std_logic;
        --cpu_reset_btn        : in    std_logic;
        -- LEDs
        oLedsPins_led : out   std_logic_vector(7 downto 0);
        --UART
        oUartPins_txd : out   std_logic;
        iUartPins_rxd : in    std_logic;
        -- AAU I2C interface
        oMpuScl       : out   std_logic;
        ioMpuSda      : inout std_logic;
        oMpuAd0       : out   std_logic;
        -- I2C controller interface
        i2c_sda       : inout std_logic;
        i2c_scl       : inout std_logic;
        ad0           : out   std_logic
    );
end entity patmos_top;

architecture rtl of patmos_top is
    component Patmos is
        port(
            clk                         : in  std_logic;
            reset                       : in  std_logic;
            io_comConf_M_Cmd            : out std_logic_vector(2 downto 0);
            io_comConf_M_Addr           : out std_logic_vector(31 downto 0);
            io_comConf_M_Data           : out std_logic_vector(31 downto 0);
            io_comConf_M_ByteEn         : out std_logic_vector(3 downto 0);
            io_comConf_M_RespAccept     : out std_logic;
            io_comConf_S_Resp           : in  std_logic_vector(1 downto 0);
            io_comConf_S_Data           : in  std_logic_vector(31 downto 0);
            io_comConf_S_CmdAccept      : in  std_logic;
            io_comSpm_M_Cmd             : out std_logic_vector(2 downto 0);
            io_comSpm_M_Addr            : out std_logic_vector(31 downto 0);
            io_comSpm_M_Data            : out std_logic_vector(31 downto 0);
            io_comSpm_M_ByteEn          : out std_logic_vector(3 downto 0);
            io_comSpm_S_Resp            : in  std_logic_vector(1 downto 0);
            io_comSpm_S_Data            : in  std_logic_vector(31 downto 0);
            io_ledsPins_led             : out std_logic_vector(7 downto 0);
            io_aauMpuPins_data_0        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_1        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_2        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_3        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_4        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_5        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_6        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_7        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_8        : in  std_logic_vector(31 downto 0);
            io_aauMpuPins_data_9        : in  std_logic_vector(31 downto 0);
            io_i2CInterfacePins_MCmd    : out std_logic_vector(2 downto 0);
            io_i2CInterfacePins_MAddr   : out std_logic_vector(15 downto 0);
            io_i2CInterfacePins_MData   : out std_logic_vector(31 downto 0);
            io_i2CInterfacePins_MByteEn : out std_logic_vector(3 downto 0);
            io_i2CInterfacePins_SResp   : in  std_logic_vector(1 downto 0);
            io_i2CInterfacePins_SData   : in  std_logic_vector(31 downto 0);
            io_uartPins_tx              : out std_logic;
            io_uartPins_rx              : in  std_logic
        );
    end component;

    component imu_mpu is
        port(
            address    : in    std_logic_vector(1 downto 0);
            clk        : in    std_logic;
            reset_n    : in    std_logic;
            readdata_0 : out   std_logic_vector(31 downto 0);
            readdata_1 : out   std_logic_vector(31 downto 0);
            readdata_2 : out   std_logic_vector(31 downto 0);
            readdata_3 : out   std_logic_vector(31 downto 0);
            readdata_4 : out   std_logic_vector(31 downto 0);
            readdata_5 : out   std_logic_vector(31 downto 0);
            readdata_6 : out   std_logic_vector(31 downto 0);
            readdata_7 : out   std_logic_vector(31 downto 0);
            readdata_8 : out   std_logic_vector(31 downto 0);
            readdata_9 : out   std_logic_vector(31 downto 0);
            scl_out    : out   std_logic;
            sda_inout  : inout std_logic
        );
    end component;

    component I2Ccontroller is
        generic(
            OCP_DATA_WIDTH : integer := 32;
            OCP_ADDR_WIDTH : integer := 16;
            input_clk      : INTEGER := 50_000_000; --input clock speed from user logic in Hz
            bus_clk        : INTEGER := 100_000); --speed the i2c bus (scl) will run at in Hz
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

            sda     : INOUT STD_LOGIC;  --serial data output of i2c bus
            scl     : INOUT STD_LOGIC   --serial clock output of i2c bus

        );
		  end component;

        -- DE2-70: 50 MHz clock => 80 MHz
        -- BeMicro: 16 MHz clock => 25.6 MHz
        -- de10-nano: start with the 50 MHz clock input and no PLL
        constant pll_infreq : real    := 50.0;
        constant pll_mult   : natural := 8;
        constant pll_div    : natural := 5;

        signal clk_int : std_logic;

        -- for generation of internal reset
        signal reset_int            : std_logic;
        signal res_reg1, res_reg2 : std_logic;
        signal res_cnt            : unsigned(2 downto 0) := "000"; -- for the simulation

        attribute altera_attribute : string;
        attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

        signal address : std_logic_vector(1 downto 0);
        signal reset_n : std_logic;
        type type_data is array (0 to 9) of std_logic_vector(31 downto 0);
        signal readdata : type_data;
        -- signal scl_out : std_logic;
        -- signal sda_inout : std_logic;

        signal i2CInterfacePins_MCmd    : std_logic_vector(2 downto 0);
        signal i2CInterfacePins_MAddr   : std_logic_vector(15 downto 0);
        signal i2CInterfacePins_MData   : std_logic_vector(31 downto 0);
        signal i2CInterfacePins_MByteEn : std_logic_vector(3 downto 0);
        signal i2CInterfacePins_SResp   : std_logic_vector(1 downto 0);
        signal i2CInterfacePins_SData   : std_logic_vector(31 downto 0);

    begin
        --	pll_inst : entity work.pll generic map(
        --			input_freq  => pll_infreq,
        --			multiply_by => pll_mult,
        --			divide_by   => pll_div
        --		)
        --		port map(
        --			inclk0 => clk,
        --			c0     => clk_int
        --		);
        -- we use a PLL
        clk_int <= clk;

        --
        --	internal reset generation
        --	should include the PLL lock signal
        --
        process(clk_int)
        begin
            if rising_edge(clk_int) then
                if (res_cnt /= "111") then
                    res_cnt <= res_cnt + 1;
                end if;
                res_reg1 <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
                res_reg2 <= res_reg1;
                reset_int  <= res_reg2;
            end if;
        end process;

        oMpuAd0 <= '0';
        reset_n <= not reset_int;

        ad0 <= '0';

        imu_mpu_inst_0 : imu_mpu port map(
                address    => address,
                clk        => clk_int,
                reset_n    => reset_n,
                readdata_0 => readdata(0),
                readdata_1 => readdata(1),
                readdata_2 => readdata(2),
                readdata_3 => readdata(3),
                readdata_4 => readdata(4),
                readdata_5 => readdata(5),
                readdata_6 => readdata(6),
                readdata_7 => readdata(7),
                readdata_8 => readdata(8),
                readdata_9 => readdata(9),
                scl_out    => oMpuScl,
                sda_inout  => ioMpuSda
            );

        Patmos_inst_0 : Patmos port map(
                clk                         => clk_int,
                reset                       => reset_int,
                io_comConf_M_Cmd            => open,
                io_comConf_M_Addr           => open,
                io_comConf_M_Data           => open,
                io_comConf_M_ByteEn         => open,
                io_comConf_M_RespAccept     => open,
                io_comConf_S_Resp           => (others => '0'),
                io_comConf_S_Data           => (others => '0'),
                io_comConf_S_CmdAccept      => '0',
                io_comSpm_M_Cmd             => open,
                io_comSpm_M_Addr            => open,
                io_comSpm_M_Data            => open,
                io_comSpm_M_ByteEn          => open,
                io_comSpm_S_Resp            => (others => '0'),
                io_comSpm_S_Data            => (others => '0'),
                io_ledsPins_led             => oLedsPins_led,
                io_aauMpuPins_data_0        => readdata(0),
                io_aauMpuPins_data_1        => readdata(1),
                io_aauMpuPins_data_2        => readdata(2),
                io_aauMpuPins_data_3        => readdata(3),
                io_aauMpuPins_data_4        => readdata(4),
                io_aauMpuPins_data_5        => readdata(5),
                io_aauMpuPins_data_6        => readdata(6),
                io_aauMpuPins_data_7        => readdata(7),
                io_aauMpuPins_data_8        => readdata(8),
                io_aauMpuPins_data_9        => readdata(9),
                io_i2CInterfacePins_MCmd    => i2CInterfacePins_MCmd,
                io_i2CInterfacePins_MAddr   => i2CInterfacePins_MAddr,
                io_i2CInterfacePins_MData   => i2CInterfacePins_MData,
                io_i2CInterfacePins_MByteEn => i2CInterfacePins_MByteEn,
                io_i2CInterfacePins_SResp   => i2CInterfacePins_SResp,
                io_i2CInterfacePins_SData   => i2CInterfacePins_SData,
                io_uartPins_tx              => oUartPins_txd,
                io_uartPins_rx              => iUartPins_rxd
            );

        I2Ccontroller_inst_0 : I2Ccontroller
            generic map(
                OCP_DATA_WIDTH => 32,
                OCP_ADDR_WIDTH => 16,
                input_clk      => 50_000_000, --input clock speed from user logic in Hz
                bus_clk        => 100_000) --speed the i2c bus (scl) will run at in Hz
            port map(
                clk     => clk_int,
                reset   => reset_int,

                -- OCP IN (slave)
                MCmd    => i2CInterfacePins_MCmd,
                MAddr   => i2CInterfacePins_MAddr,
                MData   => i2CInterfacePins_MData,
                MByteEn => i2CInterfacePins_MByteEn,
                SResp   => i2CInterfacePins_SResp,
                SData   => i2CInterfacePins_SData,
                sda     => i2c_sda,
                scl     => i2c_scl
            );

    end architecture rtl;

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
    port (
        -- Clock and reset
        clk                  : in    std_logic;
        --cpu_reset_btn      : in    std_logic;
        -- LEDs
        oLedsPins_led        : out   std_logic_vector(7 downto 0);
        --UART
        uart_1_txd           : out   std_logic;
        uart_1_rxd           : in    std_logic;
        --UART2
        uart_2_txd           : out   std_logic;
        uart_2_rxd           : in    std_logic;
        --UART3
        uart_3_txd           : out   std_logic;
        uart_3_rxd           : in    std_logic;
        -- AAU I2C interface
        oMpuScl              : out   std_logic;
        ioMpuSda             : inout std_logic;
        oMpuAd0              : out   std_logic;
        -- I2C controller interface
        i2c_sda              : inout std_logic;
        i2c_scl              : inout std_logic;
        ad0                  : out   std_logic;
        -- Actuator and propdrive OUT
        pwm_measurment_input : in    std_logic_vector(3 downto 0);
        propdrive_out_port   : out   std_logic_vector(3 downto 0);
        --SPI Master interface
        SPIMaster_miso       : in    std_logic;
        SPIMaster_mosi       : out   std_logic;
        SPIMaster_nSS        : out   std_logic;
        SPIMaster_sclk       : out   std_logic;
        --test
        test_SPIMaster_miso  : out   std_logic;
        test_SPIMaster_mosi  : out   std_logic;
        test_SPIMaster_nSS   : out   std_logic;
        test_SPIMaster_sclk  : out   std_logic;
        --DDR3
        HPS_DDR3_ADDR        : out   std_logic_vector(14 downto 0);
        HPS_DDR3_BA          : out   std_logic_vector(2 downto 0);
        HPS_DDR3_CAS_N       : out   std_logic;
        HPS_DDR3_CKE         : out   std_logic;
        HPS_DDR3_CK_N        : out   std_logic;
        HPS_DDR3_CK_P        : out   std_logic;
        HPS_DDR3_CS_N        : out   std_logic;
        HPS_DDR3_DM          : out   std_logic_vector(3 downto 0);
        HPS_DDR3_DQ          : inout std_logic_vector(31 downto 0);
        HPS_DDR3_DQS_N       : inout std_logic_vector(3 downto 0);
        HPS_DDR3_DQS_P       : inout std_logic_vector(3 downto 0);
        HPS_DDR3_ODT         : out   std_logic;
        HPS_DDR3_RAS_N       : out   std_logic;
        HPS_DDR3_RESET_N     : out   std_logic;
        HPS_DDR3_RZQ         : in    std_logic;
        HPS_DDR3_WE_N        : out   std_logic
 );
end entity patmos_top;
architecture rtl of patmos_top is
    component Patmos is
        port (
            clock                     : in    std_logic;
            reset                     : in    std_logic;
            io_Leds_led               : out   std_logic_vector(7 downto 0);
            -- io_AauMpu_data_0       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_1       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_2       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_3       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_4       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_5       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_6       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_7       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_8       : in    std_logic_vector(31 downto 0);
            -- io_AauMpu_data_9       : in    std_logic_vector(31 downto 0);
            io_I2CMaster_sdaI         : in    std_logic;
            io_I2CMaster_sdaO         : out   std_logic;
            io_I2CMaster_sclI         : in    std_logic;
            io_I2CMaster_sclO         : out   std_logic;
            io_Actuators_MCmd         : out   std_logic_vector(2 downto 0);
            io_Actuators_MAddr        : out   std_logic_vector(15 downto 0);
            io_Actuators_MData        : out   std_logic_vector(31 downto 0);
            io_Actuators_MByteEn      : out   std_logic_vector(3 downto 0);
            io_Actuators_SResp        : in    std_logic_vector(1 downto 0);
            io_Actuators_SData        : in    std_logic_vector(31 downto 0);
            io_UartCmp_rx             : in    std_logic;
            io_UartCmp_tx             : out   std_logic;
            io_Uart_rx                : in    std_logic;
            io_Uart_tx                : out   std_logic;
            io_Uart_1_rx              : in    std_logic;
            io_Uart_1_tx              : out   std_logic;
            io_SPIMaster_miso         : in    std_logic;
            io_SPIMaster_mosi         : out   std_logic;
            io_SPIMaster_nSS          : out   std_logic;
            io_SPIMaster_sclk         : out   std_logic;
            --DDR3
            io_DDR3Bridge_mem_a       : out   std_logic_vector(14 downto 0);
            io_DDR3Bridge_mem_ba      : out   std_logic_vector(2 downto 0);
            io_DDR3Bridge_mem_ck      : out   std_logic;
            io_DDR3Bridge_mem_ck_n    : out   std_logic;
            io_DDR3Bridge_mem_cke     : out   std_logic;
            io_DDR3Bridge_mem_cs_n    : out   std_logic;
            io_DDR3Bridge_mem_ras_n   : out   std_logic;
            io_DDR3Bridge_mem_cas_n   : out   std_logic;
            io_DDR3Bridge_mem_we_n    : out   std_logic;
            io_DDR3Bridge_mem_reset_n : out   std_logic;
            io_DDR3Bridge_mem_dq      : inout std_logic_vector(31 downto 0);
            io_DDR3Bridge_mem_dqs     : inout std_logic_vector(3 downto 0);
            io_DDR3Bridge_mem_dqs_n   : inout std_logic_vector(3 downto 0);
            io_DDR3Bridge_mem_odt     : out   std_logic;
            io_DDR3Bridge_mem_dm      : out   std_logic_vector(3 downto 0);
            io_DDR3Bridge_oct_rzqin   : in    std_logic
        );
    end component;

    component imu_mpu is
        port (
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

    component Actuators_PropDrive is
        generic (
            OCP_DATA_WIDTH   : natural := 32;
            OCP_ADDR_WIDTH   : natural := 16;
            ACTUATOR_NUMBER  : natural := 4;
            PROPDRIVE_NUMBER : natural := 4
        );
        port (
            clk                  : in  std_logic;
            reset                : in  std_logic;

            -- OCP IN (slave)
            MCmd                 : in  std_logic_vector(2 downto 0);
            MAddr                : in  std_logic_vector(OCP_ADDR_WIDTH - 1 downto 0);
            MData                : in  std_logic_vector(OCP_DATA_WIDTH - 1 downto 0);
            MByteEn              : in  std_logic_vector(3 downto 0);
            SResp                : out std_logic_vector(1 downto 0);
            SData                : out std_logic_vector(OCP_DATA_WIDTH - 1 downto 0);

            -- Actuator and propdrive OUT
            -- actuator_out_port : out std_logic_vector(ACTUATOR_NUMBER-1 downto 0);
            pwm_measurment_input : in  std_logic_vector(ACTUATOR_NUMBER - 1 downto 0);
            propdrive_out_port   : out std_logic_vector(PROPDRIVE_NUMBER - 1 downto 0)
        );
    end component;

    -- DE2-70: 50 MHz clock => 80 MHz
    -- BeMicro: 16 MHz clock => 25.6 MHz
    -- de10-nano: start with the 50 MHz clock input and no PLL
    constant pll_infreq                   : real    := 50.0;
    constant pll_mult                     : natural := 8;
    constant pll_div                      : natural := 5;

    signal clk_int                        : std_logic;

    -- for generation of internal reset
    signal reset_int                      : std_logic;
    signal res_reg1, res_reg2             : std_logic;
    signal res_cnt                        : unsigned(2 downto 0) := "000"; -- for the simulation

    attribute altera_attribute            : string;
    attribute altera_attribute of res_cnt : signal is "POWER_UP_LEVEL=LOW";

    signal address                        : std_logic_vector(1 downto 0);
    signal reset_n                        : std_logic;
    type type_data is array (0 to 9) of std_logic_vector(31 downto 0);
    signal readdata              : type_data;
    -- signal scl_out : std_logic;
    -- signal sda_inout : std_logic;

    signal sdaI                  : std_logic;
    signal sdaO                  : std_logic;
    signal sclI                  : std_logic;
    signal sclO                  : std_logic;

    signal actuatorsPins_MCmd    : std_logic_vector(2 downto 0);
    signal actuatorsPins_MAddr   : std_logic_vector(15 downto 0);
    signal actuatorsPins_MData   : std_logic_vector(31 downto 0);
    signal actuatorsPins_MByteEn : std_logic_vector(3 downto 0);
    signal actuatorsPins_SResp   : std_logic_vector(1 downto 0);
    signal actuatorsPins_SData   : std_logic_vector(31 downto 0);

    signal mosi                  : std_logic;
    signal miso                  : std_logic;
    signal ss                    : std_logic;
    signal sclk                  : std_logic;

begin
    -- pll_inst :entity work.pll generic map(
    -- input_freq => pll_infreq,
    -- multiply_by => pll_mult,
    -- divide_by => pll_div
    -- )
    -- port map(
    -- inclk0 => clk,
    -- c0 => clk_int
    -- );
    -- we use a PLL
    clk_int <= clk;

    --
    -- internal reset generation
    -- should include the PLL lock signal
    --
    process (clk_int)
    begin
        if rising_edge(clk_int) then
            if (res_cnt /= "111") then
                res_cnt <= res_cnt + 1;
            end if;
            res_reg1  <= not res_cnt(0) or not res_cnt(1) or not res_cnt(2);
            res_reg2  <= res_reg1;
            reset_int <= res_reg2;
        end if;
    end process;

    oMpuAd0 <= '0';
    reset_n <= not reset_int;

    ad0     <= '0';

    imu_mpu_inst_0 : imu_mpu
    port map
    (
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

    Patmos_inst_0 : Patmos
    port map
    (
        clock                => clk_int,
        reset                => reset_int,
        io_Leds_led          => oLedsPins_led,
        -- io_AauMpu_data_0 => readdata(0),
        -- io_AauMpu_data_1 => readdata(1),
        -- io_AauMpu_data_2 => readdata(2),
        -- io_AauMpu_data_3 => readdata(3),
        -- io_AauMpu_data_4 => readdata(4),
        -- io_AauMpu_data_5 => readdata(5),
        -- io_AauMpu_data_6 => readdata(6),
        -- io_AauMpu_data_7 => readdata(7),
        -- io_AauMpu_data_8 => readdata(8),
        -- io_AauMpu_data_9 => readdata(9),
        io_I2CMaster_sdaI    => sdaI,
        io_I2CMaster_sdaO    => sdaO,
        io_I2CMaster_sclI    => sclI,
        io_I2CMaster_sclO    => sclO,
        io_Actuators_MCmd    => actuatorsPins_MCmd,
        io_Actuators_MAddr   => actuatorsPins_MAddr,
        io_Actuators_MData   => actuatorsPins_MData,
        io_Actuators_MByteEn => actuatorsPins_MByteEn,
        io_Actuators_SResp   => actuatorsPins_SResp,
        io_Actuators_SData   => actuatorsPins_SData,
        io_UartCmp_rx        => uart_1_rxd,
        io_UartCmp_tx        => uart_1_txd,
        io_Uart_rx           => uart_2_rxd,
        io_Uart_tx           => uart_2_txd,
        io_Uart_1_rx         => uart_3_rxd,
        io_Uart_1_tx         => uart_3_txd,
        io_SPIMaster_miso    => miso,
        io_SPIMaster_mosi    => mosi,
        io_SPIMaster_nSS     => ss,
        io_SPIMaster_sclk    => sclk,
        --DDR3
        io_DDR3Bridge_mem_a       => HPS_DDR3_ADDR,
        io_DDR3Bridge_mem_ba      => HPS_DDR3_BA,
        io_DDR3Bridge_mem_ck      => HPS_DDR3_CK_P,
        io_DDR3Bridge_mem_ck_n    => HPS_DDR3_CK_N,
        io_DDR3Bridge_mem_cke     => HPS_DDR3_CKE,
        io_DDR3Bridge_mem_cs_n    => HPS_DDR3_CS_N,
        io_DDR3Bridge_mem_ras_n   => HPS_DDR3_RAS_N,
        io_DDR3Bridge_mem_cas_n   => HPS_DDR3_CAS_N,
        io_DDR3Bridge_mem_we_n    => HPS_DDR3_WE_N,
        io_DDR3Bridge_mem_reset_n => HPS_DDR3_RESET_N,
        io_DDR3Bridge_mem_dq      => HPS_DDR3_DQ,
        io_DDR3Bridge_mem_dqs     => HPS_DDR3_DQS_P,
        io_DDR3Bridge_mem_dqs_n   => HPS_DDR3_DQS_N,
        io_DDR3Bridge_mem_odt     => HPS_DDR3_ODT,
        io_DDR3Bridge_mem_dm      => HPS_DDR3_DM,
        io_DDR3Bridge_oct_rzqin   => HPS_DDR3_RZQ
    );

    Actuators_PropDrive_inst_0 : Actuators_PropDrive
    generic map
    (
        OCP_DATA_WIDTH   => 32,
        OCP_ADDR_WIDTH   => 16,
        ACTUATOR_NUMBER  => 4,
        PROPDRIVE_NUMBER => 4
    )
    port map
    (
        clk                  => clk,
        reset                => reset_int,
        -- OCP IN (slave)
        MCmd                 => actuatorsPins_MCmd,
        MAddr                => actuatorsPins_MAddr,
        MData                => actuatorsPins_MData,
        MByteEn              => actuatorsPins_MByteEn,
        SResp                => actuatorsPins_SResp,
        SData                => actuatorsPins_SData,
        -- Actuator and propdrive OUT
        pwm_measurment_input => pwm_measurment_input,
        propdrive_out_port   => propdrive_out_port
    );

    test_SPIMaster_miso <= miso;
    test_SPIMaster_mosi <= mosi;
    test_SPIMaster_nSS  <= ss;
    test_SPIMaster_sclk <= sclk;

    miso                <= SPIMaster_miso;
    SPIMaster_mosi      <= mosi;
    SPIMaster_nSS       <= ss;
    SPIMaster_sclk      <= sclk;

    sdaI                <= i2c_sda;
    i2c_sda             <= '0' when sdaO = '0' else 'Z';
    sclI                <= i2c_scl;
    i2c_scl             <= '0' when sclO = '0' else 'Z';

end architecture rtl;
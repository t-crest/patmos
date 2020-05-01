
library std;
use std.textio.all;

library modelsim_lib;
use modelsim_lib.util.all;

library work;
use work.test.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity patmos_tb_sram16 is

end entity;

architecture struct of patmos_tb_sram16 is
	signal clk : std_logic;
	signal reset : std_logic;
	signal led : std_logic_vector(8 downto 0);
	constant PERIOD : time := 10 ns;
	constant RESET_TIME : time := 40 ns;
    signal io_ramOut_addr : std_logic_vector(19 downto 0);
    signal io_ramOut_dout_ena : std_logic;
    signal io_ramIn_din :std_logic_vector(15 downto 0);
    signal io_ramOut_dout : std_logic_vector(15 downto 0);
    signal io_ramOut_nce : std_logic;
    signal io_ramOut_noe : std_logic;
    signal io_ramOut_nwe : std_logic;
    signal io_ramOut_nlb : std_logic;
    signal io_ramOut_nub : std_logic;
    signal pull_down : std_logic;
	signal io_ramInout_d : std_logic_vector(15 downto 0);
	signal io_ramIn_din_reg : std_logic_vector(15 downto 0);
	signal pat_uart_tx_reg : std_logic_vector(7 downto 0);
	signal pat_uart_tx_status_reg : std_logic_vector(0 downto 0);

    file OUTPUT: TEXT open WRITE_MODE is "STD_OUTPUT";

    component Patmos is
        port(
            clock           : in  std_logic;
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
            io_ledsPins_led : out std_logic_vector(8 downto 0);
            io_uartPins_tx  : out std_logic;
            io_uartPins_rx  : in  std_logic;

            io_sramCtrlPins_ramOut_addr : out std_logic_vector(19 downto 0);
            io_sramCtrlPins_ramOut_dout_ena : out std_logic;
            io_sramCtrlPins_ramIn_din : in std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_dout : out std_logic_vector(15 downto 0);
            io_sramCtrlPins_ramOut_nce : out std_logic;
            io_sramCtrlPins_ramOut_noe : out std_logic;
            io_sramCtrlPins_ramOut_nwe : out std_logic;
            io_sramCtrlPins_ramOut_nlb : out std_logic;
            io_sramCtrlPins_ramOut_nub : out std_logic

        );
    end component;

begin

	patmos_inst : Patmos port map(
		clk	=>	clk,
		reset	=>	reset,
        io_comConf_M_Cmd => open,
        io_comConf_M_Addr => open,
        io_comConf_M_Data => open,
        io_comConf_M_ByteEn => open,
        io_comConf_M_RespAccept => open,
        io_comConf_S_Resp => (others => '0'),
        io_comConf_S_Data => (others => '0'),
        io_comConf_S_CmdAccept => '0',
        io_comSpm_M_Cmd => open,
        io_comSpm_M_Addr => open,
        io_comSpm_M_Data => open,
        io_comSpm_M_ByteEn => open,
        io_comSpm_S_Resp => (others => '0'),
        io_comSpm_S_Data => (others => '0'),
        io_cpuInfoPins_id => (others => '0'),
        io_uartPins_tx => open,
        io_uartPins_rx => '0',
		io_sramCtrlPins_ramOut_addr	=>	io_ramOut_addr,
		io_sramCtrlPins_ramOut_dout_ena	=>	io_ramOut_dout_ena,
        io_sramCtrlPins_ramIn_din   =>  io_ramIn_din,
        io_sramCtrlPins_ramOut_dout =>  io_ramOut_dout,
        io_sramCtrlPins_ramOut_nce  =>  io_ramOut_nce,
		io_sramCtrlPins_ramOut_noe	=>	io_ramOut_noe,
		io_sramCtrlPins_ramOut_nwe	=>	io_ramOut_nwe,
		io_sramCtrlPins_ramOut_nlb  =>  io_ramOut_nlb,
        io_sramCtrlPins_ramOut_nub  =>  io_ramOut_nub);


    clock_gen(clk,PERIOD);
    reset_gen(reset,RESET_TIME);

    pat_uart_spy : process
        variable buf: LINE;
        constant CORE_ID : STRING (9 downto 1):="PAT: at: ";
        variable i : integer := 0;
    begin
        init_signal_spy("/patmos_tb_sram16/patmos_inst/core/iocomp/Uart/tx_empty","/patmos_tb_sram16/pat_uart_tx_status_reg");
        init_signal_spy("/patmos_tb_sram16/patmos_inst/core/iocomp/Uart/tx_data","/patmos_tb_sram16/pat_uart_tx_reg");
        write(buf,CORE_ID);
        loop
            wait until falling_edge(pat_uart_tx_status_reg(0));
            if i = 0 then
                write(buf,time'image(NOW) & " : ");
                --write(buf,real'image(real(NOW/time'val(1000000))/1000.0) & " us : ");
            end if;
            write(buf,character'val(to_integer(unsigned(pat_uart_tx_reg))));
            i := i + 1;
            --writeline(OUTPUT,buf);
            if to_integer(unsigned(pat_uart_tx_reg)) = 10 then
                writeline(OUTPUT,buf);
                i := 0;
                write(buf,CORE_ID);
            end if;
        end loop;
    end process ; -- pat_uart_spy


    -- Add uart ticker to increase the UART speed to reduce simulation time
    baud_inc : process
    begin
        loop
            wait until rising_edge(clk);
            signal_force("/patmos_tb_sram16/patmos_inst/core/iocomp/Uart/tx_baud_tick", "1", 0 ns, freeze, open, 0);
            wait until rising_edge(clk);
            signal_force("/patmos_tb_sram16/patmos_inst/core/iocomp/Uart/tx_baud_tick", "0", 0 ns, freeze, open, 0);
            wait for 3*PERIOD;
        end loop;
    end process ; -- baud_inc


    -- capture input from ssram on falling clk edge
    process(clk, reset)
    begin
        if reset='1' then
            --io_ramIn_din_reg <= (others => '0');
            io_ramIn_din <= (others => '0');
        elsif falling_edge(clk) then
            --io_ramIn_din_reg <= io_ramInout_d;
            io_ramIn_din <= io_ramInout_d;
        end if;
    end process;

    -- tristate output to ssram
    process(io_ramOut_dout_ena, io_ramOut_dout)
    begin
        if io_ramOut_dout_ena='1' then
            io_ramInout_d <= io_ramOut_dout;
        else
            io_ramInout_d <= (others => 'Z');
        end if;
    end process;

    -- input of tristate on positive clk edge
    --process(clk)
    --begin
    --    if rising_edge(clk) then
    --        io_ramIn_din <= io_ramIn_din_reg;
    --    end if;
    --end process;

    sram : entity work.cy7c10612dv33 port map(
        CE_b  => io_ramOut_nce,
        WE_b  => io_ramOut_nwe,
        OE_b  => io_ramOut_noe,
        BHE_b => io_ramOut_nub,
        BLE_b => io_ramOut_nlb,
        A     => io_ramOut_addr,
        DQ    => io_ramInout_d);


end struct;

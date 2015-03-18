
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
	signal pat_uart_tx_status_reg : std_logic;

    file OUTPUT: TEXT open WRITE_MODE is "STD_OUTPUT";

    component patmos_top is
        port(
            clk : in  std_logic;
            oLedsPins_led : out std_logic_vector(8 downto 0);
            iKeysPins_key : in std_logic_vector(3 downto 0);
            oUartPins_txd : out std_logic;
            iUartPins_rxd : in  std_logic;
            u_h : out std_logic;
            u_l : out std_logic;
            v_h : out std_logic;
            v_l : out std_logic;
            w_h : out std_logic;
            w_l : out std_logic;
            oSRAM_A : out std_logic_vector(19 downto 0);
            SRAM_DQ : inout std_logic_vector(15 downto 0);
            oSRAM_CE_N : out std_logic;
            oSRAM_OE_N : out std_logic;
            oSRAM_WE_N : out std_logic;
            oSRAM_LB_N : out std_logic;
            oSRAM_UB_N : out std_logic

        );
    end component;

begin

	patmos_inst : patmos_top port map(
		clk	=>	clk,
		oLedsPins_led => open,
        iKeysPins_key => (others => '0'),
        oUartPins_txd => open,
        iUartPins_rxd => '0',
        u_h => open,
        u_l => open,
        v_h => open,
        v_l => open,
        w_h => open,
        w_l => open,
        oSRAM_A =>  io_ramOut_addr,
        SRAM_DQ => io_ramInout_d,
        oSRAM_CE_N =>  io_ramOut_nce,
        oSRAM_OE_N =>  io_ramOut_noe,
        oSRAM_WE_N =>  io_ramOut_nwe,
        oSRAM_LB_N =>  io_ramOut_nlb,
        oSRAM_UB_N =>  io_ramOut_nub
        );


    clock_gen(clk,PERIOD);

    pat_uart_spy : process
        variable buf: LINE;
        constant CORE_ID : STRING (9 downto 1):="PAT: at: ";
        variable i : integer := 0;
    begin
        init_signal_spy("/patmos_tb_sram16/patmos_inst/comp/core/iocomp/Uart/txQueue/io_enq_valid","/patmos_tb_sram16/pat_uart_tx_status_reg");
        init_signal_spy("/patmos_tb_sram16/patmos_inst/comp/core/iocomp/Uart/txQueue/io_enq_bits","/patmos_tb_sram16/pat_uart_tx_reg");
        write(buf,CORE_ID);
        loop
            wait until falling_edge(pat_uart_tx_status_reg);
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
            signal_force("/patmos_tb_sram16/patmos_inst/comp/core/iocomp/Uart/tx_baud_tick", "1", 0 ns, freeze, open, 0);
            wait until rising_edge(clk);
            signal_force("/patmos_tb_sram16/patmos_inst/comp/core/iocomp/Uart/tx_baud_tick", "0", 0 ns, freeze, open, 0);
            wait for 3*PERIOD;
        end loop;
    end process ; -- baud_inc

    sram : entity work.cy7c10612dv33 port map(
        CE_b  => io_ramOut_nce,
        WE_b  => io_ramOut_nwe,
        OE_b  => io_ramOut_noe,
        BHE_b => io_ramOut_nub,
        BLE_b => io_ramOut_nlb,
        A     => io_ramOut_addr,
        DQ    => io_ramInout_d);


end struct;

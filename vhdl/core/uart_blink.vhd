library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

entity uart_blink is
  generic (period : time := 50 ns);

  -- fpga ports
  port (
    clk : in  std_logic;
    led : out std_logic;

    nrts : out std_logic;
    ncts : in  std_logic;
    rxd  : in  std_logic;
    txd  : out std_logic
    );

end uart_blink;


architecture bench of uart_blink is
  
  component sc_uart                     --  Declaration of uart driver
    generic (addr_bits : integer := 2;
             clk_freq  : integer := 50000000;
             baud_rate : integer := 115200;
             txf_depth : integer := 16; txf_thres : integer := 8;
             rxf_depth : integer := 16; rxf_thres : integer := 8);
    port(
      clk   : in std_logic;
      reset : in std_logic;

      address : in  std_logic_vector(1 downto 0);
      wr_data : in  std_logic_vector(31 downto 0);
      rd, wr  : in  std_logic;
      rd_data : out std_logic_vector(31 downto 0);
      rdy_cnt : out unsigned(1 downto 0);
      txd     : out std_logic;
      rxd     : in  std_logic;
      ncts    : in  std_logic;
      nrts    : out std_logic
      );
  end component;

                                        -- Internal signals
  signal address : std_logic_vector(1 downto 0)  := (others => '0');
  signal wr_data : std_logic_vector(31 downto 0) := (others => '0');
  signal rd, wr  : std_logic                     := '0';
  signal rd_data : std_logic_vector(31 downto 0);
  signal rdy_cnt : unsigned(1 downto 0);

  signal reset  : std_logic := '0';
  signal toggle : std_logic := '0';
                               
signal cnt : unsigned(31 downto 0) := (others => '0'); 
                                     
                                     constant CLK_FREQ : integer := 200000000;
  constant BLINK_FREQ : integer := 1;
  constant CNT_MAX    : integer := CLK_FREQ/BLINK_FREQ/2-1;

  signal blink : std_logic;
    
begin
  
  sc_uart_inst : sc_uart port map       -- Maps internal signals to ports
    (
      address => address,
      wr_data => wr_data,
      rd      => rd,
      wr      => wr,
      rd_data => rd_data,
      rdy_cnt => rdy_cnt,
      clk     => clk,
      reset   => reset,
      txd     => txd,
      rxd     => rxd,
      ncts    => '0',
      nrts    => open
      );

  
  process(clk)                        -- blink the led
  begin
    
    if reset = '1' then
      cnt <= (others => '0');
      wr  <= '0'; 
    elsif rising_edge(clk) then
                    if cnt = CNT_MAX then
                      cnt   <= (others => '0');
                      blink <= not blink;
                      wr    <= '1'; 
                    else
                      cnt <= cnt + 1; 
                       wr <= '0';
                    end if;
  end if;
end process;

led <= blink;
address(0) <= '1';

process(blink)                          -- write to uart
begin
     if blink = '1' then
       wr_data <= std_logic_vector(to_unsigned(49, 32));
     else
       wr_data <= std_logic_vector(to_unsigned(43, 32));
     end if;
end process;



end bench;

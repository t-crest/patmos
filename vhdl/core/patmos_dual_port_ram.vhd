-- A parameterized, inferable, true dual-port, dual-clock block RAM in VHDL.

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity patmos_dual_port_ram is
generic (
    DATA    : integer := 72;
    ADDR    : integer := 10
);
port (
    -- Port A
    a_clk   : in  std_logic;
    a_wr    : in  std_logic;
    a_addr  : in  unsigned(ADDR-1 downto 0);
    a_din   : in  unsigned(DATA-1 downto 0);
    a_dout  : out unsigned(DATA-1 downto 0);
    
    -- Port B
    b_clk   : in  std_logic;
    b_wr    : in  std_logic;
    b_addr  : in  unsigned(ADDR-1 downto 0);
    b_din   : in  unsigned(DATA-1 downto 0);
    b_dout  : out unsigned(DATA-1 downto 0)
);
end patmos_dual_port_ram;

architecture rtl of patmos_dual_port_ram is
    -- Shared memory
    type mem_type is array (0 to (2**ADDR)-1 ) of unsigned(DATA-1 downto 0);
    signal mem : mem_type;
begin

-- Port A
process(a_clk)
begin
    if(rising_edge(a_clk)) then
     --   if(a_wr='1') then
     --       mem(to_integer(a_addr)) <= a_din;
     --   end if;
        a_dout <= mem(to_integer(a_addr));
    end if;
end process;

-- Port B
process(b_clk)
begin
    if(rising_edge(b_clk)) then
        if(b_wr='1') then
            mem(to_integer(b_addr)) <= b_din;
        end if;
        b_dout <= mem(to_integer(b_addr));
    end if;
end process;

end rtl;



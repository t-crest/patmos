library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity tdp_ram is

generic (
    DATA    : integer := 32;
    ADDR    : integer := 8
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
end tdp_ram;


architecture rtl of tdp_ram is

--	 attribute ram_style : string;
    
-- Shared memory
    type mem_type is array ( (2**ADDR)-1 downto 0 ) of unsigned(DATA-1 downto 0);
    shared variable mem : mem_type := (others => (others => '0'));
	
--	 attribute ram_style of mem : variable is "distributed";

begin

-- Port A
porta : process(a_clk)
begin

    if( rising_edge(a_clk) ) then

        if(a_wr='1') then
            mem(to_integer(a_addr)) := a_din;
        end if;

        a_dout <= mem(to_integer(a_addr));
    end if;

end process;


-- Port B
portb : process(b_clk)
begin

    if( rising_edge(b_clk) ) then

        if(b_wr='1') then
            mem(to_integer(b_addr)) := b_din;
        end if;

        b_dout <= mem(to_integer(b_addr));

    end if;

end process;


end rtl;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.patmos_type_package.all;

entity patmos_stack_cache is
  port
  (
    	clk       	         		: in std_logic;
    	rst							: in std_logic;
       	din							: in patmos_stack_cache_in;
       	dout						: out patmos_stack_cache_out
  );    
end entity patmos_stack_cache;
architecture arch of patmos_stack_cache is

component patmos_dual_port_ram is
generic (
    DATA    : integer := 32;
    ADDR    : integer := 5
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

  end component;
  
begin

	stack_cache_ram : patmos_dual_port_ram port map    
	(
	-- Port A, mem
		    a_clk  => clk,
    		a_wr   => din.spill_fill,
   			a_addr => din.head_tail,
   			a_din  => din.din_from_mem,
   			a_dout => dout.dout_to_mem,
    
    -- Port B, CPU
    		b_clk  => clk,
    		b_wr   => din.write_enable,
    		b_addr => din.address,
    		b_din  => din.din_from_cpu,
    		b_dout => dout.dout_to_cpu
	);
	
  
     
end arch;



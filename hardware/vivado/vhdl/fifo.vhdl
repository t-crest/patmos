-----------------------------------------------------------------
-- Wrap-around FIFO
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity fifo is
	generic(
		FIFO_SIZE_WIDTH : integer := 8; --IN BITS OF ADDRESSING
		FIFO_DATA_WIDTH : integer := 16
	);
	port(
		clk      : in  std_logic;
		reset    : in  std_logic;

		--write port
		data_in  : in  std_logic_vector(FIFO_DATA_WIDTH - 1 downto 0);
		wr_en    : in  std_logic;
		full     : out std_logic;

		--read port
		data_out : out std_logic_vector(FIFO_DATA_WIDTH - 1 downto 0);
		rd_en    : in  std_logic;
		empty    : out std_logic
	);
end fifo;

architecture rtl of fifo is

	--true dual ported ram
	component tdp_ram is
		generic(
			DATA : integer := 32;
			ADDR : integer := 8
		);

		port(
			-- Port A
			a_clk  : in  std_logic;
			a_wr   : in  std_logic;
			a_addr : in  unsigned(ADDR - 1 downto 0);
			a_din  : in  unsigned(DATA - 1 downto 0);
			a_dout : out unsigned(DATA - 1 downto 0);

			-- Port B
			b_clk  : in  std_logic;
			b_wr   : in  std_logic;
			b_addr : in  unsigned(ADDR - 1 downto 0);
			b_din  : in  unsigned(DATA - 1 downto 0);
			b_dout : out unsigned(DATA - 1 downto 0)
		);
	end component;

	signal wr_ptr, rd_ptr   : unsigned(FIFO_SIZE_WIDTH - 1 downto 0);
	signal wr_data, rd_data : unsigned(FIFO_DATA_WIDTH - 1 downto 0);
	signal empty_flag       : std_logic;
	signal full_flag        : std_logic;

begin
	empty_flag <= '1' when wr_ptr = rd_ptr else '0';
	full_flag  <= '1' when wr_ptr = (rd_ptr - 1) else '0';

	full  <= full_flag;
	empty <= empty_flag;

	process(clk)
	begin
		if rising_edge(clk) then
			if (reset = '1') then
				-- reset
				wr_ptr <= (others => '0');
				rd_ptr <= (others => '0');
			else
				-- action: I have 4 possible conditions
				if (wr_en = '0') and (rd_en = '0') then
				-- no read, no write: do nothing
				elsif (wr_en = '0') and (rd_en = '1') then
					-- only read
					if (empty_flag = '0') then
						-- read
						rd_ptr <= rd_ptr + 1;
					end if;
				elsif (wr_en = '1') and (rd_en = '0') then
					-- write only
					if (full_flag = '1') then
						-- replace older sample
						wr_ptr <= wr_ptr + 1;
						rd_ptr <= rd_ptr + 1;
					else
						-- write
						wr_ptr <= wr_ptr + 1;
					end if;
				else                    --(wr_en = '1') and (rd_en = '1') then
					wr_ptr <= wr_ptr + 1;
					rd_ptr <= rd_ptr + 1;
				end if;
			end if;
		end if;
	end process;

	-- data casting and memory instance	
	wr_data  <= unsigned(data_in);
	data_out <= std_logic_vector(rd_data);

	tdp_ram_inst_0 : tdp_ram
		generic map(
			DATA => FIFO_DATA_WIDTH,
			ADDR => FIFO_SIZE_WIDTH
		)
		port map(
			-- Port A (used as write only)
			a_clk  => clk,
			a_wr   => wr_en,
			a_addr => wr_ptr,
			a_din  => wr_data,
			a_dout => open,

			-- Port B (used as read only)
			b_clk  => clk,
			b_wr   => '0',
			b_addr => rd_ptr,
			b_din  => (others => '0'),
			b_dout => rd_data);

end rtl;
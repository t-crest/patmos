-----------------------------------------------------------------
-- OCP to ADAU1761 audio buffer
-----------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity audio_buffer is
	GENERIC(
		BUFFER_SIZE_WIDTH : integer := 8; --IN BITS OF ADDRESSING
		BUFFER_DATA_WIDTH : integer := 16;
		OCP_DATA_WIDTH    : integer := 32;
		OCP_ADDR_WIDTH    : integer := 16
	);
	port(
		clk      : in  std_logic;
		reset    : in  std_logic;

		--ADC
		--write port
		data_in  : in  std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
		wr_en    : in  std_logic;

		--DAC
		--read port
		data_out : out std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
		rd_en    : in  std_logic;

		-- OCP IN (slave)
		MCmd     : in  std_logic_vector(2 downto 0);
		MAddr    : in  std_logic_vector((OCP_ADDR_WIDTH - 1) downto 0);
		MData    : in  std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);
		MByteEn  : in  std_logic_vector(3 downto 0);
		SResp    : out std_logic_vector(1 downto 0);
		SData    : out std_logic_vector((OCP_DATA_WIDTH - 1) downto 0)
	);
end audio_buffer;

architecture rtl of audio_buffer is
	component fifo is
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
	end component;

	-- ADC signals
	signal adc_data_out : std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
	signal adc_rd_en    : std_logic;
	signal adc_empty    : std_logic;
	--signal adc_full : std_logic;

	-- DAC signals
	signal dac_data_in : std_logic_vector(BUFFER_DATA_WIDTH - 1 downto 0);
	signal dac_wr_en   : std_logic;
	--signal dac_empty    : std_logic;
	signal dac_full    : std_logic;

	signal SResp_next : std_logic_vector(1 downto 0);
	signal SData_next : std_logic_vector((OCP_DATA_WIDTH - 1) downto 0);

begin

	--------------------------------------------------
	-- OCP related stuff
	--------------------------------------------------

	-- Address map
	-- 0000 R => ADC empty
	-- 0004 R => ADC read
	-- 0008 R => DAC full
	-- 000C W => DAC write

	dac_data_in <= MData(BUFFER_DATA_WIDTH - 1 downto 0);

	--Control mux
	process(MCmd, MAddr, adc_data_out, adc_empty, dac_full)
	begin
		SResp_next <= "00";
		SData_next <= (others => '0');
		adc_rd_en  <= '0';
		dac_wr_en  <= '0';

		case MCmd is
			-------- write
			when "001" =>
				case MAddr is
					when x"000C" =>
						dac_wr_en <= '1';
					when others =>
				end case;
				SResp_next <= "01";

			-------- read
			when "010" =>
				case MAddr is
					when x"0000" =>
						SData_next(0) <= adc_empty;
					when x"0004" =>
						SData_next(BUFFER_DATA_WIDTH - 1 downto 0) <= adc_data_out;
						adc_rd_en                                  <= '1';
					when x"0008" =>
						SData_next(0) <= dac_full;
					when others =>
				end case;
				SResp_next <= "01";

			-------- idle	
			when others =>
				SResp_next <= "00";
		end case;
	end process;

	--Register
	process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				SResp <= (others => '0');
				SData <= (others => '0');
			else
				SResp <= SResp_next;
				SData <= SData_next;
			end if;
		end if;
	end process;

	fifo_dac : fifo
		generic map(
			FIFO_SIZE_WIDTH => BUFFER_SIZE_WIDTH,
			FIFO_DATA_WIDTH => BUFFER_DATA_WIDTH
		)
		port map(
			clk      => clk,
			reset    => reset,

			--write port
			data_in  => dac_data_in,
			wr_en    => dac_wr_en,
			full     => dac_full,

			--read port
			data_out => data_out,
			rd_en    => rd_en,
			empty    => open            --dac_empty
		);

	fifo_adc : fifo
		generic map(
			FIFO_SIZE_WIDTH => BUFFER_SIZE_WIDTH,
			FIFO_DATA_WIDTH => BUFFER_DATA_WIDTH
		)
		port map(
			clk      => clk,
			reset    => reset,

			--write port
			data_in  => data_in,
			wr_en    => wr_en,
			full     => open,           --adc_full,

			--read port
			data_out => adc_data_out,
			rd_en    => adc_rd_en,
			empty    => adc_empty
		);
		
end rtl;
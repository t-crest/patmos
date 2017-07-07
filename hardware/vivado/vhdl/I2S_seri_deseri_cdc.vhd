----------------------------------------------------------------------------------
-- Engineer: Luca Pezzarossa 
-- Description: I2S serializer and deserializer with clock domain crossing
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity I2S_seri_deseri_cdc is
	generic(
		ADC_RESOLUTION : natural := 24;
		DAC_RESOLUTION : natural := 24
	);
    port ( clk : in STD_LOGIC;
           reset : in STD_LOGIC;
           
           adc_valid : out STD_LOGIC;
           adc_data_l : out STD_LOGIC_VECTOR (ADC_RESOLUTION-1 downto 0);
           adc_data_r : out STD_LOGIC_VECTOR (ADC_RESOLUTION-1 downto 0);
           
           dac_accept : out STD_LOGIC;
           dac_data_l : in STD_LOGIC_VECTOR (DAC_RESOLUTION-1 downto 0);
           dac_data_r : in STD_LOGIC_VECTOR (DAC_RESOLUTION-1 downto 0);
           
           i2s_bclk : in STD_LOGIC;
           i2s_lrclk : in STD_LOGIC;
           i2s_adc_sdata : in STD_LOGIC;
           i2s_dac_sdata : out STD_LOGIC);
end I2S_seri_deseri_cdc;

architecture Behavioral of I2S_seri_deseri_cdc is

	-- These arrays contain all the data (also the zeros)
    signal adc_data_full : STD_LOGIC_VECTOR (63 downto 0);
    signal dac_data_full : STD_LOGIC_VECTOR (63 downto 0);
    signal dac_data_buff : STD_LOGIC_VECTOR (63 downto 0);

	signal new_frame, half_frame : STD_LOGIC;
	signal i2s_lrclk_prev : STD_LOGIC;

	signal adc_shift_reg : STD_LOGIC_VECTOR (63 downto 0);
	signal adc_valid_slow, adc_valid_ms1, adc_valid_ms2, adc_valid_ms3 : STD_LOGIC;
	
	signal dac_shift_reg : STD_LOGIC_VECTOR (61 downto 0);
	signal dac_accept_slow, dac_accept_ms1, dac_accept_ms2, dac_accept_ms3 : STD_LOGIC;
	
begin
	
	-----------------------
	-- Common code
	-----------------------
	
	-- New frame pulse goes to 1 for 1 clock cycle ath the end of a frame, 
	new_frame <= not(i2s_lrclk) and i2s_lrclk_prev;
	half_frame <= i2s_lrclk and not(i2s_lrclk_prev);
	process(i2s_bclk)
	begin
		if rising_edge(i2s_bclk) then
			if reset = '1' then
				i2s_lrclk_prev <= '0';
			else
				i2s_lrclk_prev <= i2s_lrclk;
			end if;				
		end if;
	end process;
	
	-----------------------
	-- ADC deserializer
	-----------------------
	
	-- Frame to output assignments
	adc_data_l <= adc_data_full(62 downto 62-ADC_RESOLUTION+1);
    adc_data_r <= adc_data_full(30 downto 30-ADC_RESOLUTION+1);
	
	-- Slow domain
	process(i2s_bclk)
	begin
		if rising_edge(i2s_bclk) then
			if reset = '1' then
				adc_shift_reg  <= (others => '0');
				adc_data_full  <= (others => '0');
				adc_valid_slow <= '0';
			else
				adc_shift_reg(0) <= i2s_adc_sdata;
				for I in 1 to 63 loop
					adc_shift_reg(I) <= adc_shift_reg(I-1); 
				end loop;
				if new_frame = '1' then
					adc_data_full <= adc_shift_reg;
					adc_valid_slow <= not(adc_valid_slow);
				end if;
			end if;				
		end if;
	end process;

	-- ADC Metastability flip-flops and fast domain valid generation
	adc_valid <= adc_valid_ms3 xor adc_valid_ms2;
	process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				adc_valid_ms1 <= '0';
				adc_valid_ms2 <= '0';
				adc_valid_ms3 <= '0';
			else
				adc_valid_ms1 <= adc_valid_slow;
				adc_valid_ms2 <= adc_valid_ms1;
				adc_valid_ms3 <= adc_valid_ms2;
			end if;				
		end if;
	end process;

	-----------------------
	-- DAC serializer
	-----------------------
	
	-- Input to frame assignments
	dac_data_full(63) <= '0';
	dac_data_full(62 downto 62-DAC_RESOLUTION+1) <= dac_data_l;
	dac_data_full(62-DAC_RESOLUTION downto 31) <= (others => '0');
    dac_data_full(30 downto 30-DAC_RESOLUTION+1) <= dac_data_r;
	dac_data_full(30-DAC_RESOLUTION downto 0) <= (others => '0');

	-- Slow domain
	process(i2s_bclk)
	begin
		if rising_edge(i2s_bclk) then
			if reset = '1' then
				dac_accept_slow <= '0';
			else
				if half_frame = '1' then
					dac_accept_slow <= not(dac_accept_slow);
				end if;
			end if;				
		end if;
	end process;

	-- DAC Metastability flip-flops and fast domain accet generation (it happens at half frame), morover it buffers the data for domain crossing
	dac_accept <= dac_accept_ms3 xor dac_accept_ms2;
	process(clk)
	begin
		if rising_edge(clk) then
			if reset = '1' then
				dac_accept_ms1 <= '0';
				dac_accept_ms2 <= '0';
				dac_accept_ms3 <= '0';
				dac_data_buff <= (others => '0');
			else
				dac_accept_ms1 <= dac_accept_slow;
				dac_accept_ms2 <= dac_accept_ms1;
				dac_accept_ms3 <= dac_accept_ms2;
				if (dac_accept_ms3 xor dac_accept_ms2) = '1' then
					dac_data_buff <= dac_data_full;
				end if;
			end if;				
		end if;
	end process;

	-- Slow domain
	process(i2s_bclk)
	begin
		if rising_edge(i2s_bclk) then
			if reset = '1' then
				dac_shift_reg  <= (others => '0');
			else
				if new_frame = '1' then
					dac_shift_reg  <= dac_data_buff(61 downto 0);
					i2s_dac_sdata  <= dac_data_buff(62);
				else
					for I in 1 to 61 loop
						dac_shift_reg(I) <= dac_shift_reg(I-1);
						dac_shift_reg(0) <= '0';
						i2s_dac_sdata <= dac_shift_reg(61); 
					end loop;
				end if;
			end if;				
		end if;
	end process;

end Behavioral;

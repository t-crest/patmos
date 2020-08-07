-- Copyright: 2020, Technical University of Denmark, DTU Compute
-- Author: Christos Gkiokas
-- License: Simplified BSD License
--
-- PWM duty cycle measurment
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity PWMMeasure is
	port(
		clk        	 : in  std_logic;
		reset        : in  std_logic;
		pwm_in_port  : in std_logic;	
		duty_cycle_val : out std_logic_vector(31 downto 0)
		
	);
end PWMMeasure;

architecture rtl of PWMMeasure is

signal reg_in_1    : std_logic := '0';
signal reg_in_2    : std_logic := '0';
signal reg_in_3    : std_logic := '0';

signal pulse_high  : std_logic := '0';
signal counter : integer := 0;
signal clock_count : std_logic_vector(31 downto 0);
signal sample_counter : std_logic := '0';
begin

process(clk,clock_count,reg_in_1,reg_in_2,reg_in_3)
begin
        
	if rising_edge(clk) then
		reg_in_1 <= pwm_in_port;
		reg_in_2 <= reg_in_1;
		reg_in_3 <= reg_in_2;
		
		if reg_in_2 = '1' and reg_in_3 = '0' then
				pulse_high <= '1';
		end if;
		
		if reg_in_2 = '0' and reg_in_3 = '1' then
				pulse_high <= '0';
				
				
		end if;
		
	end if;

end process; 


process(clk,pulse_high)
begin
	if rising_edge(clk) then
	   sample_counter <= '0';
		if pulse_high = '1' then
			counter <= counter + 1;
			
		elsif pulse_high = '0' then
			clock_count <= std_logic_vector(to_unsigned(counter,clock_count'length));
			counter <= 0;
			sample_counter <= '1';
        end if;
     end if;
end process;

process(clk,sample_counter )
begin
    if  rising_edge(sample_counter) then
        duty_cycle_val <= clock_count;
    end if;
end process;


end rtl; 
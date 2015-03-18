LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY pll IS
	generic (input_freq: real; -- in MHz
             multiply_by : natural; divide_by : natural);
	PORT
	(
		inclk0		: IN STD_LOGIC  := '0';
		c0		: OUT STD_LOGIC;
		c1		: OUT STD_LOGIC;
		c2		: OUT STD_LOGIC;
		c3		: OUT STD_LOGIC;
		locked		: OUT STD_LOGIC 
	);
END pll;

ARCHITECTURE SIM OF pll IS

BEGIN
	c0 <= inclk0;
	c1 <= inclk0;
	c2 <= inclk0;
	c3 <= inclk0;
	locked <= '1';

END SIM;
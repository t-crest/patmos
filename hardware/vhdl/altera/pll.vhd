--
-- Copyright: 2013, Technical University of Denmark, DTU Compute
-- Author: Martin Schoeberl (martin@jopdesign.com)
-- License: Simplified BSD License
--

-- clock c1 is 180 degrees phase shifted
-- clock c2 is  90 degrees phase shifted
-- clock c3 is 270 degrees phase shifted

LIBRARY ieee;
USE ieee.std_logic_1164.all;

LIBRARY altera_mf;
USE altera_mf.all;

ENTITY pll IS
	generic (
		device_fam: string := "Cyclone II";
		input_freq: real; -- in MHz
		multiply_by : natural; 
		divide_by : natural);
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

ARCHITECTURE SYN OF pll IS

	SIGNAL sub_wire0	: STD_LOGIC_VECTOR (1 DOWNTO 0);
	SIGNAL sub_wire1	: STD_LOGIC_VECTOR (5 DOWNTO 0);
	SIGNAL sub_wire2	: STD_LOGIC ;
    
    constant output_period : integer
        := integer(1000000.0/(input_freq*real(multiply_by)/real(divide_by)));

	COMPONENT altpll
	GENERIC (
		clk0_divide_by		: NATURAL;
		clk0_duty_cycle		: NATURAL;
		clk0_multiply_by		: NATURAL;
		clk0_phase_shift		: STRING;
		clk1_divide_by		: NATURAL;
		clk1_duty_cycle		: NATURAL;
		clk1_multiply_by		: NATURAL;
		clk1_phase_shift		: STRING;
		clk2_divide_by		: NATURAL;
		clk2_duty_cycle		: NATURAL;
		clk2_multiply_by		: NATURAL;
		clk2_phase_shift		: STRING;
		clk3_divide_by		: NATURAL;
		clk3_duty_cycle		: NATURAL;
		clk3_multiply_by		: NATURAL;
		clk3_phase_shift		: STRING;
		compensate_clock		: STRING;
		gate_lock_signal		: STRING;
		inclk0_input_frequency		: NATURAL;
		intended_device_family		: STRING;
		invalid_lock_multiplier		: NATURAL;
		lpm_hint		: STRING;
		lpm_type		: STRING;
		operation_mode		: STRING;
		port_activeclock		: STRING;
		port_areset		: STRING;
		port_clkbad0		: STRING;
		port_clkbad1		: STRING;
		port_clkloss		: STRING;
		port_clkswitch		: STRING;
		port_configupdate		: STRING;
		port_fbin		: STRING;
		port_inclk0		: STRING;
		port_inclk1		: STRING;
		port_locked		: STRING;
		port_pfdena		: STRING;
		port_phasecounterselect		: STRING;
		port_phasedone		: STRING;
		port_phasestep		: STRING;
		port_phaseupdown		: STRING;
		port_pllena		: STRING;
		port_scanaclr		: STRING;
		port_scanclk		: STRING;
		port_scanclkena		: STRING;
		port_scandata		: STRING;
		port_scandataout		: STRING;
		port_scandone		: STRING;
		port_scanread		: STRING;
		port_scanwrite		: STRING;
		port_clk0		: STRING;
		port_clk1		: STRING;
		port_clk2		: STRING;
		port_clk3		: STRING;
		port_clk4		: STRING;
		port_clk5		: STRING;
		port_clkena0		: STRING;
		port_clkena1		: STRING;
		port_clkena2		: STRING;
		port_clkena3		: STRING;
		port_clkena4		: STRING;
		port_clkena5		: STRING;
		port_extclk0		: STRING;
		port_extclk1		: STRING;
		port_extclk2		: STRING;
		port_extclk3		: STRING;
		valid_lock_multiplier		: NATURAL
	);
	PORT (
			clk	: OUT STD_LOGIC_VECTOR (5 DOWNTO 0);
			inclk	: IN STD_LOGIC_VECTOR (1 DOWNTO 0);
			locked	: OUT STD_LOGIC 
	);
	END COMPONENT;

BEGIN
	sub_wire0(0) <= inclk0;
	sub_wire0(1) <= '0';
	c0           <= sub_wire1(0);
	c1           <= sub_wire1(1);
	c2           <= sub_wire1(2);
	c3           <= sub_wire1(3);
	locked       <= sub_wire2;

	altpll_component : altpll
	GENERIC MAP (
		clk0_divide_by => divide_by,
		clk0_duty_cycle => 50,
		clk0_multiply_by => multiply_by,
		clk0_phase_shift => "0",
		clk1_divide_by => divide_by,
		clk1_duty_cycle => 50,
		clk1_multiply_by => multiply_by,
		clk1_phase_shift => integer'image(2*output_period/4),
		clk2_divide_by => divide_by,
		clk2_duty_cycle => 50,
		clk2_multiply_by => multiply_by,
		clk2_phase_shift => integer'image(1*output_period/4),
		clk3_divide_by => divide_by,
		clk3_duty_cycle => 50,
		clk3_multiply_by => multiply_by,
		clk3_phase_shift => integer'image(3*output_period/4),
		compensate_clock => "CLK0",
		gate_lock_signal => "NO",
		inclk0_input_frequency => integer(1000000.0/input_freq),
		intended_device_family => device_fam,
		invalid_lock_multiplier => 5,
		lpm_hint => "CBX_MODULE_PREFIX=pll",
		lpm_type => "altpll",
		operation_mode => "NORMAL",
		port_activeclock => "PORT_UNUSED",
		port_areset => "PORT_UNUSED",
		port_clkbad0 => "PORT_UNUSED",
		port_clkbad1 => "PORT_UNUSED",
		port_clkloss => "PORT_UNUSED",
		port_clkswitch => "PORT_UNUSED",
		port_configupdate => "PORT_UNUSED",
		port_fbin => "PORT_UNUSED",
		port_inclk0 => "PORT_USED",
		port_inclk1 => "PORT_UNUSED",
		port_locked => "PORT_USED",
		port_pfdena => "PORT_UNUSED",
		port_phasecounterselect => "PORT_UNUSED",
		port_phasedone => "PORT_UNUSED",
		port_phasestep => "PORT_UNUSED",
		port_phaseupdown => "PORT_UNUSED",
		port_pllena => "PORT_UNUSED",
		port_scanaclr => "PORT_UNUSED",
		port_scanclk => "PORT_UNUSED",
		port_scanclkena => "PORT_UNUSED",
		port_scandata => "PORT_UNUSED",
		port_scandataout => "PORT_UNUSED",
		port_scandone => "PORT_UNUSED",
		port_scanread => "PORT_UNUSED",
		port_scanwrite => "PORT_UNUSED",
		port_clk0 => "PORT_USED",
		port_clk1 => "PORT_USED",
		port_clk2 => "PORT_UNUSED",
		port_clk3 => "PORT_UNUSED",
		port_clk4 => "PORT_UNUSED",
		port_clk5 => "PORT_UNUSED",
		port_clkena0 => "PORT_UNUSED",
		port_clkena1 => "PORT_UNUSED",
		port_clkena2 => "PORT_UNUSED",
		port_clkena3 => "PORT_UNUSED",
		port_clkena4 => "PORT_UNUSED",
		port_clkena5 => "PORT_UNUSED",
		port_extclk0 => "PORT_UNUSED",
		port_extclk1 => "PORT_UNUSED",
		port_extclk2 => "PORT_UNUSED",
		port_extclk3 => "PORT_UNUSED",
		valid_lock_multiplier => 1
	)
	PORT MAP (
		inclk => sub_wire0,
		clk => sub_wire1,
		locked => sub_wire2
	);



END SYN;


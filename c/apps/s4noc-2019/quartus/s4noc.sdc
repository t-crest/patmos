###########################################################################
# SDC files for DE2-115 board
###########################################################################

# Clock in input pin (400 MHz)
create_clock -period 2.5 [get_ports clk]

# Create generated clocks based on PLLs
derive_pll_clocks -use_tan_name

derive_clock_uncertainty


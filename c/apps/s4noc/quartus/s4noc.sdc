###########################################################################
# SDC files for DE2-115 board
###########################################################################

# Clock in input pin (200 MHz)
create_clock -period 5 [get_ports clk]

# Create generated clocks based on PLLs
derive_pll_clocks -use_tan_name

derive_clock_uncertainty


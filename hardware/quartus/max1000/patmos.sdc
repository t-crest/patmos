###########################################################################
# SDC files for MAX1000 board
###########################################################################

# Clock in input pin (12 MHz)
create_clock -period 83.333 [get_ports clk]

# Create generated clocks based on PLLs
derive_pll_clocks -use_tan_name

derive_clock_uncertainty

# ** Input/Output Delays

# Use FPGA-centric constraints (general pins)
# Tsu 5 ns
set_max_delay -from [all_inputs] -to [all_registers] 5
set_min_delay -from [all_inputs] -to [all_registers] 0
# Tco 10 ns
set_max_delay -from [all_registers] -to [all_outputs] 10
set_min_delay -from [all_registers] -to [all_outputs] 0

# Use FPGA-centric constraints (SRAM pins)
# Tsu 3 ns
set_max_delay -from [get_ports *RAM*] -to [get_registers {*}] 3
# Tco 5.5 ns
set_max_delay -from [get_registers *] -to [get_ports {*RAM*}] 5.5

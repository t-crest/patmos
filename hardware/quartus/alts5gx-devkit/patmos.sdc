###########################################################################
# SDC files for Stratix-V development board
###########################################################################

# Clock in input pin (50 MHz)
create_clock -period 20 [get_ports clk]

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

# Use FPGA-centric constraints (QDR RAM pins)
# Tsu 3.0 ns
set_max_delay -from [get_ports *qdrIIplus*] -to [get_clocks {*}] 3.0
# Tco 6.2 ns
set_max_delay -from [get_clocks {*}] -to [get_ports {*qdrIIplus*}] 6.2

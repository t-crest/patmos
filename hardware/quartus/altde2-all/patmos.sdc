###########################################################################
# SDC files for DE2-115 board
###########################################################################

# Clock in input pin (50 MHz)
create_clock -period 20 [get_ports clk]

# Clock PHY (25MHz)
# create_clock -period 40 [get_ports ENET0_RX_CLK]
# create_clock -period 40 [get_ports ENET0_TX_CLK]

# Create generated clocks based on PLLs
derive_pll_clocks -use_tan_name

derive_clock_uncertainty

# ** Input/Output Delays

# Unconstrained pins for gpios
set_false_path -from * -to [get_ports {*oLedsPins*}]
set_false_path -from * -to [get_ports {*osevenSegmentDisplayPins*}]
set_false_path -from [get_ports {*iKeysPins*}] -to *

# Use FPGA-centric constraints (SRAM pins)
# Tsu 3 ns
set_max_delay -from [get_ports *RAM*] -to [get_registers {*}] 3
# Tco 5.5 ns
set_max_delay -from [get_registers *] -to [get_ports {*RAM*}] 5.5

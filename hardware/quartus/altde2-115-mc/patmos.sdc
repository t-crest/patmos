###########################################################################
# SDC files for DE2-115 board
###########################################################################

# Clock in input pin (50 MHz)
create_clock -period 20 -name inclk_IN [get_ports {inclk0}]

# Create generated clocks based on PLLs
derive_pll_clocks -use_tan_name

derive_clock_uncertainty

## Cutting paths between the JTAG and other clocks
create_clock -period 100.00 -name altera_reserved_tck [get_ports altera_reserved_tck]
set_clock_groups -asynchronous -group {altera_reserved_tck}

## Cutting paths for the JTAG tdo, tdi, and tms pins
set_false_path -to [get_ports altera_reserved_tdo]
set_false_path -from [get_ports altera_reserved_tdi]
set_false_path -from [get_ports altera_reserved_tms]

# ** Input/Output Delays

# Use FPGA-centric constraints (general pins)
# Tsu 5 ns
#set_max_delay -from [all_inputs] -to [all_registers] 5
#set_min_delay -from [all_inputs] -to [all_registers] 0
# Tco 10 ns
#set_max_delay -from [all_registers] -to [all_outputs] 10
#set_min_delay -from [all_registers] -to [all_outputs] 0

# Use FPGA-centric constraints (SRAM pins)
# Tsu 3 ns
set_max_delay -from [get_ports *RAM*] -to [get_registers {*}] 3
# Tco 5.5 ns
set_max_delay -from [get_registers *] -to [get_ports {*RAM*}] 5.5

## ADC input clock
create_clock -name clk_adc -period 50.000 [get_ports {sys_adc_feedback_clk}]

set_input_delay -add_delay -max -clock [get_clocks {clk_adc}]  37.0 [get_ports {*_Sync_Dat_*}]
set_input_delay -add_delay -min -clock [get_clocks {clk_adc}]   5.8 [get_ports {*_Sync_Dat_*}]

# False paths for asynch clk_adc
set_false_path -from [get_clocks {altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0]}] -to [get_clocks {clk_adc}]
set_false_path -from [get_clocks {clk_adc}] -to [get_clocks {altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0]}]

# EnDat I/O
set_input_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -max 10 [get_ports {*_endat_RS485_Data_Input}]
set_input_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -min 0 [get_ports {*_endat_RS485_Data_Input}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -max 5 [get_ports {*_endat_RS485_Data_Enable}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -min 0 [get_ports {*_endat_RS485_Data_Enable}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -max 5 [get_ports {*_endat_RS485_Clk_Out}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -min 0 [get_ports {*_endat_RS485_Clk_Out}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -max 5 [get_ports {*_endat_RS485_Data_Out}]
set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } -min 0 [get_ports {*_endat_RS485_Data_Out}]

# I/O false paths
set_false_path -to [get_ports *LED\[*\]*]
set_false_path -to [get_ports *sys_adc_clk]
set_false_path -to [get_ports *sys_pfc_*]
set_false_path -from [get_ports *button\[*\]*]
set_false_path -from [get_ports *drive*_sm_igbt_err*]
set_false_path -from [get_ports *sys_pfc_*]

set_output_delay -clock { altpll_patmos:pll_inst|altpll:altpll_component|altpll_patmos_altpll:auto_generated|wire_pll1_clk[0] } 10 [get_ports *drive*pwm*]

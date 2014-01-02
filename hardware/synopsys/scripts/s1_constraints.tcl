
# create clock
# use a relative high clock period for area optimized
create_clock clk -period 2 -name clk

#clock skew
set_clock_uncertainty .1 clk
set_fix_hold clk

# input propagation delay
set_input_delay -max .3 -clock clk [get_object_name [get_ports io* -filter pin_direction==in]]

# output propagation delay
set_output_delay -max .3 -clock clk [get_object_name [get_ports io* -filter pin_direction==out]]

#set_max_area 9000 
write -hierarchy -format ddc -output patmos_constrained.ddc

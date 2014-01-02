# its getting cold outside, so produce some heat by using all cores :)
set_host_options -max_cores 4

remove_design [get_designs]

analyze -format verilog -library WORK {vhdl/Patmos.strippedMemBlocks.v}

elaborate PatmosCore -architecture verilog -library WORK

write -hierarchy -format ddc -output patmos_readin.ddc

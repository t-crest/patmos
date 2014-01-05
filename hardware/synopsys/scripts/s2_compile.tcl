

compile -map_effort medium

# check the design
report_constraint -all_violators
check_design
report_timing
report_area

#change_names -rules verilog -hierarchy

#ungroup -flatten -all

write -format verilog -hierarchy -output netlists/patmos_mapped.v
write_sdf netlists/patmos_mapped.sdf
write_sdc netlists/patmos_mapped.sdc

write -hierarchy -format ddc -output patmos_compiled.ddc

#exec perl netlists/fix_pipe_sdf.pl netlists/gcd_chip

#remove_design [get_designs]


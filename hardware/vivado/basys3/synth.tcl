open_project ./vivado/basys3/basys3.xpr
reset_project
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1
close_project

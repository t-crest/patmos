set NumericStdNoWarnings 1
when -label end_of_simulation {end_of_sim == '1'} {echo "End of simulation"
; stop ; quit; }
run 20000ns
quit

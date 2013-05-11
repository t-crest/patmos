all: sim

sim:
	rm -rf work
	vlib work
	vlog -quiet ../generated/Patmos.v Patmos_tb.v
	vsim -i -do sim.do Patmos_tb

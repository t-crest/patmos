# Use Wine on OSX
# I would like to use a better way, but some shell variables
# are not set within make.... Don't know why...
ifeq ($(TERM_PROGRAM),Apple_Terminal)
	WINE=wine
else
	WINE=
endif

all: sim

sim:
	rm -rf work
	$(WINE) vlib work
	$(WINE) vcom -quiet +acc gen_utils.vhd conversions.vhd sim_ssram_512x36.vhd
	$(WINE) vlog -quiet +acc ../build/Patmos.v Patmos_tb.v
	$(WINE) vsim -i -do sim.do Patmos_tb

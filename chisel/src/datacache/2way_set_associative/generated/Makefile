#
#	Makefile for ModelSim simulation
#


OPTIONS=-93 -quiet

ifeq ($(WINDIR),)
	S=:
else
	S=\;
endif

# Use Wine on OSX
# I would like to use a better way, but some shell variables
# are not set within make.... Don't know why...
ifeq ($(TERM_PROGRAM),Apple_Terminal)
	WINE=wine
else
	WINE=
endif


all: clean base
	$(WINE) vlog $(OPTIONS) dc2way_top.v	
	$(WINE) vlog $(OPTIONS) Test_dc2way.v
	$(WINE) vsim -do sim.do dc2way_top

batch: clean base
	$(WINE) vlog $(OPTIONS) dc2way_top.v
	$(WINE) vlog $(OPTIONS) Test_dc2way.v

	$(WINE) vsim -c -do batch.do dc2way_top

base:
	$(WINE) vlib work
	$(WINE) vlog $(OPTIONS) dc2way_top.v
	$(WINE) vlog $(OPTIONS) Test_dc2way.v





clean:
	-rm -f transcript
	-rm -f *.wlf
	-rm -rf work


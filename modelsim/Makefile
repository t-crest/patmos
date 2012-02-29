#
#	Makefile for ModelSim simulation
#


PATMOSDIR=../vhdl
#OPTIONS=-93 -quiet -check_synthesis -lint -pedanticerrors
OPTIONS=-93 -quiet

ifeq ($(WINDIR),)
	WINE=wine
	S=:
else
	WINE=
	S=\;
endif

all: clean base
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/testbench.vhd
	$(WINE) vsim -do sim.do pc_tester

base:
	$(WINE) vlib work
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/type_package.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/register_file.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/pc_generator.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/clock_input.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/fetch.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/decode.vhd
	$(WINE) vcom $(OPTIONS) $(PATMOSDIR)/core/execution.vhd
clean:
	-rm *.wlf


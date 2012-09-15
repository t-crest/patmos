#
#	Makefile for ModelSim simulation
#


SRCDIR=../hdl/vhdl
#OPTIONS=-93 -quiet -check_synthesis -lint -pedanticerrors
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


all: simulate

batch: VSIM_OPT:=-c -do batch.do
batch: vsim


simulate: VSIM_OPT:=-do sim.do 
simulate: vsim

vsim: clean base
	$(WINE) vcom $(OPTIONS) dma_controller_tb.vhd
	$(WINE) vsim $(VSIM_OPT) dma_controller_tb

base:
	$(WINE) vlib work
	$(WINE) vcom $(OPTIONS) $(SRCDIR)/dma_controller_dtl.e.vhd
	$(WINE) vcom $(OPTIONS) $(SRCDIR)/dma_controller_dtl.p.vhd
	$(WINE) vcom $(OPTIONS) $(SRCDIR)/dma_controller_dtl.a.vhd

clean:
	-rm -f transcript
	-rm -f *.wlf
	-rm -rf work

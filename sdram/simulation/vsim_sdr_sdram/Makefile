#
#	Makefile for ModelSim simulation
#


SRCDIR=../../
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

TB = sdr_sdram_dma_controller_tb

VHDL_FILES += $(SRCDIR)../ise/ml605_edk/pcores/dma_controller_dtl_v1_00_a/hdl/vhdl/dma_controller_dtl.e.vhd
VHDL_FILES += $(SRCDIR)../ise/ml605_edk/pcores/dma_controller_dtl_v1_00_a/hdl/vhdl/dma_controller_dtl.p.vhd
VHDL_FILES += $(SRCDIR)../ise/ml605_edk/pcores/dma_controller_dtl_v1_00_a/hdl/vhdl/dma_controller_dtl.a.vhd
VHDL_FILES += $(SRCDIR)sdr_dram.vhd

VHDL_FILES += $(SRCDIR)simulation/mt48lc2m32b2_2.vhd

VHDL_FILES += $(SRCDIR)simulation/$(TB).vhd




all: simulate

batch: VSIM_OPT:=-c -t 100ps -do batch.do
batch: vsim


simulate: VSIM_OPT:=-t 100ps -do sim.do 
simulate: vsim

vsim: clean base
	$(WINE) vsim $(VSIM_OPT) $(TB)

base:
	-$(WINE) vlib work
	for f in $(VHDL_FILES) ; do \
	$(WINE) vcom $(OPTIONS) $$f \
	; done

clean:
	-rm -f transcript
	-rm -f *.wlf
	-rm -rf work

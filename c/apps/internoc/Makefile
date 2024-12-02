PATMOSHOME=../../..
BUILDDIR?=$(PATMOSHOME)/tmp
LIBCORETHREAD=$(BUILDDIR)/libcorethread.a
LIBNOC=$(PATMOSHOME)/c/libnoc
LIBMP=$(PATMOSHOME)/c/libmp
LIBETH=$(PATMOSHOME)/c/ethlib
SERIAL?=/dev/ttyUSB0

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/include  \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBCORETHREAD) \
	-I $(LIBNOC) \
	-I $(LIBMP) \
	-I $(LIBETH) \
	$(DEFINES)

# WCET analysis

# running examples

internoc_tokens_demo:
	patmos-clang $(CFLAGS) $(LIBMP)/*.c $(LIBNOC)/*.c $(LIBETH)/*.c internoc_driver.c internoc_tokens_demo.c -o internoc_demo.elf -L$(BUILDDIR)

sim:
	pasim --cores 9 -b tt_scheduling_demo.elf

emu:
	patemu tt_scheduling_demo.elf

config:
	killall jtagd; cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) internoc_demo.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

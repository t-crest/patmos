PATMOSHOME=../../..
BUILDDIR?=$(PATMOSHOME)/tmp
LIBNOC=$(PATMOSHOME)/c/libnoc
LIBMP=$(PATMOSHOME)/c/libmp
LIBETH=$(PATMOSHOME)/c/ethlib
NOCINIT?=$(PATMOSHOME)/c/cmp/nocinit.c
SERIAL?=/dev/ttyUSB0

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/include  \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBNOC) \
	-I $(LIBMP) \
	-I $(LIBETH) \
	$(DEFINES)

# WCET analysis

# compiling examples
force:
	touch rosace_patmos.c

rosace_patmos:
	patmos-clang $(CFLAGS) $(NOCINIT) assemblage.c assemblage_includes.c io.c barrier_counter.c threads.c rosace_patmos.c  -o rosace_patmos.elf -L$(BUILDDIR) -lm

# running examples
sim:
	pasim --cores 9 --debug-intrs -V -b rosace_patmos.elf 

emu:
	patemu rosace_patmos.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-115

download:
	patserdow -v $(SERIAL) rosace_patmos.elf

# utilities
clean:
	rm -f *.out *.pcap *.pml *.png *.elf

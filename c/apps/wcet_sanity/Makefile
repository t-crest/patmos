PATMOSHOME=~/t-crest/patmos
BUILDDIR?=../../../tmp
SERIAL?=/dev/ttyUSB0
LIBCORETHREAD=$(BUILDDIR)/libcorethread.a
LIBETH=$(PATMOSHOME)/c/ethlib

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBCORETHREAD) \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

# library for corethreads
.PHONY: libcorethread
libcorethread: $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: libcorethread/corethread.h
$(LIBCORETHREAD): $(BUILDDIR)/libcorethread/corethread.o
	patmos-ar r $@ $^

# WCET analysis

wcet:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml wcet_sanity_demo.c -o wcet_sanity_demo.elf
	platin wcet --disable-ait -i tpip.pml -b wcet_sanity_demo.elf -e mem_iord_byte
	platin wcet --disable-ait -i tpip.pml -b wcet_sanity_demo.elf -e wcet_mem_rd
	platin wcet --disable-ait -i tpip.pml -b wcet_sanity_demo.elf -e wcet_integration

comp:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c wcet_sanity_demo.c -o wcet_sanity_demo.elf

sim:
	pasim -b wcet_sanity_demo.elf

emu:
	patemu wcet_sanity_demo.elf

download:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	sleep 2
	patserdow -v $(SERIAL) wcet_sanity_demo.elf

# utilities

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBETH=$(PATMOSHOME)/c/ethlib

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

wcet:
	patmos-clang -O0 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml tte_wcet.c -o tte_wcet.elf
	platin wcet --disable-ait -i tpip.pml -b tte_wcet.elf -e tte_code_int
	platin wcet --disable-ait -i tpip.pml -b tte_wcet.elf -e tte_code_tt
	platin wcet --disable-ait -i tpip.pml -b tte_wcet.elf -e tte_loop

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

tte_demo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_demo.c -o tte_demo.elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 tte_demo.elf

tte_demo_latency:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_demo_latency.c -o tte_demo_latency.elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 tte_demo_latency.elf

tte_demo_interrupts:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_demo_interrupts.c -o tte_demo_interrupts.elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 tte_demo_interrupts.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

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
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml timestamp_compare.c -o timestamp_compare.elf
	platin wcet --disable-ait -i tpip.pml -b timestamp_compare.elf -e arp_process_received
	platin wcet --disable-ait -i tpip.pml -b timestamp_compare.elf -e icmp_process_received
	platin wcet --disable-ait -i tpip.pml -b timestamp_compare.elf -e checkForPacket
	platin wcet --disable-ait -i tpip.pml -b timestamp_compare.elf -e demoLoop

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

timestamp_compare:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c timestamp_compare.c -o timestamp_compare.elf

download:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	sleep 2
	patserdow -v $(SERIAL) timestamp_compare.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

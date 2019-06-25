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

wcet-server:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml flight_info_server.c -o flight_info_server.elf
	platin wcet --disable-ait -i tpip.pml -b flight_info_server.elf -e handle_fsim_msg

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

flight_info_server:
	touch flight_info_server.c
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c flight_info_server.c -o flight_info_server.elf

download:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	sleep 2
	patserdow -v $(SERIAL) flight_info_server.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

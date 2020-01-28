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

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

# WCET analysis

wcet_timestamp:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml timestamp_compare.c -o aion_demo.elf
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e arp_process_received
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e icmp_process_received
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e demoLoop

wcet_tsnsync:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml tsn_timesyncdemo.c -o aion_demo.elf
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e ptp_correct_offset
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e ptpv2_issue_msg
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e ptpv2_handle_msg
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e ptpv2_process_received
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e slaveSyncLoop
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e masterSyncLoop
	
wcet_ttesync:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml tte_timesyncdemo.c -o aion_demo.elf
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e syncWindow
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e tte_pcf_handle
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e syncMonitor
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e demoEvent

wcet_tte_executive:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml tte_sync_executive_demo.c -o aion_demo.elf
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e syncWindow
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e tte_pcf_handle
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e syncMonitor
	platin wcet --disable-ait -i tpip.pml -b aion_demo.elf -e demoEvent
	
# running examples

timestamp_compare:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c timestamp_compare.c -o aion_demo.elf

tte_timesyncdemo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_timesyncdemo.c -o aion_demo.elf

tsn_timesyncdemo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tsn_timesyncdemo.c -o aion_demo.elf

tte_sync_executive_demo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_sync_executive_demo.c -o aion_demo.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) aion_demo.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

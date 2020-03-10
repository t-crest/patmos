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
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=aion_wcet.pml timestamp_compare.c -o aion_wcet.elf
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e arp_process_received
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e icmp_process_received
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e demoLoop

wcet_tsnsync:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=aion_wcet.pml tsn_timesyncdemo.c -o aion_wcet.elf
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e receiveAndHandleFrame
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e ptp_correct_offset
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e ptpv2_issue_msg
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e ptpv2_handle_msg
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e ptpv2_process_received
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e slaveSyncLoop
	platin wcet --disable-ait -i aion_wcet.pml -b aion_wcet.elf -e masterSyncLoop
	
wcet_ttesync:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=aion_wcet.pml tte_timesyncdemo.c -o aion_wcet.elf
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e cyclicExecutiveLoop
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_sync
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_pulse
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e receiveAndHandleFrame
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e tte_pcf_handle

wcet_tteproduceconsume:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=aion_wcet.pml tte_produceconsume.c -o aion_wcet.elf
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_sync
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_pulse
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_calc
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_send
	platin wcet --disable-ait --enable-wca -i aion_wcet.pml -b aion_wcet.elf -e task_recv

	
# running examples

aion_sanity_check:
	patmos-clang -O2 $(FLAGS) $(LIBETH)/*.c aion_sanity_check.c -o aion.elf

timestamp_compare:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c timestamp_compare.c -o aion.elf

tte_timesyncdemo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_timesyncdemo.c -o aion.elf

tsn_timesyncdemo:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tsn_timesyncdemo.c -o aion.elf

tte_produceconsume:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_produceconsume.c -o aion.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) aion.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

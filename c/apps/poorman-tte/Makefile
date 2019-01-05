PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBETH=$(PATMOSHOME)/c/ethlib
ROLE?=SERVER

ROLE_LOWER=`echo $(ROLE) | tr A-Z a-z`

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

wcet:
	patmos-clang -O2 -D $(ROLE) $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml ttepwm.c -o ttepwm_$(ROLE).elf
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e $(ROLE_LOWER)_run
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e check_packet
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e exec_act_task
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e exec_daq_task
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e exec_sync_task
	platin wcet --disable-ait -i tpip.pml -b ttepwm_$(ROLE).elf -e exec_report_task

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

ttepwm:
	patmos-clang -O2 -D $(ROLE) $(CFLAGS) $(LIBETH)/*.c ttepwm.c -o ttepwm_$(ROLE).elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 ttepwm_$(ROLE).elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

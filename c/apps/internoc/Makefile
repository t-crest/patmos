PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBCORETHREAD=$(PATMOSHOME)/c/libcorethread
LIBNOC=$(PATMOSHOME)/c/libnoc
LIBMP=$(PATMOSHOME)/c/libmp
LIBETH=$(PATMOSHOME)/c/ethlib
NOCINIT?=$(PATMOSHOME)/c/cmp/nocinit.c

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBCORETHREAD) \
	-I $(LIBNOC) \
	-I $(LIBMP) \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

# touch the nocinit file
$(NOCINIT):
	touch $@

# WCET analysis

# running examples

internoc_tokens_demo:
	patmos-clang -O2 $(CFLAGS) $(NOCINIT) $(LIBMP)/*.c $(LIBNOC)/*.c $(LIBCORETHREAD)/*.c $(LIBETH)/*.c *.c -o internoc_demo.elf

config:
	killall jtagd; cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) internoc_demo.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

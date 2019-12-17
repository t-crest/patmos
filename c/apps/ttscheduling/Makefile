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
	ar r $@ $^

# library for corethreads
.PHONY: libcorethread
libcorethread: $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: libcorethread/corethread.h
$(LIBCORETHREAD): $(BUILDDIR)/libcorethread/corethread.o
	ar r $@ $^

# WCET analysis

wcet_scheduler:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml -D WCET tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e init_minimal_tttask
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e init_minimal_ttschedule
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e sort_asc_minimal_tttasks
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e tt_minimal_dispatcher
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e tt_minimal_schedule_loop

wcet_demo_tasks:
	patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c -mserialize=tpip.pml -D WCET tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_act
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_rcv
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_snd
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_mon
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_syn
	
# compiling examples

force:
	touch tt_scheduling_demo.c

tt_scheduling_demo:
	patmos-clang -O2 $(CFLAGS) tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf

tt_scheduling_demo_debug:
	patmos-clang -O2 $(CFLAGS) tt_scheduling_demo.c tt_minimal_scheduler.c -D DEBUG -o tt_scheduling_demo.elf

tt_scheduling_demo_threaded:
	patmos-clang -O2 $(CFLAGS) tt_scheduling_demo_threaded.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf -L$(BUILDDIR) -lcorethread

# running examples

sim:
	pasim --cores 9 -b tt_scheduling_demo.elf

emu:
	patemu tt_scheduling_demo.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) tt_scheduling_demo.elf

# utilities

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

PATMOSHOME=../../..
BUILDDIR?=$(PATMOSHOME)/tmp
LIBETH=$(PATMOSHOME)/c/ethlib/
LIBCORETHREADDIR=$(PATMOSHOME)/c/libcorethread
LIBCORETHREAD=$(BUILDDIR)/libcorethread.a
SERIAL?=/dev/ttyUSB0

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/include  \
	-I $(PATMOSHOME)/c/libelf/ \
	$(DEFINES)

# library for corethreads
.PHONY: libcorethread
libcorethread: $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: $(LIBCORETHREADDIR)/corethread.h
$(LIBCORETHREAD): $(BUILDDIR)/libcorethread/corethread.o
	patmos-ar r $@ $^

# WCET analysis

wcet_scheduler:
	patmos-clang $(CFLAGS) -mserialize=tpip.pml -D WCET tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e init_minimal_tttask
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e init_minimal_ttschedule
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e sort_asc_minimal_tttasks
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e tt_minimal_dispatcher
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e tt_minimal_schedule_loop

wcet_demo_tasks:
	patmos-clang $(CFLAGS) -mserialize=tpip.pml -D WCET tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_1
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_2
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_3
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_4
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_5
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_6
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_7
	platin wcet --disable-ait -i tpip.pml -b tt_scheduling_demo.elf -e task_8
	
# compiling examples

force:
	touch tt_scheduling_demo.c

tt_scheduling_demo:
	patmos-clang $(CFLAGS) tt_scheduling_demo.c tt_minimal_scheduler.c -o tt_scheduling_demo.elf

tt_scheduling_demo_debug:
	patmos-clang $(CFLAGS) tt_scheduling_demo.c tt_minimal_scheduler.c -D DEBUG -o tt_scheduling_demo.elf

tt_scheduling_demo_threaded:
	patmos-clang $(CFLAGS) $(LIBCORETHREAD) tt_scheduling_demo_threaded.c tt_minimal_scheduler.c -D THREADED -o tt_scheduling_demo.elf -L$(BUILDDIR) -lcorethread

tt_scheduling_demo_debug_threaded:
	patmos-clang $(CFLAGS) $(LIBCORETHREAD) tt_scheduling_demo_threaded.c tt_minimal_scheduler.c -D THREADED -D DEBUG -o tt_scheduling_demo.elf -L$(BUILDDIR) -lcorethread

# running examples

sim:
	pasim --cores 4 -b tt_scheduling_demo.elf

emu:
	patemu tt_scheduling_demo.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-115

download:
	patserdow -v $(SERIAL) tt_scheduling_demo.elf

# utilities

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

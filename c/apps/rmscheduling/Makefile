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
	patmos-clang $(CFLAGS) -mserialize=tpip.pml -D WCET rm_scheduling_demo.c rm_minimal_scheduler.c -o rm_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e init_minimal_rmtask
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e create_rmtasknode
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e rmschedule_enqueue
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e rmschedule_sortedinsert_release
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e rmschedule_dequeue
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e minimal_rm_scheduler
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e print_rmschedule

wcet_demo_tasks:
	patmos-clang $(CFLAGS) -mserialize=tpip.pml -D WCET rm_scheduling_demo.c rm_minimal_scheduler.c -o rm_scheduling_demo.elf
	platin wcet --disable-ait -i tpip.pml -b rm_scheduling_demo.elf -e demo_task
	
# compiling examples
force:
	touch rm_scheduling_demo.c

rm_scheduling_demo:
	patmos-clang $(CFLAGS) rm_scheduling_demo.c rm_minimal_scheduler.c -o rm_scheduling_demo.elf

rm_scheduling_demo_debug:
	patmos-clang $(CFLAGS) rm_scheduling_demo.c rm_minimal_scheduler.c -D DEBUG -o rm_scheduling_demo.elf

rm_scheduling_demo_threaded:
	patmos-clang $(CFLAGS) $(LIBCORETHREAD) rm_scheduling_demo_threaded.c rm_minimal_scheduler.c -D THREADED -o rm_scheduling_demo.elf -L$(BUILDDIR) -lcorethread

rm_scheduling_demo_debug_threaded:
	patmos-clang $(CFLAGS) $(LIBCORETHREAD) rm_scheduling_demo_threaded.c rm_minimal_scheduler.c -D THREADED -D DEBUG -o rm_scheduling_demo.elf -L$(BUILDDIR) -lcorethread

# running examples

sim_single:
	pasim -b rm_scheduling_demo.elf

sim_threaded:
	pasim --cores 4 -b rm_scheduling_demo.elf

emu:
	patemu rm_scheduling_demo.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) rm_scheduling_demo.elf

# utilities

clean:
	rm -f *.out *.pcap *.pml *.png *.elf

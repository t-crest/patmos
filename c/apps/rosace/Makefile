PATMOSHOME=../../..
BUILDDIR?=$(PATMOSHOME)/tmp
LIBNOC=$(PATMOSHOME)/c/libnoc
LIBMP=$(PATMOSHOME)/c/libmp
LIBETH=$(PATMOSHOME)/c/ethlib
NOCINIT?=$(PATMOSHOME)/c/cmp/nocinit.c
SERIAL?=/dev/ttyUSB0

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/include  \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBNOC) \
	-I $(LIBMP) \
	-I $(LIBETH) \
	$(DEFINES)

# WCET analysis

# compiling examples
force:
	touch rosace_patmos.c

rosace_wcet:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(NOCINIT) -mserialize=tpip.pml rosace_patmos_cyclic.c io.c sincos_lut.c assemblage_includes.c assemblage.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e h_c0_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e delta_e_c0_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e Vz_control_50483_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e engine486_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e h_filter_100446_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e aircraft_dynamics495_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e q_filter_100455_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e Vz_filter_100452_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e az_filter_100458_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e delta_th_c0_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e altitude_hold_50464_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e Va_c0_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e Va_control_50474_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e elevator489_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e Va_filter_100449_fun

rosace_util_wcet:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(NOCINIT) -mserialize=tpip.pml rosace_patmos_cyclic.c io.c sincos_lut.c assemblage_includes.c assemblage.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e rosace_init
	# platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e logging_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e execute_cyclic_loop

rosace_patmos:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(NOCINIT) rosace_patmos.c io.c sincos_lut.c assemblage_includes.c assemblage.c pthread_barrier.c threads.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

rosace_patmos_debug:
	patmos-clang -D USE_FLOAT -D DEBUG_PRINTS $(CFLAGS) $(NOCINIT) rosace_patmos.c io.c sincos_lut.c assemblage_includes.c assemblage.c pthread_barrier.c threads.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

rosace_patmos_cyclic:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(NOCINIT) rosace_patmos_cyclic.c io.c sincos_lut.c assemblage_includes.c assemblage.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

# running examples
sim:
	pasim --cores 9 --debug-intrs -V -b rosace_patmos.elf 

emu:
	patemu rosace_patmos.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all

download:
	patserdow -v $(SERIAL) rosace_patmos.elf

# utilities
clean:
	rm -f *.out *.pcap *.pml *.png *.elf

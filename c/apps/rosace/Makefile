PATMOSHOME=../../..
BUILDDIR?=$(PATMOSHOME)/tmp
LIBNOC=$(PATMOSHOME)/c/libnoc
LIBMP=$(PATMOSHOME)/c/libmp
LIBETH=$(PATMOSHOME)/c/ethlib
NOCINIT?=$(PATMOSHOME)/c/cmp/nocinit.c
SERIAL?=/dev/ttyUSB0
BOARD?=altde2-all
NODE?=AIRCRAFT_NODE

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O3 \
	-I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/include  \
	-I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBNOC) \
	-I $(LIBMP) \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)
LDFLAGS?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000 \
	-Xgold --defsym -Xgold __heap_end=0x1f0000 \
	-Xgold -T -Xgold $(BUILDDIR)/../hardware/spm_ram.t

# WCET analysis

# compiling examples
force:
	touch rosace_patmos_cmp.c

wcet_rosace_cyclic:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml rosace_patmos_cyclic.c rosace_dist_common.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
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

wcet_rosace_cyclic_common:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml rosace_patmos_cyclic.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e rosace_init
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e copy_output_vars
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e execute_cyclic_loop

wcet_rosace_dist_common:
	patmos-clang -D USE_FLOAT -D AIRCRAFT_NODE $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml $(LIBETH)/*.c helpers/*.c rosace_distributed.c rosace_dist_common.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e reset_sync
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e rosace_init
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e logging_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e execute_cyclic_loop
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e cyclic_dispatcher
	

wcet_aircraft_node:
	patmos-clang -D USE_FLOAT -D AIRCRAFT_NODE $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml $(LIBETH)/*.c helpers/*.c rosace_distributed.c rosace_dist_common.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e sync_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_ctrl_recv_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_dyn_send_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e engine486_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e elevator489_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e aircraft_dynamics495_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e logging_fun

wcet_filter_node:
	patmos-clang -D USE_FLOAT -D FILTER_NODE $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml $(LIBETH)/*.c helpers/*.c rosace_distributed.c rosace_dist_common.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e sync_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_dyn_recv_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_filter_send_fun

wcet_control_node:
	patmos-clang -D USE_FLOAT -D CONTROL_NODE $(CFLAGS) $(LDFLAGS) $(NOCINIT) -mserialize=tpip.pml $(LIBETH)/*.c helpers/*.c rosace_distributed.c rosace_dist_common.c onera/*.c -o rosace_wcet.elf -L$(BUILDDIR) -lm
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e sync_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_filter_recv_fun
	platin wcet --disable-ait -i tpip.pml -b rosace_wcet.elf -e vl_ctrl_send_fun

rosace_patmos_cmp:
	patmos-clang -D USE_FLOAT $(CFLAGS) $(LDFLAGS) $(NOCINIT) helpers/*.c rosace_patmos_cmp.c onera/*.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

rosace_patmos_cyclic:
	patmos-clang -D USE_FLOAT  $(CFLAGS) $(LDFLAGS) $(NOCINIT) $(LIBETH)/*.c helpers/*.c rosace_patmos_cyclic.c onera/*.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

rosace_distributed_node:
	patmos-clang -D USE_FLOAT -D $(NODE) $(CFLAGS) $(LDFLAGS) $(NOCINIT) $(LIBETH)/*.c helpers/*.c rosace_distributed.c rosace_dist_common.c conera/*.c -o rosace_patmos.elf -L$(BUILDDIR) -lm
	
rosace_patmos_argo:
	patmos-clang -D USE_FLOAT  $(CFLAGS) $(LDFLAGS) $(NOCINIT) $(LIBMP)/*.c $(LIBNOC)/*.c $(LIBETH)/*.c helpers/*.c rosace_patmos_argo.c onera/*.c -o rosace_patmos.elf -L$(BUILDDIR) -lm

# running examples
sim:
	pasim --cores 9 --debug-intrs -V -b rosace_patmos.elf 

emu:
	patemu rosace_patmos.elf

config:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=$(BOARD)

download:
	patserdow -v $(SERIAL) rosace_patmos.elf

# utilities
clean:
	rm -f *.out *.pcap *.pml *.png *.elf

CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	-mpatmos-disable-vliw \
	$(DEFINES)
LDFLAGS?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000 \
	-Xgold --defsym -Xgold __heap_end=0x1f0000 \
	-Xgold -T -Xgold ../../../hardware/spm_ram.t

# Current main programs:
# noc_roundtrip_bench
# noc_write_bench
# sspm_locking_bench
# sspm_roundtrip_bench
# sspm_write_bench
# sspm_write_with_lock_contention_bench
#
# can be compiled and downloaded from the Patmos main folder as follows:
# make app download APP=sspm SSPM_MAIN=noc_roundtrip_bench

MAIN?=no_sspm_main_given
all:
	patmos-clang $(CFLAGS) $(LDFLAGS) $(MAIN).c atomic.c led.c -I../.. ../../libcorethread/*.c ../../libnoc/*.c ../../libmp/*.c ../../cmp/nocinit.c -o sspm.elf

down:
	patserdow -v /dev/ttyUSB0 sspm.elf

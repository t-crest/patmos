
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

SSPM_MAIN?=sspm_write_bench
all:
	patmos-clang -O2 $(SSPM_MAIN).c atomic.c -I../.. ../../libcorethread/*.c ../../libnoc/*.c ../../libmp/*.c ../../cmp/nocinit.c -o sspm.elf

down:
	patserdow -v /dev/ttyUSB0 sspm.elf

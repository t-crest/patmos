
SSPM_MAIN?=no_main_provided
all:
	patmos-clang -O2 $(SSPM_MAIN).c -I../.. ../../libcorethread/*.c ../../libnoc/*.c ../../libmp/*.c ../../cmp/nocinit.c atomic.c -o sspm.elf

down:
	patserdow -v /dev/ttyUSB0 sspm.elf


SSPM_MAIN?=first_main
all:
	patmos-clang -O2 $(SSPM_MAIN).c -I../.. ../../libcorethread/*.c ../../libnoc/*.c ../../libmp/*.c ../../cmp/nocinit.c -o bench.elf

down:
	patserdow -v /dev/ttyUSB0 bench.elf

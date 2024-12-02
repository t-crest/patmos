
all:
	patmos-clang -O2 *.c -I../.. ../../libcorethread/*.c ../../libnoc/*.c ../../libmp/*.c -o bench.elf

down:
	patserdow -v /dev/ttyUSB0 bench.elf

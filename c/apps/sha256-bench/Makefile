.PHONY: all
all: io cop

io:
	patmos-clang -O2 sha256_io_benchmark.c -o sha256_io_benchmark.elf

cop:
	patmos-clang -O2 sha256_cop_benchmark.c -o sha256_cop_benchmark.elf
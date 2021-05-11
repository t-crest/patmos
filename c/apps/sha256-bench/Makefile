.PHONY: all
all: compile

.PHONY: clean
clean:
	rm -f *.elf
	rm -f *.stdout

.PHONY: compile
compile: sha256_sw_benchmark.elf sha256_io_benchmark.elf sha256_cop_benchmark.elf

.PHONY: benchmark
benchmark:
	make sw.stdout & make io.stdout & make cop.stdout

sha256_sw_benchmark.elf: sha256_sw_benchmark.c
	patmos-clang -O2 sha256_sw_benchmark.c -o sha256_sw_benchmark.elf
	
sha256_io_benchmark.elf: sha256_io_benchmark.c
	patmos-clang -O2 sha256_io_benchmark.c -o sha256_io_benchmark.elf

sha256_cop_benchmark.elf: sha256_cop_benchmark.c
	patmos-clang -O2 sha256_cop_benchmark.c -o sha256_cop_benchmark.elf

sw.stdout: sha256_sw_benchmark.elf
	patemu sha256_sw_benchmark.elf > sw.stdout

io.stdout: sha256_io_benchmark.elf
	patemu sha256_io_benchmark.elf > io.stdout

cop.stdout: sha256_cop_benchmark.elf
	patemu sha256_cop_benchmark.elf > cop.stdout


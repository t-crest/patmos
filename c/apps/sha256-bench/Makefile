.PHONY: all
all: compile

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.stdout

.PHONY: compile
compile: sw_benchmark.elf io_benchmark.elf cop_benchmark.elf

.PHONY: benchmark
benchmark:
	make sw.stdout & make io.stdout & make cop.stdout

sw_impl.o: sw_impl.c benchmark.h
	patmos-clang sw_impl.c -D BENCHMARK_SW -O2 -c -o sw_impl.o

io_impl.o: io_impl.c benchmark.h
	patmos-clang io_impl.c -D BENCHMARK_IO -O2 -c -o io_impl.o

cop_impl.o: cop_impl.c benchmark.h
	patmos-clang cop_impl.c -D BENCHMARK_COP -O2 -c -o cop_impl.o

sw_benchmark.o: benchmark.c benchmark.h
	patmos-clang benchmark.c -D BENCHMARK_SW -O2 -c -o sw_benchmark.o

io_benchmark.o: benchmark.c benchmark.h
	patmos-clang benchmark.c -D BENCHMARK_IO -O2 -c -o io_benchmark.o

cop_benchmark.o: benchmark.c benchmark.h
	patmos-clang benchmark.c -D BENCHMARK_COP -O2 -c -o cop_benchmark.o

sw_benchmark.elf: sw_impl.o sw_benchmark.o
	patmos-clang sw_impl.o sw_benchmark.o -O2 -o sw_benchmark.elf

io_benchmark.elf: io_impl.o io_benchmark.o
	patmos-clang io_impl.o io_benchmark.o -O2 -o io_benchmark.elf

cop_benchmark.elf: cop_impl.o cop_benchmark.o
	patmos-clang cop_impl.o cop_benchmark.o -O2 -o cop_benchmark.elf

sw.stdout: sw_benchmark.elf
	patemu sw_benchmark.elf > sw.stdout

io.stdout: io_benchmark.elf
	patemu io_benchmark.elf > io.stdout

cop.stdout: cop_benchmark.elf
	patemu cop_benchmark.elf > cop.stdout


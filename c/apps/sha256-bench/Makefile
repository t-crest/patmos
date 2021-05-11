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
	
# old standalone targets
.PHONY: standalone_compile
standalone_compile: standalone_sw_benchmark.elf standalone_io_benchmark.elf standalone_cop_benchmark.elf

.PHONY: standalone_benchmark
standalone_benchmark:
	make standalone_sw.stdout & make standalone_io.stdout & make standalone_cop.stdout

standalone_sw_benchmark.elf: standalone_sw_benchmark.c
	patmos-clang -O2 standalone_sw_benchmark.c -o standalone_sw_benchmark.elf
	
standalone_io_benchmark.elf: standalone_io_benchmark.c
	patmos-clang -O2 standalone_io_benchmark.c -o standalone_io_benchmark.elf

standalone_cop_benchmark.elf: standalone_cop_benchmark.c
	patmos-clang -O2 standalone_cop_benchmark.c -o standalone_cop_benchmark.elf

standalone_sw.stdout: standalone_sw_benchmark.elf
	patemu standalone_sw_benchmark.elf > standalone_sw.stdout

standalone_io.stdout: standalone_io_benchmark.elf
	patemu standalone_io_benchmark.elf > standalone_io.stdout

standalone_cop.stdout: standalone_cop_benchmark.elf
	patemu standalone_cop_benchmark.elf > standalone_cop.stdout


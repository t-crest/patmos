# Default target
all: hello_example

# Build the hello-noc example
hello_example:
	patmos-clang -O2 -DHELLO_NOC hello-noc.c -o s4noc.elf

# Build the bench-noc example
bench_example:
	patmos-clang -O2 -DBENCH_NOC bench-noc.c -o s4noc.elf

# Clean up
clean:
	rm -f s4noc.elf

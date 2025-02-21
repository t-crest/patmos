# Default target
EXAMPLE?=hello
all: $(EXAMPLE)_example

# Build the hello-noc example
hello_example:
	patmos-clang -O2 hello-noc.c -o s4noc.elf

# Build the bench-noc example
bench_example:
	patmos-clang -O2 bench-noc.c -o s4noc.elf

reactor_example:
	patmos-clang -O2 reactor.c -o s4noc.elf

# Clean up
clean:
	rm -f s4noc.elf

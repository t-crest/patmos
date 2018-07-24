# This is the trivial Hello World example, distributed over two .c files

LDFLAGS?= \
        -mpatmos-method-cache-size=0x1000 \
        -mpatmos-stack-base=0x080000 -mpatmos-shadow-stack-base=0x078000 \
        -Xgold --defsym -Xgold __heap_end=0x070000

all:
	patmos-clang -I ../.. -O2 $(LDFLAGS) *.c -o de10-nano.elf


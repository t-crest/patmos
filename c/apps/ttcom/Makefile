MAIN?=mp_pc

all:
	patmos-clang -O2 $(MAIN).c \
		-I../.. \
		../../libcorethread/*.c \
		../../libnoc/*.c \
		../../libmp/*.c \
		../../cmp/nocinit.c \
		-o $(APP).elf $(COPTS)

wcet:
	patmos-clang -O2 -mserialize=$(MAIN).pml $(MAIN).c \
	   -I ../.. \
	   ../../libcorethread/*.c \
	   ../../libnoc/*.c \
	   ../../libmp/*.c \
	   ../../cmp/nocinit.c $(COPTS)
	platin wcet -i $(MAIN).pml -b a.out -e wcetConsumerCompute --report


clean:
	- rm a.out
	- rm *.pml
	- rm *.out
	- rm *.png
	- rm *.asm
	- rm *.elf


# Following commands are optional/unused

show: all
	platin wcet -i $(MAIN).pml -b a.out -e foo -o wcet.pml
	platin visualize -i wcet.pml --show-timings=platin

dump:
	patmos-llvm-objdump -d a.out > x.asm

gen_pml:
	platin pml-config --target patmos-unknown-unknown-elf -o de2-115.pml



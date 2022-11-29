# A simple Hello World WCET analysis example

all:
	patmos-clang -O2 -mserialize=simple.pml simple.c
	platin wcet -i simple.pml -b a.out -e foo --report


clean:
	- rm a.out
	- rm *.pml
	- rm *.out
	- rm *.png
	- rm *.asm


# Following comannds are optional/unused

show: all
	platin wcet -i simple.pml -b a.out -e foo -o wcet.pml
	platin visualize -i wcet.pml -f foo --show-timings=platin

dump:
	patmos-llvm-objdump -d a.out > x.asm

gen_pml:
	platin pml-config --target patmos-unknown-unknown-elf -o de2-115.pml



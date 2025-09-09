all: clean move
	
compile:
	patmos-clang -O2 main.c
	patmos-llvm-objdump -rd a.out > a.asm
	
move: compile
	cp a.out ../../../tmp/l2-scheduler-test.elf
	
clean: 
	rm -f a.out
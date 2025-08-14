
TARGET_ADDR_MIN=786432
TARGET_ADDR_MAX=1835008
WINDOW_SIZE=10000

all: clean move

gen: generate.py
	python3 generate.py $(TARGET_ADDR_MIN) $(TARGET_ADDR_MAX) $(WINDOW_SIZE)
	
main.o: main.c gen
	patmos-clang -O2 main.c ../../libcorethread/corethread.c
	patmos-llvm-objdump -rd a.out > a.asm
	
move: main.o
	cp a.out ../../../tmp/cont-test.elf
	
clean: 
	rm -f a.out
	rm -f *.txt.c
APP?=htmrts
MAIN?=queue_test

all:
	patmos-clang -O2 -mserialize=$(APP).pml $(MAIN).c -I ../.. ../../libcorethread/*.c -o $(APP).elf $(COPTS)
	
wcet:
	platin pml-config --target patmos-unknown-unknown-elf -o config.pml
	platin wcet --disable-ait --enable-wca -i $(APP).pml -i config.pml -b $(APP).elf -e enqueue
	platin wcet --disable-ait --enable-wca -i $(APP).pml -i config.pml -b $(APP).elf -e dequeue
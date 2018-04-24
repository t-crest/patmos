MAIN?=nocontention_test
#MAIN?=contention_test

all:
	patmos-clang -O2 $(MAIN).c -I ../.. ../../libcorethread/*.c ../sspm/atomic.c -o $(APP).elf $(COPTS)

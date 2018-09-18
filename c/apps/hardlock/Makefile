MAIN?=nocontention_test
#MAIN?=contention_test

all:
	patmos-clang -O2 $(MAIN).c -I ../.. ../../libcorethread/*.c -o $(APP).elf $(COPTS)

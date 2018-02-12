MAIN?=hardlock_nocontention_test
#MAIN?=hardlock_contention_test
#MAIN?=sspm_contention_test

all:
	patmos-clang -O2 $(MAIN).c -I ../.. ../../libcorethread/*.c ../sspm/atomic.c -o $(APP).elf

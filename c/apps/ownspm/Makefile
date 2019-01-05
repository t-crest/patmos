MAIN?=pc

all:
	patmos-clang -O2 $(MAIN).c -I ../.. -I ../../include ../../libcorethread/*.c -o $(APP).elf $(COPTS)

clean:
	rm *.elf

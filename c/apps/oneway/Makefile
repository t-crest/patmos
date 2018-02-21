MAIN?=hello_oneway

all:
	patmos-clang -O2 $(MAIN).c -I ../.. ../../libcorethread/*.c -o $(APP).elf

clean:
	rm *.elf

#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/
# following is not supported anymore in LLVM 3.2
# -mfloat-abi=none \

#CC=$(PATMOS_BASE)/clang
CC=patmos-clang
CFLAGS=-target patmos-unknown-unknown-elf \
	-nodefaultlibs -nostdlib -nostartfiles \
	-fno-builtin \
	-Ttext 0x10 \
	-e main
CFLAGS+=-mpatmos-disable-stack-cache \
	-O3
#CFLAGS+=-O0

CFLAGS-WITH-LIBS=-target patmos-unknown-unknown-elf \
	-nostartfiles \
	-fno-builtin \
	-mpatmos-disable-stack-cache \
	-Ttext 0x10 \
	-e main \
	-O3

CFLAGS_ASM=-target patmos-unknown-unknown-elf \
	-fno-builtin \
        -fpatmos-emit-asm -S
CFLAGS_ASM+=-mpatmos-disable-stack-cache \
	-O3
#CFLAGS_ASM+=-O0

%: %.c
	$(CC) $(CFLAGS) -o $@ $^
	patmos-llvm-objdump -t -d $@

%.s: %.c
	$(CC) $(CFLAGS_ASM) -o $@ $^

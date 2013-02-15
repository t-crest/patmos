#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/

#CC=$(PATMOS_BASE)/clang
CC=patmos-clang
CFLAGS=-target patmos-unknown-unknown-elf \
	-nodefaultlibs -nostdlib -nostartfiles \
	-fno-builtin -mfloat-abi=none \
	-Ttext 0x10 \
	-e main
# Edgar2MS: the comments didn't work because all have to be one line, so the -Ttext and -e main was skipped which caused the elf2vhdl to crash
ifeq ($(DISABLE_SC),y)
CFLAGS+=-mpatmos-disable-stack-cache \
	-O3
else
CFLAGS+=-O0
endif

CFLAGS-WITH-LIBS=-target patmos-unknown-unknown-elf \
	-nostartfiles \
	-fno-builtin -mfloat-abi=none \
	-mpatmos-disable-stack-cache \
	-Ttext 0x10 \
	-e main \
	-O3

CFLAGS_ASM=-target patmos-unknown-unknown-elf \
	-fno-builtin -mfloat-abi=none \
        -fpatmos-emit-asm -S
ifeq ($(DISABLE_SC),y)
CFLAGS_ASM+=-mpatmos-disable-stack-cache \
	-O3
else
CFLAGS_ASM+=-O0
endif

%: %.c
	$(CC) $(CFLAGS) -o $@ $^
	patmos-llvm-objdump -t -d $@

%.s: %.c
	$(CC) $(CFLAGS_ASM) -o $@ $^

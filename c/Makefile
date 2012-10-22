#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/

#CC=$(PATMOS_BASE)/clang
CC=patmos-clang
CFLAGS=-ccc-host-triple patmos-unknown-elf \
	-nodefaultlibs -nostdlib -nostartfiles \
	-fno-builtin -mfloat-abi=none \
	-mpatmos-disable-stack-cache \
	-Ttext 0x10 \
	-e main \
	-O3

CFLAGS-WITH-LIBS=-ccc-host-triple patmos-unknown-elf \
	-nostartfiles \
	-fno-builtin -mfloat-abi=none \
	-mpatmos-disable-stack-cache \
	-Ttext 0x10 \
	-e main \
	-O3
%: %.c
	$(CC) $(CFLAGS) -o $@ $^
	patmos-llvm-objdump -d $@

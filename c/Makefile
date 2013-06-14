BUILDDIR=../tmp

#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/
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

CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf \
	-Xgold -T../chisel/spm.t \
	-mpatmos-method-cache-size=0x8000 \
	-mpatmos-disable-stack-cache \
	-O3

compile: $(BUILDDIR)/$(APP).elf

$(BUILDDIR)/%.elf: %.c
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.s: %.c
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $^

$(BUILDDIR)/bootable-%.elf: %.c
	$(CC) $(CFLAGS-BOOTABLE) -o $(BUILDDIR)/$(APP).elf $^

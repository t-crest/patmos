BUILDDIR=../tmp

#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/
#CC=$(PATMOS_BASE)/clang

CC=patmos-clang

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf \
	-DBOOTROM \
	-mpatmos-method-cache-size=0x8000 \
	-mpatmos-disable-stack-cache \
	-nodefaultlibs -nostdlib -nostartfiles \
	-mpatmos-stack-base=0x80013000 -mpatmos-shadow-stack-base=0x80014000 \
	-Xgold --section-start -Xgold .text=0x0 -e main \
	-Xgold --section-start -Xgold .rodata=0x80000010 \
	-Xgold --section-start -Xgold .data=0x80010000 \
	-Os

# Flags for compiling normal applications
CFLAGS=-target patmos-unknown-unknown-elf \
	-Xgold -T../chisel/ram.t \
	-mpatmos-method-cache-size=0x100 \
	-mpatmos-stack-base=0x1f0000 -mpatmos-shadow-stack-base=0x200000 \
	-nolibsyms \
	-O2

compile: $(BUILDDIR)/$(APP).elf

$(BUILDDIR)/%.elf: %.c
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.s: %.c
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $^

$(BUILDDIR)/bootable-%.elf: %.c
	$(CC) $(CFLAGS-BOOTABLE) -o $@ $^

$(BUILDDIR)/bootable-%.s: %.c
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $^


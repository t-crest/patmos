BUILDDIR=../tmp

#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/
#CC=$(PATMOS_BASE)/clang

CC=patmos-clang

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf \
	-mpatmos-method-cache-size=0x8000 \
	-mpatmos-disable-stack-cache \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e main \
	-Os

# Flags for compiling normal applications
CFLAGS=-target patmos-unknown-unknown-elf \
	-Xgold -T../chisel/spm.t \
	-mpatmos-method-cache-size=0x8000 \
	-mpatmos-disable-stack-cache \
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


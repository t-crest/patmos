BUILDDIR=../tmp

#PATMOS_BASE=/project/flbr/jobs/install/patmos/bin/
#CC=$(PATMOS_BASE)/clang

CC=patmos-clang

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mpatmos-disable-stack-cache \
	-mpatmos-disable-vliw=true
LDFLAGS-BOOTABLE= \
	-mpatmos-method-cache-size=0x8000 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-subfunction-align=4 \
	-mpatmos-stack-base=0x80010680 -mpatmos-shadow-stack-base=0x80010780 \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e main \
	-Xgold --section-start -Xgold .rodata=0x80000010 \
	-Xgold --section-start -Xgold .data=0x80010000 \

# Flags for compiling normal applications
CFLAGS=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR)
LDFLAGS= \
	-mpatmos-method-cache-size=0x800 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-stack-base=0x1f0000 -mpatmos-shadow-stack-base=0x200000 \
	-Xgold -T../hardware/ram.t -Xgold -n

compile: $(BUILDDIR)/$(APP).elf

$(BUILDDIR)/%.o: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -c -o $@ $(filter %.c,$^)

$(BUILDDIR)/%.elf: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.s,$^)

$(BUILDDIR)/%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

$(BUILDDIR)/bootable-%.elf: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) $(LDFLAGS-BOOTABLE) -o $@ $(filter %.c %.s,$^)

$(BUILDDIR)/bootable-%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: download.c boot.h
$(BUILDDIR)/bootable-cmpmaster.elf: download.c boot.h cmpboot.h
$(BUILDDIR)/bootable-cmpslave.elf: boot.h cmpboot.h

$(BUILDDIR)/bootable-mandelbrot_par.elf: init.h
$(BUILDDIR)/mandelbrot_par.elf: init.h

BUILDDIR=../tmp

CC=patmos-clang

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mpatmos-disable-stack-cache \
	-mpatmos-disable-vliw
LDFLAGS-BOOTABLE= \
	-mpatmos-method-cache-size=0x8000 \
	-mpatmos-max-subfunction-size=0 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-stack-base=0x80010680 -mpatmos-shadow-stack-base=0x80010780 \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e main \
	-Xgold --section-start -Xgold .rodata=0x80000010 \
	-Xgold --section-start -Xgold .data=0x80010000 \
	-Xllc -disable-machine-licm

# Flags for compiling normal applications
# Now with assumed 1 MB memory for the ml605 on-chip tests
CFLAGS=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) \
	-mpatmos-disable-vliw
LDFLAGS= \
	-mpatmos-method-cache-size=0x800 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-stack-base=0x0f0000 -mpatmos-shadow-stack-base=0x100000 \
	-Xgold -T../hardware/ram.t -Xgold -n

compile: $(BUILDDIR)/$(APP).elf

$(BUILDDIR)/%.o: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -c -o $@ $(filter %.c,$^)

$(BUILDDIR)/%.elf: %.c libnoc
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lnoc

$(BUILDDIR)/%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

$(BUILDDIR)/bootable-%.elf: %.c libnoc
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) $(LDFLAGS-BOOTABLE) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lnoc

$(BUILDDIR)/bootable-%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: download.c boot.h
$(BUILDDIR)/bootable-cmpmaster.elf: download.c boot.h cmpboot.h
$(BUILDDIR)/bootable-cmpslave.elf: boot.h cmpboot.h

$(BUILDDIR)/bootable-mandelbrot_par.elf: nocinit.c libnoc
$(BUILDDIR)/mandelbrot_par.elf: nocinit.c libnoc

$(BUILDDIR)/bootable-cmp_hello.elf: nocinit.c libnoc
$(BUILDDIR)/cmp_hello.elf: nocinit.c libnoc

$(BUILDDIR)/hello_sum.elf: nocinit.c libnoc boot.h cmpboot.h

# library for network-on-chip
.PHONY: libnoc
libnoc: $(BUILDDIR)/libnoc.a
$(BUILDDIR)/libnoc.a: $(BUILDDIR)/libnoc/noc.o
	ar r $@ $^

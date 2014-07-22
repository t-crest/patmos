# Include user makefile for local configurations
-include config.mk

BUILDDIR=../tmp

CC=patmos-clang

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mpatmos-disable-stack-cache \
	-mpatmos-disable-function-splitter \
	-mpatmos-disable-vliw
LDFLAGS-BOOTABLE= \
	-mpatmos-stack-base=0x80010700 -mpatmos-shadow-stack-base=0x80010800 \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e main \
	-Xgold --section-start -Xgold .rodata=0x80000010 \
	-Xgold --section-start -Xgold .data=0x80010000 \
	-Xllc -mpatmos-subfunction-align=0 \
	-Xllc -disable-machine-licm

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) \
	-mpatmos-disable-vliw
LDFLAGS?= \
	-mpatmos-method-cache-size=0x800 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000

compile: $(BUILDDIR)/$(APP).elf

$(BUILDDIR)/%.o: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -c -o $@ $(filter %.c,$^)

$(BUILDDIR)/%.elf: %.c libnoc libmp
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lmp -lnoc 
$(BUILDDIR)/%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

$(BUILDDIR)/bootable-%.elf: %.c libnoc
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) $(LDFLAGS-BOOTABLE) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lmp -lnoc

$(BUILDDIR)/bootable-%.s: %.c
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: download.c boot.h bootable.h
$(BUILDDIR)/bootable-cmpmaster.elf: download.c boot.h cmpboot.h bootable.h
$(BUILDDIR)/bootable-cmpslave.elf: boot.h cmpboot.h bootable.h

$(BUILDDIR)/bootable-mandelbrot.elf: bootable.h
$(BUILDDIR)/mandelbrot.elf: bootable.h

$(BUILDDIR)/bootable-mandelbrot_par.elf: nocinit.c libnoc bootable.h
$(BUILDDIR)/mandelbrot_par.elf: nocinit.c libnoc bootable.h

$(BUILDDIR)/bootable-cmp_hello.elf: nocinit.c libnoc bootable.h
$(BUILDDIR)/cmp_hello.elf: nocinit.c libnoc bootable.h

$(BUILDDIR)/bootable-noctest.elf: nocinit.c libnoc bootable.h
$(BUILDDIR)/noctest.elf: nocinit.c libnoc bootable.h

$(BUILDDIR)/bootable-extdev_test.elf: bootable.h
$(BUILDDIR)/extdev_test.elf: bootable.h

$(BUILDDIR)/hello_sum.elf: nocinit.c libnoc
$(BUILDDIR)/bootable-hello_sum_reduced.elf: nocinit.c libnoc bootable.h
$(BUILDDIR)/matrix_mult.elf: nocinit.c libnoc

$(BUILDDIR)/test_cmp.elf: nocinit.c libnoc

$(BUILDDIR)/bootable-mp_test.elf: nocinit.c libnoc libmp bootable.h
$(BUILDDIR)/mp_test.elf: nocinit.c libmp

# library for network-on-chip
.PHONY: libnoc
libnoc: $(BUILDDIR)/libnoc.a
$(BUILDDIR)/libnoc.a: $(BUILDDIR)/libnoc/noc.o
	ar r $@ $^

# library for message passing
.PHONY: libmp
libmp: libnoc $(BUILDDIR)/libmp.a
$(BUILDDIR)/libmp.a: $(BUILDDIR)/libmp/mp.o
	ar r $@ $^

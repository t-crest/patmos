CC=patmos-clang # variable CC is "magic", so we define it here to a sane default

# Include user makefile for local configurations
-include config.mk

BUILDDIR?=../tmp
LIBNOC=$(BUILDDIR)/libnoc.a
LIBMP=$(BUILDDIR)/libmp.a
LIBCORETHREAD=$(BUILDDIR)/libcorethread.a
LIBETH=$(BUILDDIR)/libeth.a
LIBELF=$(BUILDDIR)/libelf.a

NOCINIT?=nocinit.c

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE?=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mpatmos-disable-stack-cache \
	-mpatmos-disable-function-splitter \
	-mpatmos-disable-vliw
LDFLAGS-BOOTABLE?= \
	-mpatmos-stack-base=0x00020800 -mpatmos-shadow-stack-base=0x00020700 \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e _start \
	-Xgold --section-start -Xgold .rodata=0xf0008010 \
	-Xgold --section-start -Xgold .data=0x00020000 \
	-Xllc -mpatmos-subfunction-align=0 \
	-Xllc -disable-machine-licm -Xllc -disable-machine-cse

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	-mpatmos-disable-vliw
LDFLAGS?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000 \
	-Xgold -T -Xgold $(BUILDDIR)/../hardware/spm_ram.t

# 32KB stacks for 256MB external memory
# -mpatmos-stack-base=0x10000000 -mpatmos-shadow-stack-base=0x0fff8000

.PHONY: compile
compile: $(BUILDDIR)/$(APP).elf

VPATH = .:bootable:bootloader:cmp:dhry:include

$(BUILDDIR)/%.o: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -c -o $@ $(filter %.c,$^)

# A target for regular applications
$(BUILDDIR)/%.elf: %.c $(NOCINIT) $(LIBMP) $(LIBNOC) $(LIBCORETHREAD) $(LIBETH) $(LIBELF) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lmp -lnoc -lcorethread -leth -lelf

$(BUILDDIR)/%.s: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# A target for bootable applications (e.g., the bootloader)
$(BUILDDIR)/bootable-%.elf: %.c $(LIBETH) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) $(LDFLAGS-BOOTABLE) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -leth

$(BUILDDIR)/bootable-%.s: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: download.c decompress.c ethmac.c boot.h bootable.h
$(BUILDDIR)/bootable-cmpmaster.elf: download.c decompress.c ethmac.c boot.h cmpboot.h bootable.h
$(BUILDDIR)/bootable-cmpslave.elf: boot.h cmpboot.h bootable.h

$(BUILDDIR)/bootable-mandelbrot.elf: bootable.h
$(BUILDDIR)/mandelbrot.elf: bootable.h

$(BUILDDIR)/bootable-mandelbrot_par.elf: bootable.h
$(BUILDDIR)/mandelbrot_par.elf: bootable.h

$(BUILDDIR)/bootable-cmp_hello.elf: bootable.h
$(BUILDDIR)/cmp_hello.elf: bootable.h

$(BUILDDIR)/bootable-noctest.elf: bootable.h
$(BUILDDIR)/noctest.elf: bootable.h

$(BUILDDIR)/bootable-extdev_test.elf: bootable.h
$(BUILDDIR)/extdev_test.elf: bootable.h

$(BUILDDIR)/bootable-hello_sum_reduced.elf: bootable.h

$(BUILDDIR)/bootable-mp_test.elf: bootable.h

# library for network-on-chip
.PHONY: libnoc
libnoc: $(LIBNOC)
$(BUILDDIR)/libnoc/noc.o: libnoc/noc.h libnoc/coreset.h
$(LIBNOC): $(BUILDDIR)/libnoc/noc.o
	patmos-ar r $@ $^

# library for message passing
.PHONY: libmp
libmp: libnoc $(LIBMP)
$(BUILDDIR)/libmp/mp.o: libmp/mp.h libnoc/noc.h libnoc/coreset.h bootloader/cmpboot.h
$(LIBMP): $(BUILDDIR)/libmp/mp.o
	patmos-ar r $@ $^

# library for corethreads
.PHONY: libcorethread
libcorethread: cmpboot.h $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: libcorethread/corethread.h bootloader/cmpboot.h
$(LIBCORETHREAD): $(BUILDDIR)/libcorethread/corethread.o
	patmos-ar r $@ $^

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

# library for parsing ELF files
.PHONY: libelf
libelf: $(LIBELF)
$(LIBELF): $(patsubst libelf/%.c,$(BUILDDIR)/libelf/%.o,$(wildcard libelf/*.c))
	patmos-ar r $@ $^

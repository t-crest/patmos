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

SHELL=/bin/bash
#BOOTSPM data is by default in data SPM 
BOOTSPM_BASE?=0x00020000

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE?=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mpatmos-disable-stack-cache \
	-mpatmos-disable-function-splitter \
	-mpatmos-disable-vliw
LDFLAGS-BOOTABLE?= \
	-mpatmos-stack-base=$$(( $(BOOTSPM_BASE) + 0x800 )) \
	-mpatmos-shadow-stack-base=$$(( $(BOOTSPM_BASE) + 0x700 )) \
	-nodefaultlibs -nostdlib -nostartfiles \
	-Xgold --section-start -Xgold .text=0x0 -e _start \
	-Xgold --section-start -Xgold .rodata=0xf0008010 \
	-Xgold --section-start -Xgold .data=$(BOOTSPM_BASE) \
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
	-Xgold --defsym -Xgold __heap_end=0x1f0000 \
	-Xgold -T -Xgold $(BUILDDIR)/../hardware/spm_ram.t

# Flags for compiling applications that use virtual memory
# Assuming 2 MB of main memory
CFLAGS-VM?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	-mpatmos-disable-vliw -Xllc --code-model=large
LDFLAGS-VM?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x80020000 -mpatmos-shadow-stack-base=0x80010000 \
	-Xgold --defsym -Xgold __heap_end=0x60100000 \
	-Xgold -T -Xgold $(BUILDDIR)/../hardware/vm.t

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
$(BUILDDIR)/bootable-%.elf: %.c $(LIBETH) $(LIBNOC) $(NOCINIT) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) $(LDFLAGS-BOOTABLE) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -leth -lnoc

$(BUILDDIR)/bootable-%.s: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# A target for applications that use virtual memory
$(BUILDDIR)/vm-%.elf: %.c $(NOCINIT) $(LIBMP) $(LIBNOC) $(LIBCORETHREAD) $(LIBETH) $(LIBELF) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-VM) $(LDFLAGS-VM) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lmp -lnoc -lcorethread -leth -lelf

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: download.c decompress.c ethmac.c boot.h bootable.h
$(BUILDDIR)/bootable-cmpmaster.elf: download.c decompress.c ethmac.c boot.h bootable.h
$(BUILDDIR)/bootable-cmpslave.elf: boot.h bootable.h

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

# make sure the initialization file for the network-on-chip is there
$(NOCINIT):
	touch $@

# library for network-on-chip
.PHONY: libnoc
libnoc: $(LIBNOC)
$(BUILDDIR)/libnoc/noc.o: libnoc/noc.h libnoc/coreset.h
$(LIBNOC): $(BUILDDIR)/libnoc/noc.o
	patmos-ar r $@ $^

# library for message passing
.PHONY: libmp
libmp: libnoc $(LIBMP)
$(BUILDDIR)/libmp/mp.o: libmp/mp.h libnoc/noc.h libnoc/coreset.h
$(LIBMP): $(BUILDDIR)/libmp/mp.o
	patmos-ar r $@ $^

# library for corethreads
.PHONY: libcorethread
libcorethread: $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: libcorethread/corethread.h
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

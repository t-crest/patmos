CC=patmos-clang # variable CC is "magic", so we define it here to a sane default
LD=patmos-ld.lld
OBJDUMP=patmos-llvm-objdump


# Include user makefile for local configurations
-include config.mk

BUILDDIR?=../tmp
LIBNOC=$(BUILDDIR)/libnoc.a
LIBMP=$(BUILDDIR)/libmp.a
LIBCORETHREAD=$(BUILDDIR)/libcorethread.a
LIBETH=$(BUILDDIR)/libeth.a
LIBAUDIO=$(BUILDDIR)/libaudio.a
LIBELF=$(BUILDDIR)/libelf.a
LIBSD=$(BUILDDIR)/libsd.a

NOCINIT?=cmp/nocinit.c

SHELL=/bin/bash
#BOOTSPM data is by default in data SPM
BOOTSPM_BASE?=0x00020000

# Flags for compiling to boot ROM
CFLAGS-BOOTABLE?=-target patmos-unknown-unknown-elf -Os \
	-I$(CURDIR) -DBOOTROM \
	-mllvm --mpatmos-disable-stack-cache \
	-mllvm --mpatmos-disable-function-splitter \
	-mllvm --mpatmos-disable-vliw \
	-mllvm --mpatmos-subfunction-align=1 \
	-mllvm --disable-machine-licm -mllvm --disable-machine-cse -fno-jump-tables
LDFLAGS-BOOTABLE?= \
	--defsym _stack_cache_base=$$(( $(BOOTSPM_BASE) + 0x800 ))\
	--defsym _shadow_stack_base=$$(( $(BOOTSPM_BASE) + 0x700 ))\
	--nostdlib --static -O0 \
	--defsym __heap_start=end --defsym __heap_end=0x100000 \
	--section-start .text=0x0 -e _start \
	--section-start .rodata=0xf0008010 \
	--section-start .data=$(BOOTSPM_BASE)

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	$(DEFINES) -fno-jump-tables
LDFLAGS?= \
	-Wl,--defsym,__heap_end=0x1f0000 \
	-Wl,-T,$(BUILDDIR)/../hardware/spm_ram.t

# Flags for compiling applications that use virtual memory
# Assuming 2 MB of main memory
CFLAGS-VM?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	-mllvm --mpatmos-disable-vliw -mllvm --code-model=large -fno-jump-tables
LDFLAGS-VM?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x80020000 -mpatmos-shadow-stack-base=0x80010000 \
	-Xgold --defsym -Xgold __heap_end=0x60100000 \
	-Xgold -T -Xgold $(BUILDDIR)/../hardware/vm.t

# 32KB stacks for 256MB external memory
# -mpatmos-stack-base=0x10000000 -mpatmos-shadow-stack-base=0x0fff8000

.PHONY: compile
compile: $(BUILDDIR)/$(APP).elf

VPATH = .:bootable:bootloader:cmp:dhry:include:audio_apps

$(BUILDDIR)/%.o: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -c -o $@ $(filter %.c,$^)

# A target for regular applications
$(BUILDDIR)/%.elf: %.c $(NOCINIT) $(LIBMP) $(LIBNOC) $(LIBCORETHREAD) $(LIBETH) $(LIBSD) $(LIBAUDIO) $(LIBELF) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c %.s,$^) -Wl,-L$(BUILDDIR),-lmp,-lnoc,-lcorethread,-leth,-lelf,-lsd,-laudio

$(BUILDDIR)/%.s: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS) -fpatmos-emit-asm -S -o $@ $(filter %.c,$^)

# A target for bootable applications (e.g., the bootloader)
$(BUILDDIR)/%-bootable.o: %.c Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-BOOTABLE) -c -o $@ $(filter %.c,$^)

$(BUILDDIR)/bootable-%.elf: $(BUILDDIR)/%-bootable.o $(LIBETH) $(LIBNOC) $(NOCINIT) Makefile
	$(LD) $(LDFLAGS-BOOTABLE) -o $@ $(filter %-bootable.o,$^) -L$(BUILDDIR) -leth -lnoc

$(BUILDDIR)/bootable-%.s: $(BUILDDIR)/bootable-%.elf Makefile
	$(OBJDUMP) -dr $(filter %.elf,$^) > $@ 

# A target for applications that use virtual memory
$(BUILDDIR)/vm-%.elf: %.c $(NOCINIT) $(LIBMP) $(LIBNOC) $(LIBCORETHREAD) $(LIBETH) $(LIBELF) Makefile
	mkdir -p $(BUILDDIR)/$(dir $*)
	$(CC) $(CFLAGS-VM) $(LDFLAGS-VM) -o $@ $(filter %.c %.s,$^) -L$(BUILDDIR) -lmp -lnoc -lsd -lcorethread -leth -lelf

# application-specific additional dependencies
$(BUILDDIR)/bootable-bootloader.elf: $(BUILDDIR)/bootloader/download-bootable.o $(BUILDDIR)/bootloader/decompress-bootable.o  $(BUILDDIR)/bootloader/ethmac-bootable.o boot.h bootable.h

# touch the nocinit file
$(NOCINIT):
	touch $@

# library for network-on-chip
.PHONY: libnoc
libnoc: $(LIBNOC)
$(BUILDDIR)/libnoc/noc.o: libnoc/noc.h libnoc/coreset.h
$(LIBNOC): $(BUILDDIR)/libnoc/noc.o
	ar -sr $@ $^

# library for message passing
.PHONY: libmp
libmp: libnoc $(LIBMP)
$(BUILDDIR)/libmp/utils.o: libmp/mp.h libmp/mp_internal.h
$(BUILDDIR)/libmp/mp.o: libmp/mp.h libmp/mp_internal.h libnoc/noc.h libnoc/coreset.h
$(BUILDDIR)/libmp/queuing.o: libmp/mp.h libmp/mp_internal.h libnoc/noc.h
$(BUILDDIR)/libmp/sampling.o: libmp/mp.h libmp/mp_internal.h libnoc/noc.h
$(BUILDDIR)/libmp/lock.o: libmp/mp.h libmp/mp_internal.h libnoc/noc.h
$(BUILDDIR)/libmp/collective.o: libmp/mp.h libmp/mp_internal.h libnoc/noc.h libnoc/coreset.h
$(LIBMP): $(BUILDDIR)/libmp/utils.o $(BUILDDIR)/libmp/mp.o $(BUILDDIR)/libmp/queuing.o $(BUILDDIR)/libmp/sampling.o $(BUILDDIR)/libmp/lock.o
	ar -sr $@ $^

# library for corethreads
.PHONY: libcorethread
libcorethread: $(LIBCORETHREAD)
$(BUILDDIR)/libcorethread/corethread.o: libcorethread/corethread.h
$(LIBCORETHREAD): $(BUILDDIR)/libcorethread/corethread.o
	ar -sr $@ $^

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	ar -sr $@ $^

#library for audio
.PHONY: libaudio
libaudio: $(LIBAUDIO)
$(BUILDDIR)/libaudio/dsp_algorithms.o: libaudio/dsp_algorithms.h
$(BUILDDIR)/libaudio/audio.o: libaudio/audio.h libaudio/audioinit.h libaudio/latencyinit.h
$(LIBAUDIO): $(BUILDDIR)/libaudio/dsp_algorithms.o $(BUILDDIR)/libaudio/audio.o
	ar -sr $@ $^

# library for parsing ELF files
.PHONY: libelf
libelf: $(LIBELF)
$(LIBELF): $(patsubst libelf/%.c,$(BUILDDIR)/libelf/%.o,$(wildcard libelf/*.c))
	ar -sr $@ $^

# library for using SD card
.PHONY: libsd
libsd: $(LIBSD)
$(LIBSD): $(patsubst libsd/%.c,$(BUILDDIR)/libsd/%.o,$(wildcard libsd/*.c))
	ar -sr $@ $^

CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I$(CURDIR) -I $(CURDIR)/libelf/ \
	-mpatmos-disable-vliw \
	$(DEFINES)
LDFLAGS?= \
	-mpatmos-method-cache-size=0x1000 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000 \
	-Xgold --defsym -Xgold __heap_end=0x1f0000 \
	-Xgold -T -Xgold ../../../hardware/spm_ram.t

MAIN=hardlock_nocontention_test
#MAIN=hardlock_contention_test
#MAIN=sspm_contention_test

all:
	patmos-clang $(CFLAGS) $(LDFLAGS) $(MAIN).c -I../.. ../../libcorethread/*.c -I../.. ../../libsspm/*.c -o $(APP).elf

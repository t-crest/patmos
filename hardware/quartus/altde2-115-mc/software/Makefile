BINARY  = main.elf
CC      = patmos-clang

INCLUDE_PATHS+= -I.
INCLUDE_PATHS+= -I./components/biss 
#INCLUDE_PATHS+= -I./components/dspba 
INCLUDE_PATHS+= -I./components 
#INCLUDE_PATHS+= -I./hwlib/include 
INCLUDE_PATHS+= -I./mc 
#INCLUDE_PATHS+= -I./perf 
INCLUDE_PATHS+= -I./platform/common 
#INCLUDE_PATHS+= -I./DOC_DE2115_bsp
#INCLUDE_PATHS+= -I./DOC_DE2115_bsp/HAL/inc
#INCLUDE_PATHS+= -I./DOC_DE2115_bsp/drivers/inc

CFLAGS?=-target patmos-unknown-unknown-elf -O3 \
	$(INCLUDE_PATHS) \
	-DSYSTEM_BUS_WIDTH=32 \
	-mpatmos-disable-vliw
LDFLAGS?= \
	-mpatmos-method-cache-size=0x2000 -mpatmos-preferred-subfunction-size=0 \
	-mpatmos-stack-base=0x200000 -mpatmos-shadow-stack-base=0x1f8000 -lm


FILES   = $(shell find . \( ! -regex '.*/\..*' \) -type f -name "*.c")
HEADERS = $(shell find . \( ! -regex '.*/\..*' \) -type f -name "*.h")
OBJS    = $(FILES:.c=.o)


 
all: $(BINARY)
 
-include Makefile.deps
 
$(BINARY): Makefile.deps $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BINARY)
 
Makefile.deps: $(FILES) $(HEADERS)
	makedepend -- $(CFLAGS) -- $(FILES) -f- > Makefile.deps

download: $(BINARY)
	patserdow -v /dev/ttyUSB0 $(BINARY)

clean:
	-rm $(OBJS)
	-rm Makefile.deps
	-rm *.o
	-rm main.elf
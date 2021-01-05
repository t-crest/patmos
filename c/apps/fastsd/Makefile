CC		= patmos-clang
TARGET		= fastsd

SOURCES		= $(wildcard *.c)
OBJECTS		= $(SOURCES:.c=.o)

CFLAGS		+= -DPATMOS_IO_FASTSD=$(PATMOS_IO_FASTSD) -O2


$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: $(TARGET).elf

clean:
	$(RM) *.o *.elf


.PHONY: all clean

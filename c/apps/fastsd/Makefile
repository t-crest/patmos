CC		= patmos-clang
TARGET		= fastsd

SOURCES		= $(wildcard *.c)
OBJECTS		= $(SOURCES:.c=.o)

CFLAGS		+= -DDEBUG_SDC -O2


$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: $(TARGET).elf

clean:
	$(RM) *.o *.elf


.PHONY: all clean

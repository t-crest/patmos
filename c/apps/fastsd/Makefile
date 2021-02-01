CC			= patmos-clang
TARGET		= fastsd

SOURCES		= $(wildcard *.c)
OBJECTS		= $(SOURCES:.c=.o)

CFLAGS		+= -O2 -Wall -std=gnu11
#CFLAGS		+= -DDEBUG_SDC


$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: $(TARGET).elf

clean:
	$(RM) *.o *.elf


.PHONY: all clean

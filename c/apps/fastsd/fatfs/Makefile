# Makefile for elm-chan's FAT-FS file system library
# Philipp Birkl, Technical University of Vienna, 2020

LIBNAME := libfatfs
.PHONY: all clean

SOURCES := ff.c
OBJECTS := $(SOURCES:%.c=%.o)

AR		?= patmos-ar

all: $(LIBNAME).a

$(LIBNAME).a: $(OBJECTS)
	$(AR) -crs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) $(LIBNAME).a


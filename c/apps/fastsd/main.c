#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdcio.h"

void main() {
    printf("Hello world!");

    sdcio_write(0, 0xFFFFFFFF);
    uint32_t data = sdcio_read(0);

    printf("Wrote %x \t data %x", SDCIO_BASE+0, 0xFFFFFFFF);
    printf("Read %x \t data %x", SDCIO_BASE+0, data);
}
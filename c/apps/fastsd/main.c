#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdcio.h"

int main() {
    sdcio_write(0, 0xFFFFFFFF);
    uint32_t data = sdcio_read(0);

    printf("Wrote to  %08lx data %08lx\n", (uint32_t) SDCIO_BASE+0, (uint32_t) 0xAA55AA55);
    printf("Read from %08lx data %08lx\n", (uint32_t) SDCIO_BASE+0, (uint32_t) data);
}

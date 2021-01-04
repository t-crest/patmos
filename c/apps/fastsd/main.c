#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sdcio.h"

int main() {
    uint32_t wr_data = 0xFFFFFFFF;
    sdcio_write(0, wr_data);
    uint32_t rd_data = sdcio_read(0);

    printf("Wrote to  %08lx data %08lx\n", (uint32_t) SDCIO_BASE+0, (uint32_t) wr_data);
    printf("Read from %08lx data %08lx\n", (uint32_t) SDCIO_BASE+0, (uint32_t) rd_data);
}

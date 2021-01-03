#include <sdcio.h>

void sdcio_write(const uint32_t address, const uint32_t value) {
    *(SDCIO_BASE+address) = value;
}

uint32_t sdcio_read(const uint32_t address){
    return *(SDCIO_BASE+address);
}
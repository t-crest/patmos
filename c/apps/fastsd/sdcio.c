#include "sdcio.h"

#include <machine/patmos.h>

static volatile _IODEV uint32_t *const SDCIO_BASE = (volatile _IODEV uint32_t *)0xF00D0000;

void sdc_reg_write(const sdc_reg_t reg, const uint32_t value)
{
    *(SDCIO_BASE + reg) = value;
}

uint32_t sdc_reg_read(const sdc_reg_t reg)
{
    return *(SDCIO_BASE + reg);
}

void sdc_buffer_write(const sdc_reg_t address, const uint32_t value)
{
    *(SDCIO_BASE + address) = value;
}

uint32_t sdc_buffer_read(const sdc_reg_t address)
{
    return *(SDCIO_BASE + address);
}

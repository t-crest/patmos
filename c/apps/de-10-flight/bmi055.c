#include "bmi055.h"

uint8_t read_reg(unsigned sensor, unsigned reg)
{
    return (uint8_t)i2c_read(sensor, reg);
}

uint16_t read_reg16(unsigned sensor, unsigned reg_msb, unsigned reg_lsb)
{
    return (uint16_t)(i2c_read(sensor, reg_msb) << 8) | i2c_read(sensor, reg_lsb);
}

void modify_reg(unsigned sensor, unsigned reg, uint8_t clearbits, uint8_t setbits)
{
    uint8_t val = read_reg(sensor, reg);
    val &= ~clearbits;
    val |= setbits;
    write_reg(sensor, reg, val);
}

void write_reg(unsigned sensor, unsigned reg, uint8_t value)
{
    i2c_write(sensor, reg, value);
}
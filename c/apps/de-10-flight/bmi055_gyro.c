#include "bmi055_gyro.h"

int16_t read_gyro_X()
{
    //uint8_t lsb_reg = read_reg(GYRO, GYR_X_LSB);
    //uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = read_reg(GYRO, GYR_X_LSB);
    uint8_t msb = read_reg(GYRO, GYR_X_MSB);
    int16_t msblsb = (msb << 8) | lsb;
    return ((int16_t)msblsb);
}

int16_t read_gyro_Y()
{
    //uint8_t lsb_reg = read_reg(GYRO, GYR_Y_LSB);
    //uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = read_reg(GYRO, GYR_Y_LSB);
    uint8_t msb = read_reg(GYRO, GYR_Y_MSB);
    int16_t msblsb = (msb << 8) | lsb;
    return ((int16_t)msblsb);
}

int16_t read_gyro_Z()
{
    //uint8_t lsb_reg = read_reg(GYRO, GYR_Z_LSB);
    //uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = read_reg(GYRO, GYR_Z_LSB);
    uint8_t msb = read_reg(GYRO, GYR_Z_MSB);
    int16_t msblsb = ((uint16_t)msb << 8) | lsb;
    return ((int16_t)msblsb);
}

struct Report report_gyro()
{
    struct Report re;
    re.accel_x = read_gyro_X();
    re.accel_y = read_gyro_Y();
    re.accel_z = read_gyro_Z();
    re.temp = 0;
    re.timestamp = get_cpu_usecs();
    return re;
}

int set_gyro_range(unsigned max_dps)
{
    uint8_t setbits = 0;
    uint8_t clearbits = GYRO_RANGE_125_DPS | GYRO_RANGE_250_DPS;
    float lsb_per_dps;

    if (max_dps == 0)
    {
        max_dps = 2000;
    }
    if (max_dps <= 125)
    {
        //max_gyro_dps = 125;
        lsb_per_dps = 262.4;
        setbits |= GYRO_RANGE_125_DPS;
    }
    else if (max_dps <= 250)
    {
        //max_gyro_dps = 250;
        lsb_per_dps = 131.2;
        setbits |= GYRO_RANGE_250_DPS;
    }
    else if (max_dps <= 500)
    {
        //max_gyro_dps = 500;
        lsb_per_dps = 65.6;
        setbits |= GYRO_RANGE_500_DPS;
    }
    else if (max_dps <= 1000)
    {
        //max_gyro_dps = 1000;
        lsb_per_dps = 32.8;
        setbits |= GYRO_RANGE_1000_DPS;
    }
    else if (max_dps <= 2000)
    {
        //max_gyro_dps = 2000;
        lsb_per_dps = 16.4;
        setbits |= GYRO_RANGE_2000_DPS;
    }
    else
    {
        return -1;
    }

    GYRO_SCALE = (PI / (180.0f * lsb_per_dps));
    modify_reg(GYRO, GYR_RANGE, clearbits, setbits);

    return 1;
}
#include "bmi055_accel.h"

const float CONSTANTS_ONE_G = 9.80665f;

float read_temprature()
{
    unsigned char temp = read_reg(ACCEL, REG_TEMP);
    float calc_temp = (temp * 0.5f) + 23.0f;
    return calc_temp;
}

int16_t read_accel_X()
{
    uint8_t lsb_reg = read_reg(ACCEL, REG_ACC_X_LSB);
    uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = (lsb_reg & 0b11110000);
    uint8_t msb = read_reg(ACCEL, REG_ACC_X_MSB);
    int16_t msblsb = (msb << 8) | lsb;
    return ((int16_t)msblsb >> 4);
}

int16_t read_accel_Y()
{
    uint8_t lsb_reg = read_reg(ACCEL, REG_ACC_Y_LSB);
    uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = (lsb_reg & 0b11110000);
    uint8_t msb = read_reg(ACCEL, REG_ACC_Y_MSB);
    int16_t msblsb = (msb << 8) | lsb;
    return ((int16_t)msblsb >> 4);
}

int16_t read_accel_Z()
{
    uint8_t lsb_reg = read_reg(ACCEL, REG_ACC_Z_LSB);
    uint8_t new_val_bit = (lsb_reg & 0b00000001); //TBD
    uint8_t lsb = (lsb_reg & 0b11110000);
    uint8_t msb = read_reg(ACCEL, REG_ACC_Z_MSB);
    int16_t msblsb = (msb << 8) | lsb;
    return ((int16_t)msblsb >> 4);
}

struct Report report_accel()
{
    struct Report re;
    re.accel_x = read_accel_X();
    re.accel_y = read_accel_Y();
    re.accel_z = read_accel_Z();
    re.temp = read_temprature();
    re.timestamp = get_cpu_usecs();
    return re;
}

int set_accel_range(unsigned max_g)
{
    uint8_t setbits = 0;
    uint8_t clearbits = BMI055_ACCEL_RANGE_2_G | BMI055_ACCEL_RANGE_16_G;
    float lsb_per_g;

    if (max_g == 0)
    {
        max_g = 16;
    }
    if (max_g <= 2)
    {
        //Default case
        setbits |= BMI055_ACCEL_RANGE_2_G;
        lsb_per_g = 1024;
    }
    else if (max_g <= 4)
    {
        setbits |= BMI055_ACCEL_RANGE_4_G;
        lsb_per_g = 512;
    }
    else if (max_g <= 8)
    {
        setbits |= BMI055_ACCEL_RANGE_8_G;
        lsb_per_g = 256;
    }
    else if (max_g <= 16)
    {

        setbits |= BMI055_ACCEL_RANGE_16_G;
        lsb_per_g = 128;
    }
    else
    {
        return -1;
    }

    //SCALE = CONSTANTS_ONE_G / lsb_per_g;
    SCALE = CONSTANTS_ONE_G / lsb_per_g;
    modify_reg(ACCEL, REG_PMU_RANGE, clearbits, setbits);

    return 1;
}

void print_registers()
{
    printf("BMI055 accel registers\n");

    uint8_t v = read_reg(ACCEL, REG_CHIPID);
    printf("Accel Chip Id: %02x:%02x ", (unsigned)REG_CHIPID, (unsigned)v);
    printf("\n");

    v = read_reg(ACCEL, REG_PMU_BW);
    printf("Accel Bw: %02x:%02x ", (unsigned)REG_PMU_BW, (unsigned)v);
    printf("\n");

    v = read_reg(ACCEL, REG_PMU_RANGE);
    printf("Accel Range: %02x:%02x ", (unsigned)REG_PMU_RANGE, (unsigned)v);
    printf("\n");

    v = read_reg(ACCEL, REG_INT_EN_1);
    printf("Accel Int-en-1: %02x:%02x ", (unsigned)REG_INT_EN_1, (unsigned)v);
    printf("\n");

    v = read_reg(ACCEL, REG_INT_MAP_1);
    printf("Accel Int-Map-1: %02x:%02x ", (unsigned)REG_INT_MAP_1, (unsigned)v);
    printf("\n");
}
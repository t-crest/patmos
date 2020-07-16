#ifndef FLIGHT_BMI055_GYRO_H
#define FLIGHT_BMI055_GYRO_H
#include "bmi055.h"

#define GYRO 0x68 //BMI055 GYRO DEVICE

// BMI055 Gyro registers
#define GYR_CHIP_ID 0x00
#define GYR_X_LSB 0x02
#define GYR_X_MSB 0x03
#define GYR_Y_LSB 0x04
#define GYR_Y_MSB 0x05
#define GYR_Z_LSB 0x06
#define GYR_Z_MSB 0x07
#define GYR_INT_STATUS_0 0x09
#define GYR_INT_STATUS_1 0x0A
#define GYR_INT_STATUS_2 0x0B
#define GYR_INT_STATUS_3 0x0C
#define GYR_FIFO_STATUS 0x0E
#define GYR_RANGE 0x0F
#define GYR_BW 0x10
#define GYR_LPM1 0x11
#define GYR_LPM2 0x12
#define GYR_RATE_HBW 0x13
#define GYR_SOFTRESET 0x14
#define GYR_INT_EN_0 0x15
#define GYR_INT_EN_1 0x16
#define GYR_INT_MAP_0 0x17
#define GYR_INT_MAP_1 0x18
#define GYR_INT_MAP_2 0x19
#define GYRO_0_REG 0x1A
#define GYRO_1_REG 0x1B
#define GYRO_2_REG 0x1C
#define GYRO_3_REG 0x1E
#define GYR_INT_LATCH 0x21
#define GYR_INT_LH_0 0x22
#define GYR_INT_LH_1 0x23
#define GYR_INT_LH_2 0x24
#define GYR_INT_LH_3 0x25
#define GYR_INT_LH_4 0x26
#define GYR_INT_LH_5 0x27
#define GYR_SOC 0x31
#define GYR_A_FOC 0x32
#define GYR_TRIM_NVM_CTRL 0x33
#define BGW_SPI3_WDT 0x34
#define GYR_OFFSET_COMP 0x36
#define GYR_OFFSET_COMP_X 0x37
#define GYR_OFFSET_COMP_Y 0x38
#define GYR_OFFSET_COMP_Z 0x39
#define GYR_TRIM_GPO 0x3A
#define GYR_TRIM_GP1 0x3B
#define GYR_SELF_TEST 0x3C
#define GYR_FIFO_CONFIG_0 0x3D
#define GYR_FIFO_CONFIG_1 0x3E
#define GYR_FIFO_DATA 0x3F

// BMI055 Gyroscope Chip-Id
#define GYR_WHO_AM_I 0x0F

// ODR & DLPF filter bandwidth settings (they are coupled)
#define GYRO_RATE_100 (0 << 3) | (1 << 2) | (1 << 1) | (1 << 0)
#define GYRO_RATE_200 (0 << 3) | (1 << 2) | (1 << 1) | (0 << 0)
#define GYRO_RATE_400 (0 << 3) | (0 << 2) | (1 << 1) | (1 << 0)
#define GYRO_RATE_1000 (0 << 3) | (0 << 2) | (1 << 1) | (0 << 0)
#define GYRO_RATE_2000 (0 << 3) | (0 << 2) | (0 << 1) | (1 << 0)

//GYR_LPM1      0x11
#define GYRO_NORMAL (0 << 7) | (0 << 5)
#define GYRO_DEEP_SUSPEND (0 << 7) | (1 << 5)
#define GYRO_SUSPEND (1 << 7) | (0 << 5)

//GYR_RANGE        0x0F
#define GYRO_RANGE_2000_DPS (0 << 2) | (0 << 1) | (0 << 0)
#define GYRO_RANGE_1000_DPS (0 << 2) | (0 << 1) | (1 << 0)
#define GYRO_RANGE_500_DPS (0 << 2) | (1 << 1) | (0 << 0)
#define GYRO_RANGE_250_DPS (0 << 2) | (1 << 1) | (1 << 0)
#define GYRO_RANGE_125_DPS (1 << 2) | (0 << 1) | (0 << 0)

//GYR_INT_EN_0         0x15
#define GYR_DRDY_INT_EN (1 << 7)

//GYR_INT_MAP_1     0x18
#define GYR_DRDY_INT1 (1 << 0)

// Default and Max values
#define GYRO_DEFAULT_RANGE_DPS 2000
#define GYRO_DEFAULT_RATE 1000
#define GYRO_MAX_RATE 1000
#define GYRO_MAX_PUBLISH_RATE 280

#define GYRO_DEFAULT_DRIVER_FILTER_FREQ 50

/* Mask definitions for Gyro bandwidth */
#define GYRO_BW_MASK 0x0F

#define PI 3.14159265358979323846f

// struct Report
// {
//     int timestamp;
//     int16_t gyro_x;
//     int16_t gyro_y;
//     int16_t gyro_z;
// } report;

float GYRO_SCALE;
struct Report
report_gyro();

void print_registers();

int set_gyro_range(unsigned max_dps);

#endif
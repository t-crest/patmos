
#ifndef FLIGHT_BMI055_ACCEL_H
#define FLIGHT_BMI055_ACCEL_H
#include "bmi055.h"

#define ACCEL 0x18 //BMI055 ACCEL DEVICE

//BMI055_ACC_RANGE        0x0F
#define BMI055_ACCEL_RANGE_2_G (0 << 3) | (0 << 2) | (1 << 1) | (1 << 0)
#define BMI055_ACCEL_RANGE_4_G (0 << 3) | (1 << 2) | (0 << 1) | (1 << 0)
#define BMI055_ACCEL_RANGE_8_G (1 << 3) | (0 << 2) | (0 << 1) | (0 << 0)
#define BMI055_ACCEL_RANGE_16_G (1 << 3) | (1 << 2) | (0 << 1) | (0 << 0)

// Default and Max values
#define BMI055_ACCEL_DEFAULT_RANGE_G 16
#define BMI055_ACCEL_DEFAULT_RATE 1000
#define BMI055_ACCEL_MAX_RATE 1000
#define BMI055_ACCEL_MAX_PUBLISH_RATE 280

float SCALE;

struct Report report_accel();

void print_registers();

int set_accel_range(unsigned max_g);

#endif
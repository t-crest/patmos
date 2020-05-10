#pragma once

#include <stdlib.h>
#include "i2c.h"


/*Constants*/
#define MPU9250_TIMEOUT 500
#define MPU9250_GYRO_FS_250		0x0		//dps
#define MPU9250_GYRO_FS_500		0x8		//dps
#define MPU9250_GYRO_FS_1000	0x10	//dps
#define MPU9250_GYRO_FS_2000	0x18	//dps
#define MPU9250_ACCEL_FS_2		0x0		//g
#define MPU9250_ACCEL_FS_4		0x8		//g
#define MPU9250_ACCEL_FS_8		0x10	//g
#define MPU9250_ACCEL_FS_16		0x18	//g

#define MPU9250_CLOCK_PLL_ZGYRO	0x0

#define MPU9250_PIN_AD0_STATUS  0x0
#define MPU9250_CHIP_ADDR MPU9250_PIN_AD0_STATUS == 0 ? 0x68 : 0x69

/*Flags*/
#define MPU9250_READ_FLAG		0x80
#define MPU9250_WRITE_FLAG		0x00
#define MPU9250_WHO_AM_I_ID     0x71	

/*Regs*/
#define MPU9250_REG_SMPLRT_DIV		0x19
#define MPU9250_REG_CONFIG			0x1A
#define MPU9250_REG_GYRO_CONFIG		0x1B
#define MPU9250_REG_ACCEL_CONFIG	0x1C
#define MPU9250_REG_ACCEL_CONFIG_2	0x1D

#define MPU9250_REG_INT_PIN_CFG		0x37
#define MPU9250_REG_INT_ENABLE		0x38
#define MPU9250_REG_INT_STATUS		0x3A

#define MPU9250_REG_ACCEL_XOUT_H	0x3B
#define MPU9250_REG_ACCEL_XOUT_L	0x3C
#define MPU9250_REG_ACCEL_YOUT_H	0x3D
#define MPU9250_REG_ACCEL_YOUT_L	0x3E
#define MPU9250_REG_ACCEL_ZOUT_H	0x3F
#define MPU9250_REG_ACCEL_ZOUT_L	0x40

#define MPU9250_REG_TEMP_OUT_H		0x41
#define MPU9250_REG_TEMP_OUT_L		0x42

#define MPU9250_REG_GYRO_XOUT_H		0x43
#define MPU9250_REG_GYRO_XOUT_L		0x44
#define MPU9250_REG_GYRO_YOUT_H		0x45
#define MPU9250_REG_GYRO_YOUT_L		0x46
#define MPU9250_REG_GYRO_ZOUT_H		0x47
#define MPU9250_REG_GYRO_ZOUT_L		0x48

#define MPU9250_REG_SIGNAL_PATH_RESET 0x68
#define MPU9250_REG_MOT_DETECT_CTRL	0x69
#define MPU9250_REG_USER_CTRL		0x6A
#define MPU9250_REG_PWR_MGMT_1		0x6B
#define MPU9250_REG_PWR_MGMT_2		0x6C
#define MPU9250_REG_WHO_AM_I		0x75

/*Mask*/
#define MPU9250_GYRO_FS_SEL_MASK	0x18
#define MPU9250_ACCEL_FS_SEL_MASK	0x18
#define MPU9250_RESET_MASK			0x80
#define MPU9250_SLEEP_MASK			0x40
#define MPU9250_CYCLE_MASK			0x20
#define MPU9250_GYRO_STANDBY_MASK 	0x10
#define MPU9250_CLKSEL_MASK			0x7

/*Prototypes*/
int getMPU9250WhoAmI();
void writeRegByteMPU9250(unsigned char reg,  unsigned char value);
unsigned char readRegByteMPU9250(unsigned char reg);
int initMPU9250();
int16_t getMPU9250RawAccelX();
int16_t getMPU9250RawAccelY();
int16_t getMPU9250RawAccelZ();
double getMPU9250AccelX();
double getMPU9250AccelY();
double getMPU9250AccelZ();
int16_t getMPU9250RawGyroX();
int16_t getMPU9250RawGyroY();
int16_t getMPU9250RawGyroZ();
#include <stdio.h>
#include <machine/patmos.h>
#include <stdint.h>
#include <machine/spm.h>
#include "i2c_master.h"
//LEDs
#define LED ( *( ( volatile _IODEV unsigned * )	PATMOS_IO_LED ) )

#define MPU6050_I2C_ADDRESS 0x68

//MCU6050 registers
#define MPU6050_ACCEL_XOUT_H       0x3B   // R
#define MPU6050_ACCEL_XOUT_L       0x3C   // R
#define MPU6050_ACCEL_YOUT_H       0x3D   // R
#define MPU6050_ACCEL_YOUT_L       0x3E   // R
#define MPU6050_ACCEL_ZOUT_H       0x3F   // R
#define MPU6050_ACCEL_ZOUT_L       0x40   // R
#define MPU6050_TEMP_OUT_H         0x41   // R
#define MPU6050_TEMP_OUT_L         0x42   // R
#define MPU6050_GYRO_XOUT_H        0x43   // R
#define MPU6050_GYRO_XOUT_L        0x44   // R
#define MPU6050_GYRO_YOUT_H        0x45   // R
#define MPU6050_GYRO_YOUT_L        0x46   // R
#define MPU6050_GYRO_ZOUT_H        0x47   // R
#define MPU6050_GYRO_ZOUT_L        0x48   // R
#define MPU6050_PWR_MGMT_1         0x6B   // R/W
#define MPU6050_WHO_AM_I           0x75   // R

// //Blinks the LEDs once
void blink_once(){
  int i, j;
  for (i=2000; i!=0; --i)
    for (j=2000; j!=0; --j)
      LED = 0x0001;
  for (i=2000; i!=0; --i)
    for (j=2000; j!=0; --j)
      LED = 0x0000;
  return;
}

int main(int argc, char **argv)
{
  printf("Hello MCU6050!\n");
  blink_once();

  unsigned int signature = 0;


  unsigned int ACCEL_X_H = 0;
  unsigned int ACCEL_X_L = 0;
  unsigned int ACCEL_Y_H = 0;
  unsigned int ACCEL_Y_L = 0;
  unsigned int ACCEL_Z_H = 0;
  unsigned int ACCEL_Z_L = 0;
  unsigned int TEMP_L = 0;
  unsigned int TEMP_H = 0;
  unsigned int GYRO_X_H = 0;
  unsigned int GYRO_X_L = 0;
  unsigned int GYRO_Y_H = 0;
  unsigned int GYRO_Y_L = 0;
  unsigned int GYRO_Z_H = 0;
  unsigned int GYRO_Z_L = 0;


  signature = i2c_reg8_read8(MPU6050_I2C_ADDRESS, MPU6050_WHO_AM_I);
  printf("Signature = 0x%.2X\n", signature);
  printf("PWR_MGMT_1 = 0x%.2X\n", i2c_reg8_read8(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1));
  printf("Getting MPU-6050 out of sleep mode.\n");
  i2c_reg8_write8(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1, 0x00);
  printf("PWR_MGMT_1 = 0x%.2X\n", i2c_reg8_read8(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1));

  for (int i = 0; i < 5; i++) {
  // for (;;) {
    blink_once();

    ACCEL_X_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_XOUT_H);
    ACCEL_Y_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_YOUT_H);
    ACCEL_Z_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_ZOUT_H);
    TEMP_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_TEMP_OUT_H);
    GYRO_X_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_GYRO_XOUT_H);
    GYRO_Y_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_GYRO_YOUT_H);
    GYRO_Z_H = i2c_reg8_read16b(MPU6050_I2C_ADDRESS, MPU6050_GYRO_ZOUT_H);
    printf("-----------------------\n");
    printf("ACCEL_X = 0x%.4X (%d)\n", ACCEL_X_H, (short int)(ACCEL_X_H));
    printf("ACCEL_Y = 0x%.4X (%d)\n", ACCEL_Y_H, (short int)(ACCEL_Y_H));
    printf("ACCEL_Z = 0x%.4X (%d)\n", ACCEL_Z_H, (short int)(ACCEL_Z_H));
    printf("TEMP    = 0x%.4X (%.1f C)\n", TEMP_H,  ((double)((short int)(TEMP_H)) + 12412.0) / 340.0 ); //using datasheet formula for T in degrees Celsius
    printf("GYRO_X  = 0x%.4X (%d)\n", GYRO_X_H, (short int)(GYRO_X_H));
    printf("GYRO_Y  = 0x%.4X (%d)\n", GYRO_Y_H, (short int)(GYRO_Y_H));
    printf("GYRO_Z  = 0x%.4X (%d)\n", GYRO_Z_H,  (short int)(GYRO_Z_H));
  }


  return 0;
}

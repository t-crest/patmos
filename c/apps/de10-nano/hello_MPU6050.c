#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <machine/patmos.h>
//#include "libcorethread/corethread.h"
//#include "libmp/mp.h"

//LEDs
#define LED ( *( ( volatile _IODEV unsigned * )	PATMOS_IO_LED ) )

//I2C controller
#define I2C ( *( ( volatile _IODEV unsigned * )	PATMOS_IO_I2C ) )

// Default I2C address for the MPU-6050 is 0x68.
// But only if the AD0 pin is low.
// Some sensor boards have AD0 high, and the
// I2C address thus becomes 0x69.
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

//Writes to i2c, returns -1 if there was an error, 0 if succeded
int i2c_write(unsigned char chipaddress, unsigned char regaddress, unsigned char data){
  I2C = ((((unsigned int) data & 0x000000FF) << 16) | (((unsigned int) regaddress & 0x000000FF) << 8) | (((unsigned int) chipaddress & 0x0000007F) << 1)) & 0xFFFFFFFE;
  if ((I2C & 0x00000100) != 0)
  {
    return -1;
  }else{
    return 0;
  }
}

//Reads to i2c, returns the read value (8 bits), if there was an error the returned value is -1 (0xFFFFFFFF)
int i2c_read(unsigned char chipaddress, unsigned char regaddress){
  I2C = ((((unsigned int) regaddress & 0x000000FF) << 8) | (((unsigned int) chipaddress & 0x0000007F) << 1)) | 0x00000001;
  unsigned int I2C_tmp = I2C;
  if ((I2C_tmp & 0x00000100) != 0)
  {
    return -1;
  }else{
    return (int)((unsigned int)(I2C_tmp) & 0x000000FF);
  }
}

//Blinks the LEDs once
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

  signature = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_WHO_AM_I);
  printf("Signature = 0x%.2X\n", signature);

  printf("PWR_MGMT_1 = 0x%.2X\n", i2c_read(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1));
  printf("Getting MPU-6050 out of sleep mode.\n");
  i2c_write(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1, 0x00);
  printf("PWR_MGMT_1 = 0x%.2X\n", i2c_read(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1));

  //for (int i = 0; i < 5; i++) {
  for (;;) {
    blink_once();
    ACCEL_X_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_XOUT_H);
    ACCEL_Y_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_YOUT_H);
    ACCEL_X_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_XOUT_L);
    ACCEL_Y_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_YOUT_L);
    ACCEL_Z_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_ZOUT_H);
    ACCEL_Z_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_ACCEL_ZOUT_L);
    TEMP_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_TEMP_OUT_L);
    TEMP_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_TEMP_OUT_H);
    GYRO_X_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_XOUT_H);
    GYRO_X_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_XOUT_L);
    GYRO_Y_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_YOUT_H);
    GYRO_Y_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_YOUT_L);
    GYRO_Z_H = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_ZOUT_H);
    GYRO_Z_L = i2c_read(MPU6050_I2C_ADDRESS, MPU6050_GYRO_ZOUT_L);
    printf("-----------------------\n");
    printf("ACCEL_X = 0x%.2X%.2X (%d)\n", ACCEL_X_H, ACCEL_X_L, (short int)((ACCEL_X_H << 8) | ACCEL_X_L));
    printf("ACCEL_Y = 0x%.2X%.2X (%d)\n", ACCEL_Y_H, ACCEL_Y_L, (short int)((ACCEL_Y_H << 8) | ACCEL_Y_L));
    printf("ACCEL_Z = 0x%.2X%.2X (%d)\n", ACCEL_Z_H, ACCEL_Z_L, (short int)((ACCEL_Z_H << 8) | ACCEL_Z_L));
    printf("TEMP    = 0x%.2X%.2X (%.1f C)\n", TEMP_H, TEMP_L, ((double)((short int)((TEMP_H << 8) | TEMP_L)) + 12412.0) / 340.0 ); //using datasheet formula for T in degrees Celsius
    printf("GYRO_X  = 0x%.2X%.2X (%d)\n", GYRO_X_H, GYRO_X_L, (short int)((GYRO_X_H << 8) | GYRO_X_L));
    printf("GYRO_Y  = 0x%.2X%.2X (%d)\n", GYRO_Y_H, GYRO_Y_L, (short int)((GYRO_Y_H << 8) | GYRO_Y_L));
    printf("GYRO_Z  = 0x%.2X%.2X (%d)\n", GYRO_Z_H, GYRO_Z_L, (short int)((GYRO_Z_H << 8) | GYRO_Z_L));
  }


  return 0;
}

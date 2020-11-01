#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include <machine/patmos.h>
//#include "libcorethread/corethread.h"
//#include "libmp/mp.h"
#include <stdint.h>
#include <machine/spm.h>

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

#define IO_PTR_i2c_OFFSET 0xf00e0000
#define OCP_AVAIL_OFFSET    0b000100
#define OCP_READFLAG_OFFSET 0b010000
#define OCP_READDATA_OFFSET 0b001000

volatile _IODEV int *io_ptr = (volatile _IODEV int*) IO_PTR_i2c_OFFSET;
volatile _IODEV int *ocp_avail_ptr = (volatile _IODEV int*) (IO_PTR_i2c_OFFSET | OCP_AVAIL_OFFSET);
volatile _IODEV int *ocp_readflag_ptr = (volatile _IODEV int*) (IO_PTR_i2c_OFFSET | OCP_READFLAG_OFFSET);
volatile _IODEV int *ocp_readdata_ptr = (volatile _IODEV int*) (IO_PTR_i2c_OFFSET | OCP_READDATA_OFFSET);

/**
  Retrieves the value of the ocpAvailable flag
  @return True if OCP is available and can receive a new command, false otherwise
*/
uint8_t ocp_available() {
  return *ocp_avail_ptr;
}

/**
  Writes a byte to an I2C slave device
  @param slave_addr the I2C-address of the slave device (7 bits)
  @param data The byte to be sent to the slave device
*/
void write_byte(uint8_t slave_addr, uint8_t data) {
  int cmd = (slave_addr << 9) | (0 << 8) | data; //0 is the write bit
  *io_ptr = cmd;
}

/**
  Writes several bytes to the same i2c slave device
  @param slave_addr the I2C-address of the slave-device (7 bits)
  @param data Byte array of the values to be sent to the slave
  @param num_bytes the number of bytes in the array
*/
void write_bytes(uint8_t slave_addr, uint8_t* data, uint8_t num_bytes) {
  uint8_t i;
  for(i=0; i<num_bytes; i++) {
    while(!ocp_available()) { //Wait until available

    }
    write_byte(slave_addr, data[i]);
  }
}


/**
  Send a read prompt to the slave, requesting data that can be retrieved later
  @warning If a read prompt is issued, the data MUST be retrieved with get_read_data before another prompt can be made.
  If this is not done, the resulting data from read number 2 may be gibberish
  @param slave_addr The address of the slave device to be read
  @param num_bytes The number of bytes to be read. Must be in the range [1;4]
  @return 0 if successfully prompted, -1 if unsuccesful
*/
uint8_t read_prompt(uint8_t slave_addr, uint8_t num_bytes){
  if(num_bytes < 1 || num_bytes > 4 || !ocp_available()) {
    return -1;
  }
  int cmd = (num_bytes << 16) | (slave_addr << 9) | (1 << 8);
  *io_ptr = cmd;
  return 0;
}

/**
  Checks if read data is available
  @return 1 if read data can be retrieved, 0 otherwise
*/
uint8_t get_read_flag(){
  return *ocp_readflag_ptr;
}

/**
  Retrieves the data read after issuing a command with read_prompt. After reading, a new prompt may be issued
  @param data, a pointer to where data will be stored if read is succesful 
  @return The corresponding value of get_read_flag for this operation
  
*/
uint8_t get_read_data(int *data){
  if(get_read_flag()){
    *data = *ocp_readdata_ptr;
    return 1;
  } else {
    return 0;
  }
}



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

//////////////////////new
  // uint8_t bytes[]= {0x0e, 0x0e, 0x0e, 0x0e};
  // uint8_t slave_addr =  0b1010101;
  // write_bytes(slave_addr, bytes, 4);

  // write_byte(slave_addr, wr_data);

  // //Issue read prompt, starting data transfer from slave
  // read_prompt(slave_addr, 1);

  // //Get read data from slave once finished
  // while(!get_read_data(&rd_data)){
  //   //Wait until data is available
  // }
  // printf("Got: %#010x\n", rd_data);



  //////////////////////new i2c
  // uint8_t bytes[]= {0x0e, 0x0e, 0x0e, 0x0e};
  // uint8_t slave_addr =  0b1010101;
  // write_bytes(slave_addr, bytes, 4);

  // write_byte(slave_addr, wr_data);

  // //Issue read prompt, starting data transfer from slave
  // read_prompt(slave_addr, 1);

  // //Get read data from slave once finished
  // while(!get_read_data(&rd_data)){
  //   //Wait until data is available
  // }
  // printf("Got: %#010x\n", rd_data);
///////////////////
  // int rd_data;
  // uint8_t wr_data = 0x01;
  // write_byte(MPU6050_I2C_ADDRESS, MPU6050_WHO_AM_I);
  // read_prompt(MPU6050_I2C_ADDRESS, 1);
  // while(!get_read_data(&rd_data)){
  //   //Wait until data is available
  // }
  // signature = rd_data;
  // printf("Signature = 0x%.2X\n", signature);

  // write_byte(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1);
  // read_prompt(MPU6050_I2C_ADDRESS, 1);
  // while(!get_read_data(&rd_data)){
  //   //Wait until data is available
  // }
  // signature = rd_data;

  // printf("PWR_MGMT_1 = 0x%.2X\n", rd_data);
  // printf("Getting MPU-6050 out of sleep mode.\n");
  // write_byte(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1);
  // write_byte(MPU6050_PWR_MGMT_1, 0x00);

  // write_byte(MPU6050_I2C_ADDRESS, MPU6050_PWR_MGMT_1);
  // read_prompt(MPU6050_I2C_ADDRESS, 1);
  // while(!get_read_data(&rd_data)){
  //   //Wait until data is available
  // }
  // signature = rd_data;
  // printf("PWR_MGMT_1 = 0x%.2X\n", rd_data);
///////////////////

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

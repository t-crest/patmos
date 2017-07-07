#include "audio_singlecore.h"

// The following two function are used to write and read through I2C

//Writes to i2c, returns 1 if there was an error, 0 if succeded
int i2c_write(unsigned char chipaddress, unsigned short int subaddress, unsigned char data){
  I2C = ((((unsigned int) data & 0x000000FF) << 24) | (((unsigned int) subaddress & 0x0000FFFF) << 8) | (((unsigned int) chipaddress & 0x0000007F) << 1)) & 0xFFFFFFFE;
  //printf("%08X\n", ((((unsigned int) data & 0x000000FF) << 24) | (((unsigned int) subaddress & 0x0000FFFF) << 8) | (((unsigned int) chipaddress & 0x0000007F) << 1)) & 0xFFFFFFFE);
  if ((I2C & 0x00000100) != 0)
  {
    return -1;
  }else{
    return 0;
  }
}

//Reads to i2c, returns the read value, if the integer is negative there was an error
int i2c_read(unsigned char chipaddress, unsigned short int subaddress){
  I2C = ((((unsigned int) subaddress & 0x0000FFFF) << 8) | (((unsigned int) chipaddress & 0x0000007F) << 1)) | 0x00000001;
  unsigned int I2C_tmp = I2C;
  if ((I2C_tmp & 0x00000100) != 0)
  {
    return (int)(((unsigned int)(I2C_tmp) & 0x000000FF) | 0x80000000);
  }else{
    return (int)((unsigned int)(I2C_tmp) & 0x000000FF);
  }
}


int isPowerOfTwo (unsigned int x) {
 while (((x % 2) == 0) && x > 1) /* While x is even and > 1 */
   x /= 2;
 return (x == 1);
}

/*
 * @brief	reads data from the input (ADC) buffer into Patmos
 * @param[in]	*l	pointer to left audio data
 * @param[in]	*r	pointer to right audio data
 * @return	returns 0 if successful
 */
int getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r) {
  while(*audioAdcBufferEmptyReg == 1){;}// wait until not empty

  int sample = 0;
  short sample_left = 0;
  short sample_right = 0;

  sample = *audioAdcReg;
  sample_left = (short)((sample >> 16) & 0x0000FFFF);
  sample_right = (short)(sample & 0x0000FFFF);  

  *l = sample_left;
  *r = sample_right;

  return 0;
}

/*
 * @brief	writes data from patmos into the output (DAC) buffer
 * @param[in]	l	left audio data
 * @param[in]	r	right audio data
 * @return	returns 0 if successful
 */
int setOutputBufferSPM(volatile _SPM short *l, volatile _SPM short *r) {
  while(*audioDacBufferFullReg == 1){;} // wait until not full

  int sample = 0;
  int sample_left = 0;
  int sample_right = 0;

  sample_left = (int)*l;
  sample_right = (int)*r;
  sample = ((sample_left << 16) & 0xFFFF0000) | (sample_right & 0x0000FFFF);

  *audioDacReg = sample;

  return 0;
}

int ADAU1761_init(int configuration){
  int ans = 0;

  if (configuration == 0)
  {
    //configuration 0 
    // input = line in
    // output = line out (speakers)
    ans += i2c_write(CHIPADDR, 0x4000, 0x01);
    //0x4002, 0x00 0xFD 0x00 0x0C 0x10 0x00
    ans += i2c_write(CHIPADDR, 0x4008, 0x00);
    ans += i2c_write(CHIPADDR, 0x4009, 0x00);
    ans += i2c_write(CHIPADDR, 0x400A, 0x01);
    ans += i2c_write(CHIPADDR, 0x400B, 0x05);
    ans += i2c_write(CHIPADDR, 0x400C, 0x01);
    ans += i2c_write(CHIPADDR, 0x400D, 0x05);
    ans += i2c_write(CHIPADDR, 0x400E, 0x00);
    ans += i2c_write(CHIPADDR, 0x400F, 0x00);
    ans += i2c_write(CHIPADDR, 0x4010, 0x00);
    ans += i2c_write(CHIPADDR, 0x4011, 0x00);
    ans += i2c_write(CHIPADDR, 0x4012, 0x00);
    ans += i2c_write(CHIPADDR, 0x4013, 0x00);
    ans += i2c_write(CHIPADDR, 0x4014, 0x00);
    //ans += i2c_write(CHIPADDR, 0x4015, 0x00);//slavemode
    ans += i2c_write(CHIPADDR, 0x4015, 0x01);//mastermode
    ans += i2c_write(CHIPADDR, 0x4016, 0x00);
    ans += i2c_write(CHIPADDR, 0x4017, 0x00);
    ans += i2c_write(CHIPADDR, 0x4018, 0x00);
    ans += i2c_write(CHIPADDR, 0x4019, 0x03);
    ans += i2c_write(CHIPADDR, 0x401A, 0x00);
    ans += i2c_write(CHIPADDR, 0x401B, 0x00);
    ans += i2c_write(CHIPADDR, 0x401C, 0x21);
    ans += i2c_write(CHIPADDR, 0x401D, 0x00);
    ans += i2c_write(CHIPADDR, 0x401E, 0x41);
    ans += i2c_write(CHIPADDR, 0x401F, 0x00);
    ans += i2c_write(CHIPADDR, 0x4020, 0x03);
    ans += i2c_write(CHIPADDR, 0x4021, 0x09);
    ans += i2c_write(CHIPADDR, 0x4022, 0x00);
    ans += i2c_write(CHIPADDR, 0x4023, 0x00);
    ans += i2c_write(CHIPADDR, 0x4024, 0x00);
    ans += i2c_write(CHIPADDR, 0x4025, 0xE6);
    ans += i2c_write(CHIPADDR, 0x4026, 0xE6);
    ans += i2c_write(CHIPADDR, 0x4027, 0x00);
    ans += i2c_write(CHIPADDR, 0x4028, 0x00);
    ans += i2c_write(CHIPADDR, 0x4029, 0x03);
    ans += i2c_write(CHIPADDR, 0x402A, 0x03);
    ans += i2c_write(CHIPADDR, 0x402B, 0x00);
    ans += i2c_write(CHIPADDR, 0x402C, 0x00);
    ans += i2c_write(CHIPADDR, 0x402D, 0xAA);
    ans += i2c_write(CHIPADDR, 0x402F, 0xAA);
    ans += i2c_write(CHIPADDR, 0x4030, 0x00);
    ans += i2c_write(CHIPADDR, 0x4031, 0x08);
    ans += i2c_write(CHIPADDR, 0x4036, 0x03);
    ans += i2c_write(CHIPADDR, 0x40C6, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C7, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C8, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C9, 0x00);
    ans += i2c_write(CHIPADDR, 0x40D0, 0x00);
    ans += i2c_write(CHIPADDR, 0x40EB, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F2, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F3, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F4, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F5, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F6, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F7, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F8, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F9, 0x7F);
    ans += i2c_write(CHIPADDR, 0x40FA, 0x03);
  } else if (configuration == 1)  {
    //configuration 1 
    // input = line in
    // output = line out (speakers) + hppout (headphone)
    ans += i2c_write(CHIPADDR, 0x4000, 0x01);
    //0x4002, 0x00 0xFD 0x00 0x0C 0x10 0x00
    ans += i2c_write(CHIPADDR, 0x4008, 0x00);
    ans += i2c_write(CHIPADDR, 0x4009, 0x00);
    ans += i2c_write(CHIPADDR, 0x400A, 0x01);
    ans += i2c_write(CHIPADDR, 0x400B, 0x05);
    ans += i2c_write(CHIPADDR, 0x400C, 0x01);
    ans += i2c_write(CHIPADDR, 0x400D, 0x05);
    ans += i2c_write(CHIPADDR, 0x400E, 0x00);
    ans += i2c_write(CHIPADDR, 0x400F, 0x00);
    ans += i2c_write(CHIPADDR, 0x4010, 0x00);
    ans += i2c_write(CHIPADDR, 0x4011, 0x00);
    ans += i2c_write(CHIPADDR, 0x4012, 0x00);
    ans += i2c_write(CHIPADDR, 0x4013, 0x00);
    ans += i2c_write(CHIPADDR, 0x4014, 0x00);
    //ans += i2c_write(CHIPADDR, 0x4015, 0x00);//slavemode
    ans += i2c_write(CHIPADDR, 0x4015, 0x01);//mastermode
    ans += i2c_write(CHIPADDR, 0x4016, 0x00);
    ans += i2c_write(CHIPADDR, 0x4017, 0x00);
    ans += i2c_write(CHIPADDR, 0x4018, 0x00);
    ans += i2c_write(CHIPADDR, 0x4019, 0x03);
    ans += i2c_write(CHIPADDR, 0x401A, 0x00);
    ans += i2c_write(CHIPADDR, 0x401B, 0x00);
    ans += i2c_write(CHIPADDR, 0x401C, 0x21);
    ans += i2c_write(CHIPADDR, 0x401D, 0x00);
    ans += i2c_write(CHIPADDR, 0x401E, 0x41);
    ans += i2c_write(CHIPADDR, 0x401F, 0x00);
    ans += i2c_write(CHIPADDR, 0x4020, 0x03);
    ans += i2c_write(CHIPADDR, 0x4021, 0x09);
    ans += i2c_write(CHIPADDR, 0x4022, 0x00);
    ans += i2c_write(CHIPADDR, 0x4023, 0xE7);//hp
    ans += i2c_write(CHIPADDR, 0x4024, 0xE7);//hp
    ans += i2c_write(CHIPADDR, 0x4025, 0xE6);
    ans += i2c_write(CHIPADDR, 0x4026, 0xE6);
    ans += i2c_write(CHIPADDR, 0x4027, 0x00);
    ans += i2c_write(CHIPADDR, 0x4028, 0x00);
    ans += i2c_write(CHIPADDR, 0x4029, 0x03);
    ans += i2c_write(CHIPADDR, 0x402A, 0x03);
    ans += i2c_write(CHIPADDR, 0x402B, 0x00);
    ans += i2c_write(CHIPADDR, 0x402C, 0x00);
    ans += i2c_write(CHIPADDR, 0x402D, 0xAA);
    ans += i2c_write(CHIPADDR, 0x402F, 0xAA);
    ans += i2c_write(CHIPADDR, 0x4030, 0x00);
    ans += i2c_write(CHIPADDR, 0x4031, 0x08);
    ans += i2c_write(CHIPADDR, 0x4036, 0x03);
    ans += i2c_write(CHIPADDR, 0x40C6, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C7, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C8, 0x00);
    ans += i2c_write(CHIPADDR, 0x40C9, 0x00);
    ans += i2c_write(CHIPADDR, 0x40D0, 0x00);
    ans += i2c_write(CHIPADDR, 0x40EB, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F2, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F3, 0x01);
    ans += i2c_write(CHIPADDR, 0x40F4, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F5, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F6, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F7, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F8, 0x00);
    ans += i2c_write(CHIPADDR, 0x40F9, 0x7F);
    ans += i2c_write(CHIPADDR, 0x40FA, 0x03);
  }else{
    ans = 1;
  }

  if (ans != 0)
  {
    return -1;
  }else{
    return 0;
  }
}
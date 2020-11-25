#include <stdio.h>
#include <stdlib.h>
#include <machine/rtc.h>
#include <machine/patmos.h>
#include <stdbool.h>
#include <math.h>
#include "../i2c-master/i2c_master.h"
void micros(int microseconds)
{
  unsigned int timer_ms = (get_cpu_usecs());
  unsigned int loop_timer = timer_ms;
  while(timer_ms - loop_timer < microseconds)timer_ms = get_cpu_usecs();
}
void millis(int milliseconds)
{
  unsigned int timer_ms = (get_cpu_usecs()/1000);
  unsigned int loop_timer = timer_ms;
  while(timer_ms - loop_timer < milliseconds)timer_ms = (get_cpu_usecs()/1000);
}
//LEDs
#define LED ( *( ( volatile _IODEV unsigned * ) PATMOS_IO_LED ) )


#define BARO_REG                   0xA0   // R

const unsigned int CPU_PERIOD = 20; //CPU period in ns.

//Barometer v2 variables
#define MS5611_ADDR 0x77

unsigned long Coff[6], Ti = 0, offi = 0, sensi = 0;
unsigned int data[3];
//////////////

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

void LED_out(int i){
  if(i==1) LED = 0x0001;
  else LED = 0x0000;
  return;
}

void check_barometerv2(void)
{
  i2c_reg8_write8(MS5611_ADDR, 0x1E, (int)NULL);
  millis(5);
  i2c_reg8_write8(MS5611_ADDR, 0x48, (int)NULL);
  millis(10);
  
  unsigned long ptemp = i2c_reg8_read24b(MS5611_ADDR, 0x00);
  

  i2c_reg8_write8(MS5611_ADDR, 0x58, (int)NULL);
  millis(10);
  

  // Convert the data
  unsigned long temp = i2c_reg8_read24b(MS5611_ADDR, 0x00);
  
  unsigned long dT = temp - ((Coff[4] * 256));
  temp = 2000 + (dT * (Coff[5] / pow(2, 23)));
  
  unsigned long long off = Coff[1] * 65536 + (Coff[3] * dT) / 128;
  unsigned long long sens = Coff[0] * 32768 + (Coff[2] * dT) / 256;

  if(temp >= 2000)
  {
     Ti = 0;
     offi = 0;
     sensi = 0;
  }
  else if(temp < 2000)
  {
    Ti = (dT * dT) / (pow(2,31));
    offi = 5 * ((pow((temp - 2000), 2))) / 2;
    sensi =  5 * ((pow((temp - 2000), 2))) / 4; 
    if(temp < -1500)
    {
       offi = offi + 7 * ((pow((temp + 1500), 2)));      
       sensi = sensi + 11 * ((pow((temp + 1500), 2))) / 2;
    }
  }
   
  temp -= Ti;
  off -= offi;
  sens -= sensi;

  ptemp = (ptemp * sens / 2097152 - off);
  ptemp /= 32768;
  float pressure = ptemp / 100.0;
  float ctemp = temp / 100.0;
  float fTemp = ctemp * 1.8 + 32.0;

  // Output data to serial monitor
  printf("Temperature in Celsius : %f            ",ctemp);
  printf(" Pressure : %f ",pressure);
  printf(" mbar \n"); 
  millis(10);

}

int main(int argc, char **argv)
{
  printf("Hello Baro!\n");
  
  for (int i = 0; i < 6; i++) {                   //Start communication with the MPU-6050.
      Coff[i] = i2c_reg8_read16b(MS5611_ADDR, 0xA2 + i*2);

    }
    printf("C1 = %lu\n",Coff[0]);
    printf("C2 = %lu\n",Coff[1]);
    printf("C3 = %lu\n",Coff[2]);
    printf("C4 = %lu\n",Coff[3]);
    printf("C5 = %lu\n",Coff[4]);
    printf("C6 = %lu\n",Coff[5]);
  
  for(int i=0;i<100;i++)
  check_barometerv2();
  return 0;
}
  

#ifndef _AUDIO_SINGLECORE_H_
#define _AUDIO_SINGLECORE_H_

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//I2C
#define I2C ( *( ( volatile _IODEV unsigned * )	0xF00C0000 ) )
#define CHIPADDR 0x38
#define SUBADDR_BASE 0x4000//0x0210

//#define LED_ADDR       (volatile _SPM int *) 0xF0090000
//#define KEY_ADDR       (volatile _SPM int *) 0xF00A0000

#define DAC_ADDR      (volatile _SPM int *) 0xF00B000C
#define DACBUFFU_ADDR  (volatile _SPM int *) 0xF00B0008

#define ADC_ADDR      (volatile _SPM int *) 0xF00B0004
#define ADCBUFEM_ADDR  (volatile _SPM int *) 0xF00B0000

//Leds
//volatile _SPM int *ledReg = LED_ADDR;
//Keys
//volatile _SPM int *keyReg = KEY_ADDR;

//DAC buffer
volatile _SPM int *audioDacReg	   = DAC_ADDR;
volatile _SPM int *audioDacBufferFullReg        = DACBUFFU_ADDR;

//ADC buffer
volatile _SPM int *audioAdcReg	   = ADC_ADDR;
volatile _SPM int *audioAdcBufferEmptyReg     = ADCBUFEM_ADDR;

int i2c_write(unsigned char chipaddress, unsigned short int subaddress, unsigned char data);
int i2c_read(unsigned char chipaddress, unsigned short int subaddress);
int ADAU1761_init(int configuration);
int getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int setOutputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);

#endif /* _AUDIO_SINGLECORE_H_ */

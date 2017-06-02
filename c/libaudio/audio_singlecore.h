#ifndef _AUDIO_SINGLECORE_H_
#define _AUDIO_SINGLECORE_H_

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LED_ADDR       (volatile _SPM int *) 0xF0090000
#define KEY_ADDR       (volatile _SPM int *) 0xF00A0000

#define AUDIODEV_BASE   0xF00C0000

#define DACL_ADDR      (volatile _SPM int *)(AUDIODEV_BASE+0x0000)
#define DACR_ADDR      (volatile _SPM int *)(AUDIODEV_BASE+0x0010)
#define DACEN_ADDR     (volatile _SPM int *)(AUDIODEV_BASE+0x0020)
#define DACBUFS_ADDR   (volatile _SPM int *)(AUDIODEV_BASE+0x0040)
#define DACBUFWP_ADDR  (volatile _SPM int *)(AUDIODEV_BASE+0x0050)
#define DACBUFFU_ADDR  (volatile _SPM int *)(AUDIODEV_BASE+0x0060)
#define ADCL_ADDR      (volatile _SPM int *)(AUDIODEV_BASE+0x0080)
#define ADCR_ADDR      (volatile _SPM int *)(AUDIODEV_BASE+0x0090)
#define ADCEN_ADDR     (volatile _SPM int *)(AUDIODEV_BASE+0x00A0)
#define ADCBUFS_ADDR   (volatile _SPM int *)(AUDIODEV_BASE+0x00B0)
#define ADCBUFRP_ADDR  (volatile _SPM int *)(AUDIODEV_BASE+0x00C0)
#define ADCBUFEM_ADDR  (volatile _SPM int *)(AUDIODEV_BASE+0x00D0)
#define I2CDATA_ADDR   (volatile _SPM int *)(AUDIODEV_BASE+0x00E0)
#define I2CADDR_ADDR   (volatile _SPM int *)(AUDIODEV_BASE+0x00F0)
#define I2CACK_ADDR    (volatile _SPM int *)(AUDIODEV_BASE+0x0100)
#define I2CREQ_ADDR    (volatile _SPM int *)(AUDIODEV_BASE+0x0110)

//Leds
volatile _SPM int *ledReg = LED_ADDR;
//Keys
volatile _SPM int *keyReg = KEY_ADDR;
//DAC
volatile _SPM int *audioDacLReg	   = DACL_ADDR;
volatile _SPM int *audioDacRReg    = DACR_ADDR;
volatile _SPM int *audioDacEnReg   = DACEN_ADDR;
//DAC buffer
volatile _SPM int *audioDacBufferSizeReg        = DACBUFS_ADDR;
volatile _SPM int *audioDacBufferWritePulseReg  = DACBUFWP_ADDR;
volatile _SPM int *audioDacBufferFullReg        = DACBUFFU_ADDR;
//ADC
volatile _SPM int *audioAdcLReg	   = ADCL_ADDR;
volatile _SPM int *audioAdcRReg	   = ADCR_ADDR;
volatile _SPM int *audioAdcEnReg   = ADCEN_ADDR;
//ADC buffer
volatile _SPM int *audioAdcBufferSizeReg      = ADCBUFS_ADDR;
volatile _SPM int *audioAdcBufferReadPulseReg = ADCBUFRP_ADDR;
volatile _SPM int *audioAdcBufferEmptyReg     = ADCBUFEM_ADDR;
//I2C
volatile _SPM int *i2cDataReg = I2CDATA_ADDR;
volatile _SPM int *i2cAdrReg  = I2CADDR_ADDR;
volatile _SPM int *i2cAckReg  = I2CACK_ADDR;
volatile _SPM int *i2cReqReg  = I2CREQ_ADDR;

void 	setup(int guitar);

int setInputBufferSize(int bufferSize);
int setOutputBufferSize(int bufferSize);

int getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int setOutputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);

#endif /* _AUDIO_SINGLECORE_H_ */

#ifndef AUDIO_H
#define AUDIO_H

//Leds
volatile _SPM int *ledReg = (volatile _SPM int *) 0xF0090000;

//Keys
volatile _SPM int *keyReg = (volatile _SPM int *) 0xF00A0000;

//DAC
volatile _SPM int *audioDacLReg	   = (volatile _SPM int *) 0xF00B0000;
volatile _SPM int *audioDacRReg    = (volatile _SPM int *) 0xF00B0010;
volatile _SPM int *audioDacEnReg   = (volatile _SPM int *) 0xF00B0020;
volatile _SPM int *audioDacBusyReg = (volatile _SPM int *) 0xF00B0030;
volatile _SPM int *audioDacReqReg  = (volatile _SPM int *) 0xF00B0040;
volatile _SPM int *audioDacLrcReg  = (volatile _SPM int *) 0xF00B0050;
volatile _SPM int *audioDacBufferReqReg = (volatile _SPM int *) 0xF00B0060;
volatile _SPM int *audioDacBufferAckReg = (volatile _SPM int *) 0xF00B0070;

//ADC
volatile _SPM int *audioAdcLReg	   = (volatile _SPM int *) 0xF00B0080;
volatile _SPM int *audioAdcRReg	   = (volatile _SPM int *) 0xF00B0090;
volatile _SPM int *audioAdcEnReg   = (volatile _SPM int *) 0xF00B00A0;
volatile _SPM int *audioAdcBusyReg = (volatile _SPM int *) 0xF00B00B0;
volatile _SPM int *audioAdcReqReg  = (volatile _SPM int *) 0xF00B00C0;
volatile _SPM int *audioAdcLrcReg  = (volatile _SPM int *) 0xF00B00D0;

//I2C
volatile _SPM int *i2cDataReg = (volatile _SPM int *) 0xF00B00E0;
volatile _SPM int *i2cAdrReg  = (volatile _SPM int *) 0xF00B00F0;
volatile _SPM int *i2cAckReg  = (volatile _SPM int *) 0xF00B0100;
volatile _SPM int *i2cReqReg  = (volatile _SPM int *) 0xF00B0110;


int 	writeToI2C(char* addrC,char* dataC);
void 	setup();
int 	setOutputAudio(short l, short r);
int 	getInputAudio(short *l, short *r);
int 	changeVolume(int vol);
void 	waitSyncDac();
void 	waitSyncAdc();

int     setOutputBuffer(short l, short r);

#endif

//#ifndef FILTER_ORDER_1PLUS
//#define FILTER_ORDER_1PLUS 2
//#endif

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

//DAC buffer
volatile _SPM int *audioDacBufferSizeReg        = (volatile _SPM int *) 0xF00B0040;
volatile _SPM int *audioDacBufferWritePulseReg  = (volatile _SPM int *) 0xF00B0050;
volatile _SPM int *audioDacBufferFullReg        = (volatile _SPM int *) 0xF00B0060;

//ADC
volatile _SPM int *audioAdcLReg	   = (volatile _SPM int *) 0xF00B0080;
volatile _SPM int *audioAdcRReg	   = (volatile _SPM int *) 0xF00B0090;
volatile _SPM int *audioAdcEnReg   = (volatile _SPM int *) 0xF00B00A0;

//ADC buffer
volatile _SPM int *audioAdcBufferSizeReg      = (volatile _SPM int *) 0xF00B00B0;
volatile _SPM int *audioAdcBufferReadPulseReg = (volatile _SPM int *) 0xF00B00C0;
volatile _SPM int *audioAdcBufferEmptyReg     = (volatile _SPM int *) 0xF00B00D0;

//I2C
volatile _SPM int *i2cDataReg = (volatile _SPM int *) 0xF00B00E0;
volatile _SPM int *i2cAdrReg  = (volatile _SPM int *) 0xF00B00F0;
volatile _SPM int *i2cAckReg  = (volatile _SPM int *) 0xF00B0100;
volatile _SPM int *i2cReqReg  = (volatile _SPM int *) 0xF00B0110;


int 	writeToI2C(char* addrC,char* dataC);
void 	setup();
int 	changeVolume(int vol);
void 	waitSyncDac();
void 	waitSyncAdc();


int 	setOutputBuffer(short l, short r);
int 	getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int     getInputBuffer(short *l, short *r);
int     setOutputBufferSize(int bufferSize);
int     setInputBufferSize(int bufferSize);

//audio operations
int     filterIIR(volatile _SPM int *pnt_i, volatile _SPM short (*x)[2], volatile _SPM short (*y)[2], volatile _SPM int *accum, volatile _SPM short *B, volatile _SPM short *A, int shiftLeft);

#endif

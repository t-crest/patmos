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
void 	setup(int guitar);
int 	changeVolume(int vol);
void 	waitSyncDac();
void 	waitSyncAdc();


int 	setOutputBuffer(short l, short r);
int 	getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int     getInputBuffer(volatile short *l, volatile short *r);
int     setOutputBufferSize(int bufferSize);
int     setInputBufferSize(int bufferSize);

//audio operations
int     filterIIR(int FILT_ORD_1PL, volatile _SPM int *pnt_i, volatile _SPM short (*x)[2], volatile _SPM short (*y)[2], volatile _SPM int *accum, volatile _SPM short *B, volatile _SPM short *A, int shiftLeft);
int     storeSin(int *sinArray, int SIZE, int OFFSET, int AMP);
int     storeSinInterpol(int *sinArray, short *fracArray, int SIZE, int OFFSET, int AMP);
int     filter_coeff_bp_br(int FILT_ORD_1PL, volatile _SPM short *B, volatile _SPM short *A, int Fc, int Fb, volatile _SPM int *shiftLeft, int fixedShift);
int     filter_coeff_hp_lp(int FILT_ORD_1PL, volatile _SPM short *B, volatile _SPM short *A, int Fc, float Q, volatile _SPM int *shiftLeft, int fixedShift, int type);
int     combFilter_1st(int AUDIO_BUFF_LEN, volatile _SPM int *pnt, volatile short (*audio_buffer)[2], volatile _SPM short *y, volatile _SPM int *accum, volatile _SPM short *g, volatile _SPM int *del);
int     combFilter_2nd(int AUDIO_BUFF_LEN, volatile _SPM int *pnt, volatile short (*audio_buffer)[2], volatile _SPM short *y, volatile _SPM int *accum, volatile _SPM short *g, volatile _SPM int *del);
int     distortion(volatile _SPM short *x, volatile _SPM short *y, volatile _SPM int *accum);
int     fuzz(volatile _SPM short *x, volatile _SPM short *y, volatile _SPM int *accum, const int K, const int KonePlus, const int shiftLeft);
int     overdrive(volatile _SPM short *x, volatile _SPM short *y, volatile _SPM int *accum);



//----------------------------COMPLETE AUDIO FUNCTIONS---------------------------------//



int alloc_vibrato_vars(int *ADDR);
int audio_vibrato(int VIBR_P, volatile _SPM short *x, volatile _SPM short *y);


#endif

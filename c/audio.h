//#ifndef FILTER_ORDER_1PLUS
//#define FILTER_ORDER_1PLUS 2
//#endif

#ifndef AUDIO_H
#define AUDIO_H

#define ONE_16b 0x7FFF
#define BUFFER_SIZE 128
#define Fs 52083 // Hz

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

/*
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
*/


//----------------------------COMPLETE AUDIO FUNCTIONS---------------------------------//

#define VIBRATO_L 150 // modulation amount in samples (amp of sin)
#define VIBRATO_P (int)(Fs/4) // period of vibrato (period of sin)

#define FILTER_ORDER_1PLUS 3 //order of IIR filters

#define DELAY_L (int)(Fs/4) // for delay

const int CORES_AMOUNT = 4;
int addr[CORES_AMOUNT] = {0};

struct AudioFX {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
};

void audioIn(struct AudioFX *thisFX);
void audioOut(struct AudioFX *thisFX);

//same core:
void audioChainCore(struct AudioFX *sourceFX, struct AudioFX *destinationFX);


/*
  High-Pass / Low-Pass filters (2nd order)
*/

struct HpfLpf {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM short (*x_buf)[2]; // input buffer
    volatile _SPM short (*y_buf)[2]; // output buffer
    volatile _SPM short *A; // [a2, a1,  1]
    volatile _SPM short *B; // [b2, b1, b0]
    volatile _SPM int   *pnt; //audio input pointer
    volatile _SPM int   *sftLft; //x or y buffer pointer
};

int alloc_hpfLpf_vars(struct HpfLpf *hpflpfP, int coreNumber, int Fc, float Q, int type);
int audio_hpfLpf(struct HpfLpf *hpflpfP);


/*
  Vibrato
*/

struct Vibrato {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM int   *del; // delay
    volatile _SPM short *frac; //fraction for interpol.
    volatile _SPM int   *pnt; //audio input pointer
    volatile _SPM int   *v_pnt; //vibrato array pointer
    volatile _SPM int   *audio_pnt; //audio output pointer
    volatile _SPM int   *n_audio_pnt; //next audio o. pointer
    //SRAM variables
    short audio_buff[VIBRATO_L][2];
    int sinArray[VIBRATO_P];
    short fracArray[VIBRATO_P];
};

int alloc_vibrato_vars(struct Vibrato *vibrP, int coreNumber);
int audio_vibrato(struct Vibrato *vibrP);

/*
  Overdrive and distortion
*/

struct Overdrive {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
};

int alloc_overdrive_vars(struct Overdrive *odP, int coreNumber);
int audio_overdrive(struct Overdrive *odP);

struct Distortion {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM int   *k; //for distortion
    volatile _SPM int   *kOnePlus; //for distortion
    volatile _SPM int   *sftLft; //shift left amount
};

int alloc_distortion_vars(struct Distortion *distP, int coreNumber, float amount);
int audio_distortion(struct Distortion *distP);


/*
  Delay
*/

struct IIRdelay {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM short *g; //gains [g1, g0]
    volatile _SPM int   *del; // delays [d1, d0]
    volatile _SPM int   *pnt; //audio input pointer
    //SRAM variables
    short audio_buff[DELAY_L][2];
};

int alloc_delay_vars(struct IIRdelay *delP, int coreNumber);
int audio_delay(struct IIRdelay *delP);

#endif

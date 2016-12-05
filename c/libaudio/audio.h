

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//for guitar:
#define GUITAR 0

// for multicore platform with noc:
#define MULTICORE 1

#define ONE_16b 0x7FFF
#define ONE_32b 0x7FFFFFFF
#define BUFFER_SIZE 128
#define Fs 52083 // Hz

// ADDRESSES FOR OCP
#if ( MULTICORE == 1 )
//#include "libnoc/noc.h"
#include "libmp/mp.h"
#include "libcorethread/corethread.h"
#define SPM_OFFSET   (unsigned int)NOC_SPM_BASE
#else
#define SPM_OFFSET   0
#endif

/*
How the effect is located in a core:
-INTERNAL: not connected to NoC
-NOC_R: receives data from NoC
*/
typedef enum {INTERNAL, NOC_R, NOC_S, NOC_RS} fx_location_t;


#define LED_ADDR       (volatile _SPM int *) 0xF0090000
#define KEY_ADDR       (volatile _SPM int *) 0xF00A0000
#define DACL_ADDR      (volatile _SPM int *) 0xF00B0000
#define DACR_ADDR      (volatile _SPM int *) 0xF00B0010
#define DACEN_ADDR     (volatile _SPM int *) 0xF00B0020
#define DACBUFS_ADDR   (volatile _SPM int *) 0xF00B0040
#define DACBUFWP_ADDR  (volatile _SPM int *) 0xF00B0050
#define DACBUFFU_ADDR  (volatile _SPM int *) 0xF00B0060
#define ADCL_ADDR      (volatile _SPM int *) 0xF00B0080
#define ADCR_ADDR      (volatile _SPM int *) 0xF00B0090
#define ADCEN_ADDR     (volatile _SPM int *) 0xF00B00A0
#define ADCBUFS_ADDR   (volatile _SPM int *) 0xF00B00B0
#define ADCBUFRP_ADDR  (volatile _SPM int *) 0xF00B00C0
#define ADCBUFEM_ADDR  (volatile _SPM int *) 0xF00B00D0
#define I2CDATA_ADDR   (volatile _SPM int *) 0xF00B00E0
#define I2CADDR_ADDR   (volatile _SPM int *) 0xF00B00F0
#define I2CACK_ADDR    (volatile _SPM int *) 0xF00B0100
#define I2CREQ_ADDR    (volatile _SPM int *) 0xF00B0110

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


int 	writeToI2C(char* addrC,char* dataC);
void 	setup(int guitar);
int 	changeVolume(int vol);

int 	setOutputBuffer(short l, short r);
int     setOutputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int     getInputBuffer(volatile short *l, volatile short *r);
int 	getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r);
int     setOutputBufferSize(int bufferSize);
int     setInputBufferSize(int bufferSize);


//----------------------------COMPLETE AUDIO FUNCTIONS---------------------------------//


//for vibrato:
#define VIBRATO_L 150 // modulation amount in samples (amp of sin)
#define VIBRATO_P (int)(Fs/4) // period of vibrato (period of sin)
//#define FILTER_ORDER_1PLUS 3 //order of IIR filters
// for delay
#define DELAY_L (int)(Fs/4)
//for chorus
#define CHORUS_P1 Fs //period of 1st chorus
#define CHORUS_P2 (int)(4*Fs/5) //period of 2nd chorus
#define CHORUS_L  2083 // modulation amount in samples
//for tremolo
# define TREMOLO_P (int)(Fs/4)
//for WahWah
#define WAHWAH_P (int)(Fs*0.6)
#define WAHWAH_FC_CEN 1200
#define WAHWAH_FC_AMP 900
#define WAHWAH_FB_CEN 330
#define WAHWAH_FB_AMP 300
    //constants
#define WAHWAH_DRY_GAIN (int)(ONE_16b*0.2)
#define WAHWAH_WET_GAIN (int)(ONE_16b*0.8)

/*
  GENERAL
*/

//DEBUG STUFF
const int DEBUG_ELEMENTS = 4;
const int DEBUG_LOOPLENGTH = 64;

// first/last: connection to AudioInterface
typedef enum {NO_FIRST, FIRST} fst_t;
typedef enum {NO_LAST, LAST} lst_t;
// connected to same core or to NoC
typedef enum {NO_NOC, NOC} con_t;
// comparison of receive/send buffer sizes
typedef enum {XeY, XgY, XlY} pt_t;
// possible effects:
typedef enum {DRY, DRY_8S, HP, LP, BP, BR,
             VIBRATO, OVERDRIVE,
             DISTORTION, TREMOLO,
             DELAY, CHORUS, WAHWAH} fx_t;

struct AudioFX {
    //effect ID
    volatile _SPM int *fx_id;
    //core number
    volatile _SPM int *cpuid;
    //connection type
    volatile _SPM fst_t *is_fst; // audio input node
    volatile _SPM lst_t *is_lst; // audio output node
    volatile _SPM con_t *in_con;  //input  connection: same core or NoC
    volatile _SPM con_t *out_con; //output connection: same core or NoC
    //pointers to SPM data
    volatile _SPM unsigned int *x_pnt; //pointer to x location
    volatile _SPM unsigned int *y_pnt; //pointer to y location
    //send and receive NoC channel pointers
    volatile _SPM unsigned int *sendChanP;
    volatile _SPM unsigned int *recvChanP;
    //processing type
    volatile _SPM pt_t *pt;
    //parameters: P, RPR, SPR, PPSR
    volatile _SPM unsigned int *p;
    volatile _SPM unsigned int *rpr;
    volatile _SPM unsigned int *spr;
    volatile _SPM unsigned int *ppsr;
    //in and out buffer size ( both for NoC or same core, in samples, multiples of 2)
    volatile _SPM unsigned int *xb_size; //x buffer
    volatile _SPM unsigned int *yb_size; //y buffer
    //audio data
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    //Audio effect implemented
    volatile _SPM fx_t *fx;
    /*
    //pointer to FX processing function
    volatile _SPM unsigned int *funcP;
    */
};

//audio FX SPM allocation
int alloc_audio_vars(struct AudioFX *audioP, int FX_ID, fx_t FX_TYPE, con_t in_con, con_t out_con, unsigned int IN_SIZE, unsigned int OUT_SIZE, unsigned int P_AMOUNT, fst_t is_fst, lst_t is_lst);

//same core:
int audio_connect_same_core(struct AudioFX *srcP, struct AudioFX *dstP);
//NoC:
int audio_connect_to_core(struct AudioFX *srcP, const unsigned int sendChanID);
int audio_connect_from_core(const unsigned int recvChanID, struct AudioFX *dstP);

//audio processing
int audio_process(struct AudioFX *audioP, volatile _UNCACHED int *dataP) __attribute__((section("text.spm")));


/*
/ *
  High-Pass / Low-Pass / Band-Pass / Band-Reject filters (2nd order)
* /

struct Filter {
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
    volatile _SPM int   *type; // to choose between HP, LP, BP or BR
};

int alloc_filter_vars(struct Filter *filterP, int coreNumber, int Fc, float QorFb, int type);
int audio_filter(struct Filter *filterP);

struct Filter32 {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM long long int   *accum; //accummulator accum[2]
    volatile _SPM short (*x_buf)[2]; // input buffer
    volatile _SPM short (*y_buf)[2]; // output buffer
    volatile _SPM int *A; // [a2, a1,  1]
    volatile _SPM int *B; // [b2, b1, b0]
    volatile _SPM int   *pnt; //audio input pointer
    volatile _SPM int   *sftLft; //x or y buffer pointer
    volatile _SPM int   *type; // to choose between HP, LP, BP or BR
};

int alloc_filter32_vars(struct Filter32 *filterP, int coreNumber, int Fc, float QorFb, int type);
int audio_filter32(struct Filter32 *filterP);


/ *
  Vibrato
* /

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
    //Main Memory variables
    short audio_buff[VIBRATO_L][2];
    int sinArray[VIBRATO_P];
    short fracArray[VIBRATO_P];
};

int alloc_vibrato_vars(struct Vibrato *vibrP, int coreNumber);
int audio_vibrato(struct Vibrato *vibrP);

/ *
  Overdrive and distortion
* /

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


/ *
  Delay
* /

struct IIRdelay {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM short *g; //gains [g1, g0]
    volatile _SPM int   *del; // delays [d1, d0]
    volatile _SPM int   *pnt; //audio input pointer
    //Main Memory variables
    short audio_buff[DELAY_L][2];
};

int alloc_delay_vars(struct IIRdelay *delP, int coreNumber);
int audio_delay(struct IIRdelay *delP);

/ *
  Chorus
* /

struct Chorus {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM short *g; // gains [g2, g1, g0]
    volatile _SPM int   *del; // delays [d2, d1, d0]
    volatile _SPM int   *pnt; //audio input pointer
    volatile _SPM int   *c1_pnt; //1st mod array pointer
    volatile _SPM int   *c2_pnt; //2nd mod array pointer
    //Main Memory variables
    short audio_buff[CHORUS_L][2];
    int modArray1[CHORUS_P1];
    int modArray2[CHORUS_P2];
};

int alloc_chorus_vars(struct Chorus *chorP, int coreNumber);
int audio_chorus(struct Chorus *chorP);

/ *
  Tremolo
* /

struct Tremolo {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *pnt; //modulation pointer
    volatile _SPM int   *pnt_n; //modulation pointer next
    volatile _SPM short *frac; //fraction of modulation
    volatile _SPM short *frac1Minus; //1 - frac
    volatile _SPM int   *mod; //interpolated mod value
    //Main Memory variables
    int modArray[TREMOLO_P];
    short fracArray[TREMOLO_P];
};

int alloc_tremolo_vars(struct Tremolo *tremP, int coreNumber);
int audio_tremolo(struct Tremolo *tremP);

struct Tremolo32 {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *pnt; //modulation pointer
    //Main Memory variables
    long long int modArray[TREMOLO_P];
};

int alloc_tremolo32_vars(struct Tremolo32 *tremP, int coreNumber);
int audio_tremolo32(struct Tremolo32 *tremP);

/ *
  Wah-Wah
* /

struct WahWah {
    //SPM variables
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    volatile _SPM int   *accum; //accummulator accum[2]
    volatile _SPM short (*x_buf)[2]; // input buffer
    volatile _SPM short (*y_buf)[2]; // output buffer
    volatile _SPM short *A; // [a2, a1,  1]
    volatile _SPM short *B; // [b2, b1, b0]
    volatile _SPM int   *pnt; //audio input pointer
    volatile _SPM int   *wah_pnt; //modulation pointer
    volatile _SPM int   *sftLft; //x or y buffer pointer
    //Main Memory Variables
    int fcArray[WAHWAH_P]; //for Fc
    int fbArray[WAHWAH_P]; //for Fb
    short aArray[WAHWAH_P][3]; //for A coefficients
    short bArray[WAHWAH_P][3]; //for B coefficients
};

int alloc_wahwah_vars(struct WahWah *wahP, int coreNumber);
int audio_wahwah(struct WahWah *wahP);


*/

#endif /* _AUDIO_H_ */

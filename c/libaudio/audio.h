#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libmp/mp.h"
#include "libcorethread/corethread.h"

#include "audioinit.h"
#include "latencyinit.h"


//for guitar:
#define GUITAR 0

// for multicore platform with noc:
#define MULTICORE 1

#define ONE_16b 0x7FFF
#define ONE_32b 0x7FFFFFFF
#define Fs 52083 // Hz

// ADDRESSES FOR OCP
#if ( MULTICORE == 1 )
#define SPM_OFFSET   (unsigned int)NOC_SPM_BASE
#else
#define SPM_OFFSET   0
#endif


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
#define VIBRATO_L 200 // modulation amount in samples (amp of sin)
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

unsigned int current_mode = 0; // [MODES]

//DEBUG STUFF
//const int DEBUG_ELEMENTS = 4;
//const int DEBUG_LOOPLENGTH = 64;

//
//type of connection: first last, to NoC, or to same core
typedef enum {FIRST, LAST, NOC, SAME} con_t;
// comparison of receive/send buffer sizes
typedef enum {XeY, XgY, XlY} pt_t;
// possible effects:
typedef enum {DRY, DRY_8S, DELAY,
              OVERDRIVE, WAHWAH,
              CHORUS, DISTORTION,
              HP, LP, BP, BR,
              VIBRATO,
              TREMOLO} fx_t;

struct AudioFX {
    //effect ID
    _SPM int *fx_id;
    //core number
    _SPM int *cpuid;
    //connection type
    _SPM con_t *in_con;
    _SPM con_t *out_con;
    //amount of send and receive channels (fork or join effects)
    _SPM unsigned int *send_am;
    _SPM unsigned int *recv_am;
    //pointers to SPM data
    _SPM unsigned int *x_pnt; //pointer to x location
    _SPM unsigned int *y_pnt; //pointer to y location
    //receive and send NoC channel pointers
    _SPM unsigned int *recvChanP;
    _SPM unsigned int *sendChanP;
    //processing type
    _SPM pt_t *pt;
    //parameters: S, Nr, Ns, Nf
    _SPM unsigned int *s;
    _SPM unsigned int *Nr;
    _SPM unsigned int *Ns;
    _SPM unsigned int *Nf;
    //in and out buffer size ( both for NoC or same core, in samples, multiples of 2)
    _SPM unsigned int *xb_size; //x buffer
    _SPM unsigned int *yb_size; //y buffer
    //audio data
    volatile _SPM short *x; //input audio x[2]
    volatile _SPM short *y; //output audio y[2]
    //Audio effect implemented
    _SPM fx_t *fx;
    //Pointer to effect struct
    _SPM unsigned int *fx_pnt;
    //Boolean variable for last types: checks need to wait for output
    _SPM int *last_init;
    //Latency counter (from input to output)
    _SPM unsigned int *last_count;
    _SPM unsigned int *latency;
};

//audio FX SPM allocation
int alloc_audio_vars(struct AudioFX *audioP, int FX_ID, fx_t FX_TYPE, con_t in_con, con_t out_con, unsigned int RECV_AM, unsigned int SEND_AM, unsigned int IN_SIZE, unsigned int OUT_SIZE, unsigned int P_AMOUNT, unsigned int LAT);

int free_audio_vars(struct AudioFX *audioP);

//same core:
int audio_connect_same_core(struct AudioFX *srcP, struct AudioFX *dstP);
//NoC:
int audio_connect_to_core(struct AudioFX *srcP, const unsigned int sendChanID, unsigned int s_ind);
int audio_connect_from_core(const unsigned int recvChanID, struct AudioFX *dstP, unsigned int r_ind);

//audio processing
int audio_process(struct AudioFX *audioP); // __attribute__((section("text.spm")));



/*
  High-Pass / Low-Pass / Band-Pass / Band-Reject filters (2nd order)
*/

struct Filter {
    int   accum[2]; //accummulator accum[2]
    short x_buf[3][2]; // input buffer
    short y_buf[3][2]; // output buffer
    short A[3]; // [a2, a1,  1]
    short B[3]; // [b2, b1, b0]
    int   pnt; //audio input pointer
    int   sftLft; //x or y buffer pointer
    int   type; // to choose between HP, LP, BP or BR
};

/*
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
*/

/*
  Vibrato
*/

struct Vibrato {
    int   accum[2]; //accummulator accum[2]
    int   del; // delay
    short frac; //fraction for interpol.
    int   pnt; //audio input pointer
    int   v_pnt; //vibrato array pointer
    int   audio_pnt; //audio output pointer
    int   n_audio_pnt; //next audio o. pointer
    //SRAM Memory variables
    short (*audio_buf_pnt)[VIBRATO_L]; //pointer to audio_buff[2][VIBRATO_L]
    int *sin_array_pnt; //pointer to sin_array[VIBRATO_P]
    short *frac_array_pnt; //pointer to frac_array[VIBRATO_P]
};

/*
  Overdrive and distortion
*/

struct Overdrive {
    int   accum[2]; //accummulator accum[2]
};

struct Distortion {
    int   accum[2]; //accummulator accum[2]
    int   k; //for distortion
    int   kOnePlus; //for distortion
    int   sftLft; //shift left amount
};

/*
  Delay
*/

struct IIRdelay {
    int   accum[2]; //accummulator accum[2]
    short g[2]; //gains [g1, g0]
    int   del[2]; // delays [d1, d0]
    int   pnt; //audio input pointer
    //SRAM Memory variables
    short (*audio_buf)[DELAY_L]; // audio_buff[2][DELAY_L]
};

/*
  Chorus
*/

struct Chorus {
    int   accum[2]; //accummulator accum[2]
    short g[3]; // gains [g2, g1, g0]
    int   del[3]; // delays [d2, d1, d0]
    int   pnt; //audio input pointer
    int   c1_pnt; //1st mod array pointer
    int   c2_pnt; //2nd mod array pointer
    //SRAM Memory variables
    short (*audio_buf)[CHORUS_L]; // audio_buf[2][CHORUS_L]
    int *mod_array1; // mod_array1[CHORUS_P1]
    int *mod_array2; // mod_array2[CHORUS_P2]
};


/*
  Tremolo
*/

struct Tremolo {
    int   pnt; //modulation pointer
    int   pnt_n; //modulation pointer next
    short frac; //fraction of modulation
    short frac1Minus; //1 - frac
    int   mod; //interpolated mod value
    //SRAM Memory variables
    int *mod_array; // mod_array[TREMOLO_P]
    short *frac_array; //frac_array[TREMOLO_P]
};


/*
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
*/

/*
  Wah-Wah
*/

struct WahWah {
    int   accum[2]; //accummulator accum[2]
    short x_buf[3][2]; // input buffer
    short y_buf[3][2]; // output buffer
    short A[3]; // [a2, a1,  1]
    short B[3]; // [b2, b1, b0]
    int   pnt; //audio input pointer
    int   wah_pnt; //modulation pointer
    int   sftLft; //x or y buffer pointer
    //SRAM Memory Variables
    int *fc_array; // fc_array[WAHWAH_P]
    int *fb_array; // fb_array[WAHWAH_P]
    short (*a_array)[WAHWAH_P]; //for A coefficients: a_array[3][WAHWAH_P]
    short (*b_array)[WAHWAH_P]; //for B coefficients: b_array[3][WAHWAH_p]
};


#endif /* _AUDIO_H_ */

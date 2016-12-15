#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#define ONE_16b 0x7FFF
#define Fs 52083 // Hz


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




struct Distortion {
    int   accum[2]; //accummulator accum[2]
    int   k; //for distortion
    int   kOnePlus; //for distortion
    int   sftLft; //shift left amount
};

struct Overdrive {
    int   accum[2]; //accummulator accum[2]
};

struct IIRdelay {
    int   accum[2]; //accummulator accum[2]
    short g[2]; //gains [g1, g0]
    int   del[2]; // delays [d1, d0]
    int   pnt; //audio input pointer
    //Shared Memory variables
    short (*audio_buf)[DELAY_L]; // audio_buff[2][DELAY_L]
};

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

struct Chorus {
    int   accum[2]; //accummulator accum[2]
    short g[3]; // gains [g2, g1, g0]
    int   del[3]; // delays [d2, d1, d0]
    int   pnt; //audio input pointer
    int   c1_pnt; //1st mod array pointer
    int   c2_pnt; //2nd mod array pointer
    //Shared Memory variables
    short (*audio_buf)[CHORUS_L]; // audio_buf[2][CHORUS_L]
    int *mod_array1; // mod_array1[CHORUS_P1]
    int *mod_array2; // mod_array2[CHORUS_P2]
};

struct Tremolo {
    int   pnt; //modulation pointer
    int   pnt_n; //modulation pointer next
    short frac; //fraction of modulation
    short frac1Minus; //1 - frac
    int   mod; //interpolated mod value
    //Shared Memory variables
    int *mod_array; // mod_array[TREMOLO_P]
    short *frac_array; //frac_array[TREMOLO_P]
};

struct Vibrato {
    int   accum[2]; //accummulator accum[2]
    int   del; // delay
    short frac; //fraction for interpol.
    int   pnt; //audio input pointer
    int   v_pnt; //vibrato array pointer
    int   audio_pnt; //audio output pointer
    int   n_audio_pnt; //next audio o. pointer
    //Shared Memory pointers
    short (*audio_buf_pnt)[VIBRATO_L]; //pointer to audio_buff[2][VIBRATO_L]
    int *sin_array_pnt; //pointer to sin_array[VIBRATO_P]
    short *frac_array_pnt; //pointer to frac_array[VIBRATO_P]
};

struct WahWah {
    int   accum[2]; //accummulator accum[2]
    short x_buf[3][2]; // input buffer
    short y_buf[3][2]; // output buffer
    short A[3]; // [a2, a1,  1]
    short B[3]; // [b2, b1, b0]
    int   pnt; //audio input pointer
    int   wah_pnt; //modulation pointer
    int   sftLft; //x or y buffer pointer
    //Shared Memory Variables
    int *fc_array; // fc_array[WAHWAH_P]
    int *fb_array; // fb_array[WAHWAH_P]
    short (*a_array)[WAHWAH_P]; //for A coefficients: a_array[3][WAHWAH_P]
    short (*b_array)[WAHWAH_P]; //for B coefficients: b_array[3][WAHWAH_p]
};


unsigned int alloc_filter_vars(_SPM struct Filter *filtP, int Fc, float QorFb, int thisType) {

    //calculate filter coefficients (3rd order)
    filtP->type = thisType;

    //give some random values to coefficients
    filtP->A[0] = ONE_16b * 0.3;
    filtP->A[1] = ONE_16b * 0.6;
    filtP->A[2] = 0;
    filtP->B[0] = ONE_16b * -0.3;
    filtP->B[1] = ONE_16b * -0.6;
    filtP->B[2] = ONE_16b * 0.6;

    filtP->pnt = 2;

    return 0;
}

unsigned int alloc_vibrato_vars(_SPM struct Vibrato *vibrP) {

    //modulation arrays
    vibrP->audio_buf_pnt = malloc(VIBRATO_L * 2 * sizeof(short)); // short audio_buf[2][VIBRATO_L]
    vibrP->sin_array_pnt  = malloc(VIBRATO_P * sizeof(int)); // int sin_array[VIBRATO_P]
    vibrP->frac_array_pnt = malloc(VIBRATO_P * sizeof(short)); // short frac_array[VIBRATO_P]

    //empty buffer
    for(int i=0; i<VIBRATO_L; i++) {
        vibrP->audio_buf_pnt[0][i] = 0;
        vibrP->audio_buf_pnt[1][i] = 0;
    }

    //initialise vibrato variables
    vibrP->pnt = VIBRATO_L - 1; //start on top
    vibrP->v_pnt = 0;

    return 0;
}

unsigned int alloc_wahwah_vars(_SPM struct WahWah *wahP) {

    //shift left is fixed!
    wahP->sftLft = 1;


    //modulation arrays
    wahP->fc_array = malloc(WAHWAH_P * sizeof(int)); // int fc_array[WAHWAH_P]
    wahP->fb_array = malloc(WAHWAH_P * sizeof(int)); // int fb_array[WAHWAH_P]
    wahP->a_array  = malloc(WAHWAH_P * 3 * sizeof(short)); // short a_array[3][WAHWAH_P]
    wahP->b_array  = malloc(WAHWAH_P * 3 * sizeof(short)); // short b_array[3][WAHWAH_P]

    /*
    //random array of coefficients
    for(int i=0; i<WAHWAH_P; i++) {
        wahP->b_array[2][i] = ONE_16b * 0.3;
        wahP->b_array[1][i] = ONE_16b * 0.6;
        wahP->b_array[0][i] = ONE_16b * -0.6;
        wahP->a_array[2][i] = 0;
        wahP->a_array[1][i] = ONE_16b * -0.6;
        wahP->a_array[0][i] = ONE_16b * 0.6;
    }
    */

    wahP->wah_pnt = 2;

    return 0;
}

unsigned int alloc_tremolo_vars(_SPM struct Tremolo *tremP) {

    //modulation arrays
    tremP->mod_array  = malloc(TREMOLO_P * sizeof(int)); // int mod_array[TREMOLO_P]
    tremP->frac_array = malloc(TREMOLO_P * sizeof(short)); // short frac_array[TREMOLO_P]

     //pointers:
    tremP->pnt = 0;
    tremP->pnt_n = 1;

    return 0;
}

unsigned int alloc_chorus_vars(_SPM struct Chorus *chorP) {

    //initialise chorus variables
    //set gains:
    chorP->g[2] = ONE_16b * 0.45; //g0
    chorP->g[1] = ONE_16b * 0.4;  //g1
    chorP->g[0] = ONE_16b * 0.4;  //g2
    //set delays:
    chorP->del[2] = 0; // always d0 = 0

    //data and modulation arrays
    chorP->audio_buf  = malloc(CHORUS_L * 2 * sizeof(short)); // short audio_buf[2][CHORUS_L]
    chorP->mod_array1 = malloc(CHORUS_P1 * sizeof(int)); // int mod_array1[CHORUS_P1]
    chorP->mod_array2 = malloc(CHORUS_P2 * sizeof(int)); // int mod_array2[CHORUS_P2]

    //empty buffer
    for(int i=0; i<CHORUS_L; i++) {
        chorP->audio_buf[0][i] = 0;
        chorP->audio_buf[1][i] = 0;
    }

     //pointers:
    chorP->pnt = CHORUS_L - 1; //starts on top
    chorP->c1_pnt = 0;
    chorP->c2_pnt = 0;

    return 0;
}

unsigned int alloc_distortion_vars(_SPM struct Distortion *distP) {

    //initialise k, kOnePlus, shiftLeft:
    float amount = 0.9;
    distP->k = 0x90000;
    distP->sftLft = 0;
    while(distP->k > ONE_16b) {
        distP->sftLft = distP->sftLft + 1;
        distP->k = distP->k >> 1;
    }
    distP->kOnePlus = 0x98000 >> distP->sftLft;

    return 0;
}

unsigned int alloc_delay_vars(_SPM struct IIRdelay *delP) {

    //initialise delay variables
    //set gains: for comb delay:
    delP->g[1] = ONE_16b;       // g0 = 1
    delP->g[0] = ONE_16b * 0.5; // g1 = 0.7
    //set delays:
    delP->del[1] = 0; // always d0 = 0
    delP->del[0] = DELAY_L - 1; // d1 = as long as delay buffer
    //pointer starts on top
    delP->pnt = DELAY_L - 1;

    //alloc audio array
    delP->audio_buf = malloc(DELAY_L * 2 * sizeof(short)); // short audio_buf[2][DELAY_L]

    //empty buffer
    for(int i=0; i<DELAY_L; i++) {
        delP->audio_buf[0][i] = 0;
        delP->audio_buf[1][i] = 0;
    }

    return 0;
}




int filterIIR_2nd(_SPM int *pnt_i, _SPM short (*x)[2], _SPM short (*y)[2], _SPM int *accum, _SPM short *B, _SPM short *A, _SPM int *shiftLeft) {
    int pnt; //pointer for x_filter
    accum[0] = 0;
    accum[1] = 0;
    for(int i=0; i<3; i++) { //FILTER_ORDER_1PLUS = 3
        pnt = (*pnt_i + i + 1) % 3; //FILTER_ORDER_1PLUS = 3
        // SIGNED SHIFT (arithmetical): losing a 2-bit resolution
        accum[0] += (B[i]*x[pnt][0]) >> 2;
        accum[0] -= (A[i]*y[pnt][0]) >> 2;
        accum[1] += (B[i]*x[pnt][1]) >> 2;
        accum[1] -= (A[i]*y[pnt][1]) >> 2;
    }
    //accumulator limits: [ (2^(30-2-1))-1 , -(2^(30-2-1)) ]
    //accumulator limits: [ 0x7FFFFFF, 0x8000000 ]
    // digital saturation
    for(int i=0; i<2; i++) {
        if (accum[i] > 0x7FFFFFF) {
            accum[i] = 0x7FFFFFF;
        }
        else {
            if (accum[i] < -0x8000000) {
                accum[i] = -0x8000000;
            }
        }
    }
    y[*pnt_i][0] = accum[0] >> (13-*shiftLeft);
    y[*pnt_i][1] = accum[1] >> (13-*shiftLeft);

    return 0;
}

int audio_filter(_SPM struct Filter *filtP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        //increment pointer
        filtP->pnt = ( filtP->pnt + 1 ) % 3;
        //first, read sample
        filtP->x_buf[filtP->pnt][0] = xP[index];
        filtP->x_buf[filtP->pnt][1] = xP[index+1];
        //then, calculate filter
        filterIIR_2nd(&filtP->pnt, filtP->x_buf, filtP->y_buf, filtP->accum, filtP->B, filtP->A, &filtP->sftLft);
        //check if it is BP/BR
        if(filtP->type == 2) { //BP
            filtP->accum[0] = ( (int)xP[index]   - (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
            filtP->accum[1] = ( (int)xP[index+1] - (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
        }
        else {
            if(filtP->type == 3) { //BR
                filtP->accum[0] = ( (int)xP[index]   + (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
                filtP->accum[1] = ( (int)xP[index+1] + (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
            }
            else { //HP or LP
                filtP->accum[0] = filtP->y_buf[filtP->pnt][0];
                filtP->accum[1] = filtP->y_buf[filtP->pnt][1];
            }
        }
        //set output
        yP[index]   = (short)filtP->accum[0];
        yP[index+1] = (short)filtP->accum[1];
    }

    return 0;
}

int audio_filter_2(_SPM struct Filter *filtP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        //increment pointer
        filtP->pnt = ( filtP->pnt + 1 ) % 3;
        //first, read sample
        filtP->x_buf[filtP->pnt][0] = xP[index];
        filtP->x_buf[filtP->pnt][1] = xP[index+1];
        //then, calculate filter
        filterIIR_2nd(&filtP->pnt, filtP->x_buf, filtP->y_buf, filtP->accum, filtP->B, filtP->A, &filtP->sftLft);
        //check if it is BP/BR
        if(filtP->type == 2) { //BP
            filtP->accum[0] = ( (int)xP[index]   - (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
            filtP->accum[1] = ( (int)xP[index+1] - (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
        }
        else {
            if(filtP->type == 3) { //BR
                filtP->accum[0] = ( (int)xP[index]   + (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
                filtP->accum[1] = ( (int)xP[index+1] + (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
            }
            else { //HP or LP
                filtP->accum[0] = filtP->y_buf[filtP->pnt][0];
                filtP->accum[1] = filtP->y_buf[filtP->pnt][1];
            }
        }
        //set output
        yP[index]   = (short)filtP->accum[0];
        yP[index+1] = (short)filtP->accum[1];
    }

    return 0;
}

int audio_vibrato(_SPM struct Vibrato *vibrP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        //update delay pointers
        vibrP->del = vibrP->sin_array_pnt[vibrP->v_pnt];
        vibrP->frac = vibrP->frac_array_pnt[vibrP->v_pnt];
        short frac1Minus = ONE_16b - vibrP->frac;
        vibrP->v_pnt = ( vibrP->v_pnt + 1 )%VIBRATO_P;
        //vibrato pointers:
        vibrP->audio_pnt   = (vibrP->pnt+vibrP->del)%VIBRATO_L;
        vibrP->n_audio_pnt = (vibrP->pnt+vibrP->del+1)%VIBRATO_L;
        _Pragma("loopbound min 2 max 2")
        for(int j=0; j<2; j++) { //stereo
            int index = 2 * i + j;
            //first, read sample
            vibrP->audio_buf_pnt[j][vibrP->pnt] = xP[index];
            vibrP->accum[j] =  (vibrP->audio_buf_pnt[j][vibrP->n_audio_pnt] * (vibrP->frac));
            vibrP->accum[j] += (vibrP->audio_buf_pnt[j][vibrP->audio_pnt]   * (frac1Minus));
            yP[index] = vibrP->accum[j] >> 15;
        }
        //update input pointer
        if(vibrP->pnt == 0) {
            vibrP->pnt = VIBRATO_L - 1;
        }
        else {
            vibrP->pnt = vibrP->pnt - 1;
        }
    }

    return 0;
}

int audio_wahwah(_SPM struct WahWah *wahP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        //update filter coefficients
        wahP->B[2] = wahP->b_array[2][wahP->wah_pnt]; //b0
        wahP->B[1] = wahP->b_array[1][wahP->wah_pnt]; //b1
        // b2 doesnt need to be updated: always 1
        wahP->A[1] = wahP->a_array[1][wahP->wah_pnt]; //a1
        wahP->A[0] = wahP->a_array[0][wahP->wah_pnt]; //a2
        //update pointers
        wahP->wah_pnt = (wahP->wah_pnt+1) % WAHWAH_P;
        wahP->pnt = (wahP->pnt+1) % 3; //FILTER_ORDER_1PLUS = 3
        //first, read sample
        wahP->x_buf[wahP->pnt][0] = xP[index];
        wahP->x_buf[wahP->pnt][1] = xP[index+1];
        //then, calculate filter
        filterIIR_2nd(&wahP->pnt, wahP->x_buf, wahP->y_buf, wahP->accum, wahP->B, wahP->A, &wahP->sftLft);
        //Band-Pass stuff
        wahP->accum[0] = ( (int)xP[index]   - (int)wahP->y_buf[wahP->pnt][0] ); // >> 1;
        wahP->accum[1] = ( (int)xP[index+1] - (int)wahP->y_buf[wahP->pnt][1] ); // >> 1;
        //mix with original: gains are fixed
        wahP->accum[0] = ( (int)(WAHWAH_WET_GAIN*wahP->accum[0]) >> 15 )  + ( (int)(WAHWAH_DRY_GAIN*xP[index])   >> 15 );
        wahP->accum[1] = ( (int)(WAHWAH_WET_GAIN*wahP->accum[1]) >> 15 )  + ( (int)(WAHWAH_DRY_GAIN*xP[index+1]) >> 15 );
        //set output
        yP[index]   = (short)wahP->accum[0];
        yP[index+1] = (short)wahP->accum[1];
    }

    return 0;
}

int audio_tremolo(_SPM struct Tremolo *tremP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        //update pointer
        tremP->pnt = (tremP->pnt + 1) % TREMOLO_P;
        tremP->pnt_n = (tremP->pnt_n + 1) % TREMOLO_P;
        //modulation values
        tremP->frac = tremP->frac_array[tremP->pnt];
        tremP->frac1Minus = ONE_16b - tremP->frac;
        tremP->mod  = tremP->mod_array[tremP->pnt] * tremP->frac1Minus;
        tremP->mod += tremP->mod_array[tremP->pnt_n] * tremP->frac;
        tremP->mod = tremP->mod >> 15;
        //calculate output
        yP[index]   = (xP[index]   * tremP->mod) >> 15;
        yP[index+1] = (xP[index+1] * tremP->mod) >> 15;
    }

    return 0;
}

__attribute__((always_inline))
int combFilter_2nd(int AUDIO_BUF_LEN, _SPM int *pnt, short (*audio_buffer)[AUDIO_BUF_LEN], volatile _SPM short *y, _SPM int *accum, _SPM short *g, _SPM int *del) {
    accum[0] = 0;
    accum[1] = 0;
    int audio_pnt = (*pnt+del[0])%AUDIO_BUF_LEN;
    accum[0] += (g[0]*audio_buffer[0][audio_pnt]) >> 2;
    accum[1] += (g[0]*audio_buffer[1][audio_pnt]) >> 2;
    audio_pnt = (*pnt+del[1])%AUDIO_BUF_LEN;
    accum[0] += (g[1]*audio_buffer[0][audio_pnt]) >> 2;
    accum[1] += (g[1]*audio_buffer[1][audio_pnt]) >> 2;
    audio_pnt = (*pnt+del[2])%AUDIO_BUF_LEN;
    accum[0] += (g[2]*audio_buffer[0][audio_pnt]) >> 2;
    accum[1] += (g[2]*audio_buffer[1][audio_pnt]) >> 2;
    //accumulator limits: [ (2^(30-2-1))-1 , -(2^(30-2-1)) ]
    //accumulator limits: [ 0x7FFFFFF, 0x8000000 ]
    // digital saturation
    for(int i=0; i<2; i++) {
        if (accum[i] > 0x7FFFFFF) {
            accum[i] = 0x7FFFFFF;
        }
        else {
            if (accum[i] < -0x8000000) {
                accum[i] = -0x8000000;
            }
        }
    }
    y[0] = accum[0] >> 13;
    y[1] = accum[1] >> 13;

    return 0;
}

int audio_chorus(_SPM struct Chorus *chorP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        // SINUSOIDAL MODULATION OF DELAY LENGTH
        chorP->del[0] = chorP->mod_array1[chorP->c1_pnt];
        chorP->del[1] = chorP->mod_array2[chorP->c2_pnt];
        chorP->c1_pnt = (chorP->c1_pnt + 1) % CHORUS_P1;
        chorP->c2_pnt = (chorP->c2_pnt + 1) % CHORUS_P2;
        //first, read sample
        chorP->audio_buf[0][chorP->pnt] = xP[index];
        chorP->audio_buf[1][chorP->pnt] = xP[index+1];
        //calculate AUDIO comb filter
        combFilter_2nd(CHORUS_L, &chorP->pnt, chorP->audio_buf, &yP[index], chorP->accum, chorP->g, chorP->del);
        //update pointer
        if(chorP->pnt == 0) {
            chorP->pnt = CHORUS_L - 1;
        }
        else {
            chorP->pnt = chorP->pnt - 1;
        }
    }

    return 0;
}

__attribute__((always_inline))
int combFilter_1st(int AUDIO_BUF_LEN, _SPM int *pnt, short (*audio_buffer)[AUDIO_BUF_LEN], volatile _SPM short *y, _SPM int *accum, _SPM short *g, _SPM int *del) {
    accum[0] = 0;
    accum[1] = 0;
    int audio_pnt = (*pnt+del[0])%AUDIO_BUF_LEN;
    accum[0] += (g[0]*audio_buffer[0][audio_pnt]) >> 2;
    accum[1] += (g[0]*audio_buffer[1][audio_pnt]) >> 2;
    audio_pnt = (*pnt+del[1])%AUDIO_BUF_LEN;
    accum[0] += (g[1]*audio_buffer[0][audio_pnt]) >> 2;
    accum[1] += (g[1]*audio_buffer[1][audio_pnt]) >> 2;
    //accumulator limits: [ (2^(30-2-1))-1 , -(2^(30-2-1)) ]
    //accumulator limits: [ 0x7FFFFFF, 0x8000000 ]
    // digital saturation
    for(int i=0; i<2; i++) {
        if (accum[i] > 0x7FFFFFF) {
            accum[i] = 0x7FFFFFF;
        }
        else {
            if (accum[i] < -0x8000000) {
                accum[i] = -0x8000000;
            }
        }
    }
    y[0] = accum[0] >> 13;
    y[1] = accum[1] >> 13;

    return 0;
}

int audio_delay(_SPM struct IIRdelay *delP, volatile _SPM short *xP, volatile _SPM short *yP) {
    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        int index = 2 * i;
        //first, read sample
        delP->audio_buf[0][delP->pnt] = xP[index];
        delP->audio_buf[1][delP->pnt] = xP[index+1];
        //calculate IIR comb filter
        combFilter_1st(DELAY_L, &delP->pnt, delP->audio_buf, &yP[index], delP->accum, delP->g, delP->del);
        //replace content on buffer
        delP->audio_buf[0][delP->pnt] = yP[index];
        delP->audio_buf[1][delP->pnt] = yP[index+1];
        //update pointer
        if(delP->pnt == 0) {
            delP->pnt = DELAY_L - 1;
        }
        else {
            delP->pnt = delP->pnt -1;
        }
    }

    return 0;
}

int audio_overdrive(_SPM struct Overdrive *odP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //THRESHOLD IS 1/3 = 0x2AAB
    //input abs:
    unsigned int x_abs[2];

    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {
        _Pragma("loopbound min 2 max 2")
        for(int j=0; j<2; j++) {
            int index = 2 * i + j;
            x_abs[j] = abs(xP[index]);
            if(x_abs[j] > (2 * 0x2AAB)) { // saturation : y = 1
                if (xP[index] > 0) {
                    yP[index] = 0x7FFF;
                }
                else {
                    yP[index] = 0x8000;
                }
            }
            else {
                if(x_abs[j] > 0x2AAB) { // smooth overdrive: y = ( 3 - (2-3*x)^2 ) / 3;
                    odP->accum[j] = (0x17FFF * x_abs[j]) >> 15 ; // result is 1 sign + 17 bits
                    odP->accum[j] = 0xFFFF - odP->accum[j];
                    odP->accum[j] = (odP->accum[j] * odP->accum[j]) >> 15;
                    odP->accum[j] = 0x17FFF - odP->accum[j];
                    odP->accum[j] = (odP->accum[j] * 0x2AAB) >> 15;
                    if(xP[index] > 0) { //positive
                        if(odP->accum[j] > 32767) {
                            yP[index] = 32767;
                        }
                        else {
                            yP[index] = odP->accum[j];
                        }
                    }
                    else { // negative
                        yP[index] = -odP->accum[j];
                    }
                }
                else { // linear zone: y = 2*x
                    yP[index] = xP[index] << 1;
                }
            }
        }
    }

    return 0;
}

int audio_distortion(_SPM struct Distortion *distP, volatile _SPM short *xP, volatile _SPM short *yP) {

    _Pragma("loopbound min 100 max 100")
    for(int i=0; i<100; i++) {

        _Pragma("loopbound min 2 max 2")
        for(int j=0; j<2; j++) {
            int index = 2 * i + j;
            distP->accum[0] = (distP->kOnePlus * xP[index]);// >> 15;
            distP->accum[1] = (distP->k * abs(xP[index])) >> 15;
            distP->accum[1] = distP->accum[1] + ((ONE_16b+distP->sftLft) >> distP->sftLft);
            distP->accum[0] = distP->accum[0] / distP->accum[1];
            //reduce if it is poisitive only
            if (xP[index] > 0) {
                yP[index] = distP->accum[0] - 1;
            }
            else {
                yP[index] = distP->accum[0];
            }
        }
    }

    return 0;
}



const unsigned int OFFS_ADDR = 0x00000000;

int main() {

    unsigned int LAST_ADDR = OFFS_ADDR; //correct?

    _SPM struct IIRdelay *delayP;
    delayP = (_SPM struct IIRdelay *) LAST_ADDR;
    alloc_delay_vars(delayP);
    LAST_ADDR += sizeof(struct IIRdelay);

    _SPM struct Overdrive *overdriveP;
    overdriveP = (_SPM struct Overdrive *) LAST_ADDR;
    //nothing to allocate
    LAST_ADDR += sizeof(struct Overdrive);

    _SPM struct WahWah *wahwahP;
    wahwahP = (_SPM struct WahWah *) LAST_ADDR;
    alloc_wahwah_vars(wahwahP);
    LAST_ADDR += sizeof(struct WahWah);

    _SPM struct Chorus *chorusP;
    chorusP = (_SPM struct Chorus *) LAST_ADDR;
    alloc_chorus_vars(chorusP);
    LAST_ADDR += sizeof(struct Chorus);

    _SPM struct Distortion *distortionP;
    distortionP = (_SPM struct Distortion *) LAST_ADDR;
    alloc_distortion_vars(distortionP);
    LAST_ADDR += sizeof(struct Distortion);

    _SPM struct Filter *hpfP;
    hpfP = (_SPM struct Filter *) LAST_ADDR;
    alloc_filter_vars(hpfP, 5000, 0.707, 1);
    LAST_ADDR += sizeof(struct Filter);

    _SPM struct Filter *bpfP;
    bpfP = (_SPM struct Filter *) LAST_ADDR;
    alloc_filter_vars(hpfP, 1000, 300, 2);
    LAST_ADDR += sizeof(struct Filter);

    _SPM struct Vibrato *vibratoP;
    vibratoP = (_SPM struct Vibrato *) LAST_ADDR;
    alloc_vibrato_vars(vibratoP);
    LAST_ADDR += sizeof(struct Vibrato);

    _SPM struct Tremolo *tremoloP;
    tremoloP = (_SPM struct Tremolo *) LAST_ADDR;
    alloc_tremolo_vars(tremoloP);
    LAST_ADDR += sizeof(struct Tremolo);

    //X and Y data
    volatile _SPM short * xP;
    volatile _SPM short * yP;
    xP = (volatile _SPM short *) LAST_ADDR;
    yP = (volatile _SPM short *) (LAST_ADDR + 2 * 100 * sizeof(short));

    //initialise some data
    for(int i=0; i<100; i++) {
        xP[i*2]   = ONE_16b * (float)(i+1)/(float)100; //from 0 to 1
        yP[i*2+1] = ONE_16b - (ONE_16b * (float)(i+1)/(float)100); //from 1 to 0
    }

    //Audio FX processing (100 samples each function)
    audio_delay(delayP, xP, yP);
    audio_overdrive(overdriveP, xP, yP);
    audio_wahwah(wahwahP, xP, yP);
    audio_chorus(chorusP, xP, yP);
    audio_distortion(distortionP, xP, yP);
    audio_filter(hpfP, xP, yP);
    audio_filter_2(bpfP, xP, yP);
    audio_vibrato(vibratoP, xP, yP);
    audio_tremolo(tremoloP, xP, yP);

    //free memory
    free(delayP->audio_buf);
    free(wahwahP->fc_array);
    free(wahwahP->fb_array);
    free(wahwahP->a_array);
    free(wahwahP->b_array);
    free(chorusP->audio_buf);
    free(chorusP->mod_array1);
    free(chorusP->mod_array2);
    free(vibratoP->audio_buf_pnt);
    free(vibratoP->sin_array_pnt);
    free(vibratoP->frac_array_pnt);
    free(tremoloP->mod_array);
    free(tremoloP->frac_array);

    return 0;
}

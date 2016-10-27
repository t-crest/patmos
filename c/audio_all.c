#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF
#define BUFFER_SIZE 128
#define FILTER_ORDER_1PLUS 3
#define COMB_FILTER_ORDER_1PLUS 3 //max for chorus
#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"

#define VIBRATO_LENGTH 150

//-----------------LOCATION IN SCRATCHPAD MEMORY------------------------//
#define FX_ADDR     0x00000000
#define ACCUM_ADDR   ( FX_ADDR      + sizeof(int) )
//for simple input/output:
#define X_ADDR       ( ACCUM_ADDR   + 2 * sizeof(int) )
#define Y_ADDR       ( X_ADDR       + 2 * sizeof(short) )
//for filter buffers:
#define B_ADDR       ( Y_ADDR       + 2 * sizeof(short) )
#if ( (FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define A_ADDR       ( B_ADDR       + FILTER_ORDER_1PLUS * sizeof(short) )
#define X_FILT_ADDR  ( A_ADDR       + FILTER_ORDER_1PLUS * sizeof(short) )
#else //if it's odd, align with 4-byte word:
#define A_ADDR       ( B_ADDR       + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#define X_FILT_ADDR  ( A_ADDR       + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#endif
#define Y_FILT_ADDR  ( X_FILT_ADDR  + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define OUTREG_ADDR  ( Y_FILT_ADDR  + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR     ( OUTREG_ADDR  + 2 * sizeof(int) )
#define MOD_PNT_ADDR ( PNT_ADDR     + sizeof(int) )
#define SFTLFT_ADDR  ( MOD_PNT_ADDR + sizeof(int) )
#define G_ADDR       ( SFTLFT_ADDR  + sizeof(int) )
#if ( (COMB_FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define DEL_ADDR     ( G_ADDR       + COMB_FILTER_ORDER_1PLUS * sizeof(short) )
#else // if it's odd
#define DEL_ADDR     ( G_ADDR       + COMB_FILTER_ORDER_1PLUS * sizeof(short) + 2 ) //to align with 4-byte word
#endif
#define TREM_P_ADDR  ( DEL_ADDR     + COMB_FILTER_ORDER_1PLUS * sizeof(int) )
#define VIBR_P_ADDR  ( TREM_P_ADDR  + sizeof(int) )
#define ECHO_L_ADDR  ( VIBR_P_ADDR  + sizeof(int) )

//--------------------variables in local SPM------------------------------//
volatile _SPM int *FX              = (volatile _SPM int *)        FX_ADDR;
volatile _SPM int *accum           = (volatile _SPM int *)        ACCUM_ADDR;

volatile _SPM short *x             = (volatile _SPM short *)      X_ADDR; //input audio
volatile _SPM short *y             = (volatile _SPM short *)      Y_ADDR; //output audio

volatile _SPM short *B             = (volatile _SPM short *)      B_ADDR; // [b2, b1, b0]
volatile _SPM short *A             = (volatile _SPM short *)      A_ADDR; // [a2, a1,  1]
volatile _SPM short (*x_filter)[2] = (volatile _SPM short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM short (*y_filter)[2] = (volatile _SPM short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM int *outputReg       = (volatile _SPM int *)        OUTREG_ADDR; //needed for BPF, wah,,,

volatile _SPM int *pnt             = (volatile _SPM int *)        PNT_ADDR; // audio pointer
volatile _SPM int *mod_pnt         = (volatile _SPM int *)        MOD_PNT_ADDR; // modulation pointer
volatile _SPM int *shiftLeft       = (volatile _SPM int *)        SFTLFT_ADDR; //shift left amount;
//for comb filter effects:
volatile _SPM short *g             = (volatile _SPM short *)      G_ADDR; // g[COMB_FILTER_ORDER_1PLUS]: array of gains [... g2, g1, g0]
volatile _SPM int *del             = (volatile _SPM int *)        DEL_ADDR; // del[COMB_FILTER_ORDER_1PLUS]: array of delays [...d2, d1, 0]
//SPM MACROS
volatile _SPM int *TREMOLO_P       = (volatile _SPM int *)        TREM_P_ADDR; //tremolo period
volatile _SPM int *VIBRATO_P       = (volatile _SPM int *)        VIBR_P_ADDR; //vibrato period
volatile _SPM int *ECHO_LENGTH     = (volatile _SPM int *)        ECHO_L_ADDR; //length of echo

//------------------  variables in external SRAM---------------------------//
int sinArray[Fs]; //max 1 second
int usedArray[Fs]; //used by all effects
//FOR COMB FILTER EFFECTS:
volatile short audio_buffer[(int)Fs/2][2]; //up to 1/2 second



int main() {

    int exit = 0;
    float arrayDivider;

    setup(1); // 1 for guitar, 0 for volca

    //store sin: 1 second betwen -1 and 1
    storeSin(sinArray, Fs, 0, ONE_16b);

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    while(exit == 0) {
        int chooseFX;
        printf("Effects list:\n 00 -> Exit\n 01 -> Dry Sound (no effect)\n 02 -> Tremolo\n 03 -> High-Pass Filter\n 04 -> Low-Pass Filter\n 05 -> Band-Pass Filter\n 06 -> Wah-Wah\n 07 -> Phaser\n 08 -> Vibrato\n 09 -> Echo\n 10 -> Chorus\n 11 -> Overdrive\n 12 -> Fuzz\n 100 -> Change Input Port\n");
        printf("Choose effect: ");
        scanf(" %d", &chooseFX);
        if(chooseFX == 100) {
            int port;
            printf("\nInput ports:\n 00 -> Line Input\n 01 -> Mic Input\n");
            printf("Choose port: ");
            scanf(" %d", &port);
            while( (port!=0) && (port!=1) ) {
                printf("\nInvalid value! choose again: ");
                scanf(" %d", &port);
            }
            printf("\nExecuting setup again...\n");
            setup(port);
        }
        else {
            while ( (chooseFX < 0) || (chooseFX > 12) ) {
                printf("\nInvalid value! choose again: ");
                scanf(" %d", &chooseFX);
            }
        }
        printf("\n");
        switch (chooseFX) {

        case 0:
            printf("Finishing...\n");
            exit = 1;
            break;

        case 1:
            printf("Chosen: Dry Sound (no effect)\n");
            *FX = 1;
            break;

        case 2:
            printf("Chosen: Tremolo\n");
            int trem_p;
            printf("Choose Tremolo period (up to %d samples, recommended: 20000): ", Fs);
            scanf(" %d", &trem_p);
            *TREMOLO_P = trem_p;
            //calculate interpolated array:
            arrayDivider = (float)Fs/(float)*TREMOLO_P;
            printf("\nArray Divider is: %f\n", arrayDivider);
            printf("Calculating tremolo array...\n");
            for(int i=0; i<*TREMOLO_P; i++) {
                //offset = 0.6, amplitude = 0.3
                usedArray[i] = (ONE_16b*0.6) + 0.3*sinArray[(int)floor(i*arrayDivider)];
            }
            printf("Done!\n");
            *FX = 2;
            break;

        case 3:
            printf("Chosen: High-Pass Filter\n");
            filter_coeff_hp_lp(FILTER_ORDER_1PLUS, B, A, 5000, 0.707, shiftLeft, 0, 1); // 1 for HPF
            *FX = 3;
            break;

        case 4:
            printf("Chosen: Low-Pass Filter\n");
            filter_coeff_hp_lp(FILTER_ORDER_1PLUS, B, A, 600, 0.707, shiftLeft, 0, 0); // 0 for LPF
            *FX = 4;
            break;

        case 5:
            printf("Chosen: Band-Pass Filter\n");
            filter_coeff_bp_br(FILTER_ORDER_1PLUS, B, A, 1000, 300, shiftLeft, 0); //BPF
            *FX = 5;
            break;

        case 6:
            printf("Chosen: Wah-Wah\n");
            *FX = 6;
            break;

        case 7:
            printf("Chosen: Phaser\n");
            *FX = 7;
            break;

        case 8:
            printf("Chosen: Vibrato\n");
            //set gains: for VIBRATO: only 1st delayed signal
            g[1] = 0; // g0 = 0;
            g[0] = ONE_16b-1; // g1 = 1;

            //set delays: first, fixed:
            del[1] = 0; // always d0 = 0
            int vibr_p;
            printf("Choose Vibrato period (up to %d samples, recommended: 20000): ", Fs);
            scanf(" %d", &vibr_p);
            *VIBRATO_P = vibr_p;
            //calculate interpolated array:
            arrayDivider = (float)Fs/(float)*VIBRATO_P;
            printf("\nArray Divider is: %f\n", arrayDivider);
            printf("Calculating vibrato array...\n");
            float mult1 = (VIBRATO_LENGTH-1)*0.5;
            for(int i=0; i<*VIBRATO_P; i++) {
                //offset = (FIR_BUFF-1)*0.5, amplitude = (FIR_BUFF-1)*0.5
                usedArray[i] = mult1 + (mult1/ONE_16b)*sinArray[(int)floor(i*arrayDivider)];
            }
            printf("Done!\n");
            *FX = 8;
            break;

        case 9:
            printf("Chosen: Echo\n");
            int echo_length;
            printf("Choose Echo length (up to %d samples): ", (int)Fs/2);
            scanf(" %d", &echo_length);
            *ECHO_LENGTH = echo_length;
            //set gains: for comb delay:
            g[1] = ONE_16b;       // g0 = 1
            g[0] = ONE_16b * 0.5; // g1 = 0.7
            //set delays:
            del[1] = 0; // always d0 = 0
            del[0] = *ECHO_LENGTH - 1; // d1 = as long as delay buffer
            *FX = 9;
            break;

        case 10:
            printf("Chosen: Chorus\n");
            *FX = 10;
            break;

        case 11:
            printf("Chosen: Overdrive\n");
            *FX = 11;
            break;

        case 12:
            printf("Chosen: Fuzz\n");
            *FX = 12;
            break;

        case 100:
            *FX = 0;
            break;

        }

        if(exit == 0) {
            //loop through function until keys 2,3 are touched

            if(*FX == 1) {
                //DRY AUDIO
                while(*keyReg != 3) {
                    getInputBufferSPM(&x[0], &x[1]);
                    setOutputBuffer(x[0], x[1]);
                }
            }

            if(*FX == 2) {
                //TREMOLO
                *mod_pnt = 0;
                while(*keyReg != 3) {
                    //update pointer
                    *mod_pnt = (*mod_pnt + 1) % *TREMOLO_P;
                    //get input
                    getInputBufferSPM(&x[0], &x[1]);
                    //modulate amplitude and set output
                    y[0] = (x[0] * usedArray[*mod_pnt]) >> 15;
                    y[1] = (x[1] * usedArray[*mod_pnt]) >> 15;
                    setOutputBuffer(y[0], y[1]);
                }
            }

            if( (*FX == 3) || (*FX == 4) || (*FX == 5) ) {
                //HIGH-PASS / LOW-PASS / BAND-PASS FILTERS
                //first, fill filter buffer
                for(*pnt=0; *pnt<(FILTER_ORDER_1PLUS-1); *pnt++) {
                    getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
                }
                while(*keyReg != 3) {
                    //increment pointer
                    *pnt = (*pnt+1) % FILTER_ORDER_1PLUS;
                    //first, read last sample
                    getInputBufferSPM(&x_filter[*pnt][0], &x_filter[*pnt][1]);
                    //then, calculate filter
                    filterIIR_2nd(*pnt, x_filter, y_filter, accum, B, A, *shiftLeft);
                    //set output
                    if(*FX == 5) {
                        //for BPF: subtract filtered from input
                        outputReg[0] = ( x_filter[*pnt][0] - y_filter[*pnt][0] ) >> 1;
                        outputReg[1] = ( x_filter[*pnt][1] - y_filter[*pnt][1] ) >> 1;
                        setOutputBuffer((short)outputReg[0], (short)outputReg[1]);
                    }
                    else {
                        setOutputBuffer(y_filter[*pnt][0], y_filter[*pnt][1]);
                    }
                }
            }

            if( (*FX == 6) || (*FX == 7) ) {
                //WAH-WAH or PHASER
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
                }
            }

            if(*FX == 8) {
                //VIBRATO
                *pnt = VIBRATO_LENGTH - 1; //start on top
                *mod_pnt = 0;
                while(*keyReg != 3) {
                    //update delay
                    del[0] = usedArray[*mod_pnt];
                    *mod_pnt = (*mod_pnt + 1) % *VIBRATO_P;
                    //first, read sample
                    getInputBuffer(&audio_buffer[*pnt][0], &audio_buffer[*pnt][1]);
                    /*
                    //calculate FIR comb filter
                    combFilter_1st(VIBRATO_LENGTH, pnt, audio_buffer, y, accum, g, del);
                    */
                    //simple vibrato:
                    y[0] = audio_buffer[(*pnt+del[0])%VIBRATO_LENGTH][0];
                    y[1] = audio_buffer[(*pnt+del[0])%VIBRATO_LENGTH][1];
                    //output sample
                    setOutputBuffer(y[0], y[1]);
                    //update pointer
                    if(*pnt == 0) {
                        *pnt = VIBRATO_LENGTH - 1;
                    }
                    else {
                        *pnt = *pnt - 1;
                    }
                }
            }

            if(*FX == 9) {
                //ECHO
                *pnt = *ECHO_LENGTH - 1; // start on top
                while(*keyReg != 3) {
                    //first, read sample
                    getInputBuffer(&audio_buffer[*pnt][0], &audio_buffer[*pnt][1]);
                    //calculate IIR comb filter
                    combFilter_1st(*ECHO_LENGTH, pnt, audio_buffer, y, accum, g, del);
                    //output sample
                    setOutputBuffer(y[0], y[1]);
                    //replace content on buffer
                    audio_buffer[*pnt][0] = y[0];
                    audio_buffer[*pnt][1] = y[1];
                    //update pointer
                    if(*pnt == 0) {
                        *pnt = *ECHO_LENGTH - 1;
                    }
                    else {
                        *pnt = *pnt - 1;
                    }
                }
            }

            if(*FX == 10) {
                //CHORUS
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
                }
            }

            if(*FX == 11) {
                //OVERDRIVE
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
                }
            }

            if(*FX == 12) {
                //FUZZ
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
                }
            }
        }
    }


    return 0;
}

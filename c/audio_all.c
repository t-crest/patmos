#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ONE_16b 0x7FFF

#define BUFFER_SIZE 128
#define FILTER_ORDER_1PLUS 3

#define Fs 52083 // Hz

#include "audio.h"
#include "audio.c"


// LOCATION IN SCRATCHPAD MEMORY
#define FX_ADDR     0x00000000
#define ACCUM_ADDR  ( FX_ADDR     + sizeof(int) )
//for simple input/output:
#define X_ADDR      ( ACCUM_ADDR  + 2 * sizeof(int) )
#define Y_ADDR      ( X_ADDR      + 2 * sizeof(short) )
//for filter buffers:
#define B_ADDR      ( Y_ADDR      + 2 * sizeof(short) )
#if ( (FILTER_ORDER_1PLUS % 2) == 0 ) //if it's even
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#define X_FILT_ADDR ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) )
#else //if it's odd, align with 4-byte word:
#define A_ADDR      ( B_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#define X_FILT_ADDR ( A_ADDR      + FILTER_ORDER_1PLUS * sizeof(short) + 2 )
#endif
#define Y_FILT_ADDR ( X_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define OUTREG_ADDR ( Y_FILT_ADDR + 2 * FILTER_ORDER_1PLUS * sizeof(short) )
#define PNT_ADDR    ( OUTREG_ADDR + 2 * sizeof(int) )
#define SFTLFT_ADDR ( PNT_ADDR    + sizeof(int) )

//to have fixed-point multiplication:
volatile _SPM int *FX              = (volatile _SPM int *)        FX_ADDR;
volatile _SPM int *accum           = (volatile _SPM int *)        ACCUM_ADDR;

volatile _SPM short *x             = (volatile _SPM short *)      X_ADDR; //input audio
volatile _SPM short *y             = (volatile _SPM short *)      Y_ADDR; //output audio

volatile _SPM short *B             = (volatile _SPM short *)      B_ADDR; // [b2, b1, b0]
volatile _SPM short *A             = (volatile _SPM short *)      A_ADDR; // [a2, a1,  1]
volatile _SPM short (*x_filter)[2] = (volatile _SPM short (*)[2]) X_FILT_ADDR; // x_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM short (*y_filter)[2] = (volatile _SPM short (*)[2]) Y_FILT_ADDR; // y_filter[FILTER_ORDER_1PLUS][2] = {0};
volatile _SPM int *outputReg       = (volatile _SPM int *)        OUTREG_ADDR; //needed for BPF

volatile _SPM int *pnt             = (volatile _SPM int *)        PNT_ADDR; // pointer indicates last position of x_filter buffer
volatile _SPM int *shiftLeft       = (volatile _SPM int *)        SFTLFT_ADDR; //shift left amount;



int main() {

    int exit = 0;


    setup(0); //for volca

    // enable input and output
    *audioDacEnReg = 1;
    *audioAdcEnReg = 1;

    setInputBufferSize(BUFFER_SIZE);
    setOutputBufferSize(BUFFER_SIZE);

    while(exit == 0) {
        int chooseFX;
        printf("Effects list:\n 00 -> Exit\n 01 -> Dry Sound (no effect)\n 02 -> Tremolo\n 03 -> High-Pass Filter\n 04 -> Low-Pass Filter\n 05 -> Band-Pass Filter\n 06 -> Wah-Wah\n 07 -> Phaser\n 08 -> Vibrato\n 09 -> Echo\n 10 -> Chorus\n 11 -> Overdrive\n 12 -> Fuzz\n");
        printf("Choose effect: ");
        scanf(" %d", &chooseFX);
        while ( (chooseFX < 0) || (chooseFX > 12) ) {
            printf("\nInvalid value! choose again: ");
            scanf(" %d", &chooseFX);
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
            *FX = 8;
            break;
        case 9:
            printf("Chosen: Echo\n");
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
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
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
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
                }
            }
            if(*FX == 9) {
                //ECHO
                while(*keyReg != 3) {
                    printf("Effect not implemented yet\n");
                    break;
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

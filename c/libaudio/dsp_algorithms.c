#include "audio.h"
#include "dsp_algorithms.h"

//__attribute__((always_inline))
int filterIIR_1st(int pnt_i, _SPM short (*x)[2], _SPM short (*y)[2], _SPM int *accum, _SPM short *B, _SPM short *A, int shiftLeft) {
    int pnt; //pointer for x_filter
    accum[0] = 0;
    accum[1] = 0;
    for(int i=0; i<2; i++) { //FILTER_ORDER_1PLUS = 2
        pnt = (pnt_i + i + 1) % 2; //FILTER_ORDER_1PLUS = 2
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
    y[pnt_i][0] = accum[0] >> (13-shiftLeft);
    y[pnt_i][1] = accum[1] >> (13-shiftLeft);

    return 0;
}

//__attribute__((always_inline))
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

/*
__attribute__((always_inline))
int filterIIR_2nd_32(int pnt_i, volatile _SPM short (*x)[2], volatile _SPM short (*y)[2], volatile _SPM long long int *accum, volatile _SPM int *B, volatile _SPM int *A, int shiftLeft) {
    int pnt; //pointer for x_filter
    accum[0] = 0;
    accum[1] = 0;
    for(int i=0; i<3; i++) { //FILTER_ORDER_1PLUS = 3
        pnt = (pnt_i + i + 1) % 3; //FILTER_ORDER_1PLUS = 3
        accum[0] += ((long long int)B[i]*x[pnt][0]);
        accum[0] -= ((long long int)A[i]*y[pnt][0]);
        accum[1] += ((long long int)B[i]*x[pnt][1]);
        accum[1] -= ((long long int)A[i]*y[pnt][1]);
    }
    y[pnt_i][0] = (short)(accum[0] >> (31-shiftLeft));
    y[pnt_i][1] = (short)(accum[1] >> (31-shiftLeft));

    return 0;
}
*/

int storeSinInterpol(int *sinArray, short *fracArray, int SIZE, int OFFSET, int AMP) {
    //printf("Storing sin array and frac array...\n");
    float zeiger;
    for(int i=0; i<SIZE; i++) {
        zeiger = (float)OFFSET + ((float)AMP)*sin(2.0*M_PI* i / SIZE);
        sinArray[i] = (int)floor(zeiger);
        fracArray[i] = (zeiger-(float)sinArray[i])*(pow(2,15)-1);
    }
    //printf("Sin array and frac array storage done\n");

    return 0;
}

int storeSin(int *sinArray, int SIZE, int OFFSET, int AMP) {
    //printf("Storing sin array...\n");
    for(int i=0; i<SIZE; i++) {
        sinArray[i] = OFFSET + AMP*sin(2.0*M_PI* i / SIZE);
    }
    /*
    //FIX THIS:
    const int HALF  = (int)ceil(SIZE/2);
    const int ADDER = SIZE%2;
    //first half:
    for(int i=0; i<HALF; i++) {
        sinArray[i] = OFFSET + AMP*sin(2.0*M_PI* i / SIZE);
    }
    //2nd half: odd simmetry
    for(int i=0; i<HALF+ADDER; i++) {
        sinArray[HALF + i] = -sinArray[HALF - (i-ADDER)];
    }
    */
    //printf("Sin array storage done\n");

    return 0;
}

/*
int storeSinLong(long long int *sinArray, long long int SIZE, long long int OFFSET, long long int AMP) {
    printf("Storing LONG sin array...\n");
    for(long long int i=0; i<SIZE; i++) {
        sinArray[i] = OFFSET + AMP*sin(2.0*M_PI* i / SIZE);
    }
    printf("LONG Sin array storage done\n");

    return 0;
}
*/

int checkRanges(int FILT_ORD_1PL, float *Bfl, float *Afl, _SPM int *shiftLeft, int fixedShift) {
    if(!fixedShift) {
        *shiftLeft = 0;
    }
    //check for overflow if coefficients
    float maxVal = 0;
    for(int i=0; i<FILT_ORD_1PL; i++) {
        if( (fabs(Bfl[i]) > 1) && (fabs(Bfl[i]) > maxVal) ) {
            maxVal = fabs(Bfl[i]);
        }
        if( (fabs(Afl[i]) > 1) && (fabs(Afl[i]) > maxVal) ) {
            maxVal = fabs(Afl[i]);
        }
    }
    //if coefficients were too high, scale down
    if(maxVal > 1) {
        if (fixedShift == 1) { // if shiftLeft was fixed
            if (*shiftLeft > 0) {
                maxVal = maxVal / (*shiftLeft * 2) ; //similar to shifting
            }
            if (maxVal > 1) { // if its still out of range
                //printf("ERROR! coefficients are out of range, max is %f\n", maxVal);
                return 1;
            }
        }
        if(!fixedShift) {
            //printf("Greatest coefficient found is %f, ", maxVal);
        }
    }
    while(maxVal > 1) { //loop until maxVal < 1
        *shiftLeft = *shiftLeft + 1; //here we shift right, but the IIR result will be shifted left
        maxVal--;
    }
    if(!fixedShift) {
        //printf("shift left amount is %d\n", *shiftLeft);
    }

    return 0;
}


int filter_coeff_bp_br(int FILT_ORD_1PL, _SPM short *B, _SPM short *A, int Fc, int Fb,_SPM int *shiftLeft, int fixedShift) {
    // if FILTER ORDER = 1, Fb is ignored
    float c, d;
    float Bfl[FILT_ORD_1PL];
    float Afl[FILT_ORD_1PL];
    //init to 0
    for(int i=0; i< FILT_ORD_1PL; i++) {
        Bfl[i] = 0;
        Afl[i] = 0;
    }
    if(FILT_ORD_1PL == 2) { //1st order
        if(!fixedShift) {
            //printf("Calculating 1st order coefficients...\n");
        }
        c = ( tan(M_PI * Fc / Fs) - 1) / ( tan(M_PI * Fc / Fs) + 1 );
        Bfl[1] = c; // b0
        Bfl[0] = 1; // b1
        Afl[0] = c; // a1
    }
    else {
        if(FILT_ORD_1PL == 3) { // 2nd order
            if(!fixedShift) {
                //printf("Calculating 2nd order coefficients...\n");
            }
            c = ( tan(M_PI * Fb / Fs) - 1) / ( tan(M_PI * Fb / Fs) + 1 );
            d = -1 * cos(2 * M_PI * Fc / Fs);
            Bfl[2] = -1 * c; // b0
            Bfl[1] = d * (1 - c); // b1
            Bfl[0] = 1; // b2
            Afl[1] = d * (1 - c); // a1
            Afl[0] = -1 * c; // a2
        }
    }
    // check ranges and set leftShift amount
    int notRangesOK = checkRanges(FILT_ORD_1PL, Bfl, Afl, shiftLeft, fixedShift);
    if (notRangesOK == 1) {
        return 1;
    }
    // now all coefficients should be between 0 and 1
    for(int i=0; i<FILT_ORD_1PL; i++) {
        B[i] = (short) ( (int) (ONE_16b * Bfl[i]) >> *shiftLeft );
        A[i] = (short) ( (int) (ONE_16b * Afl[i]) >> *shiftLeft );
    }
    if(!fixedShift) {
        if(FILT_ORD_1PL == 2) {
            //printf("done! c: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", c, B[1], B[0], A[1], A[0]);
        }
        if(FILT_ORD_1PL == 3) {
            //printf("done! c: %f, d: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", c, d, B[2], B[1], B[0], A[2], A[1], A[0]);
        }
    }

    return 0;
}
/*
int filter_coeff_bp_br_32(int FILT_ORD_1PL, volatile _SPM int *B, volatile _SPM int *A, int Fc, int Fb, volatile _SPM int *shiftLeft, int fixedShift) {
    // if FILTER ORDER = 1, Fb is ignored
    float c, d;
    float Bfl[FILT_ORD_1PL];
    float Afl[FILT_ORD_1PL];
    //init to 0
    for(int i=0; i< FILT_ORD_1PL; i++) {
        Bfl[i] = 0;
        Afl[i] = 0;
    }
    if(FILT_ORD_1PL == 2) { //1st order
        if(!fixedShift) {
            printf("Calculating 1st order coefficients...\n");
        }
        c = ( tan(M_PI * Fc / Fs) - 1) / ( tan(M_PI * Fc / Fs) + 1 );
        Bfl[1] = c; // b0
        Bfl[0] = 1; // b1
        Afl[0] = c; // a1
    }
    else {
        if(FILT_ORD_1PL == 3) { // 2nd order
            if(!fixedShift) {
                printf("Calculating 2nd order coefficients...\n");
            }
            c = ( tan(M_PI * Fb / Fs) - 1) / ( tan(M_PI * Fb / Fs) + 1 );
            d = -1 * cos(2 * M_PI * Fc / Fs);
            Bfl[2] = -1 * c; // b0
            Bfl[1] = d * (1 - c); // b1
            Bfl[0] = 1; // b2
            Afl[1] = d * (1 - c); // a1
            Afl[0] = -1 * c; // a2
        }
    }
    // check ranges and set leftShift amount
    int notRangesOK = checkRanges(FILT_ORD_1PL, Bfl, Afl, shiftLeft, fixedShift);
    if (notRangesOK == 1) {
        return 1;
    }
    // now all coefficients should be between 0 and 1
    for(int i=0; i<FILT_ORD_1PL; i++) {
        B[i] = (int) ( (long long int) (ONE_32b * Bfl[i]) >> *shiftLeft );
        A[i] = (int) ( (long long int) (ONE_32b * Afl[i]) >> *shiftLeft );
    }
    if(!fixedShift) {
        if(FILT_ORD_1PL == 2) {
            printf("done! c: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", c, B[1], B[0], A[1], A[0]);
        }
        if(FILT_ORD_1PL == 3) {
            printf("done! c: %f, d: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", c, d, B[2], B[1], B[0], A[2], A[1], A[0]);
        }
    }

    return 0;
}
*/

int filter_coeff_hp_lp(int FILT_ORD_1PL, _SPM short *B, _SPM short *A, int Fc, float Q, _SPM int *shiftLeft, int fixedShift, int type) {
    float K = tan(M_PI * Fc / Fs);// K is same for all
    float Bfl[FILT_ORD_1PL];
    float Afl[FILT_ORD_1PL];
    //init to 0
    for(int i=0; i< FILT_ORD_1PL; i++) {
        Bfl[i] = 0;
        Afl[i] = 0;
    }
    float common_factor;
    if(type == 0) { //LPF
        if(FILT_ORD_1PL == 2) { //1st order
            //printf("Calculating LPF for 1st order...\n");
            Bfl[1] = K/(K+1); //b0
            Bfl[0] = K/(K+1); //b1
            Afl[0] = (K-1)/(K+1); //a1
        }
        else {
            if(FILT_ORD_1PL == 3) { //2nd order
                //printf("Calculating LPF for 2nd order...\n");
                common_factor = 1/(pow(K,2)*Q + K + Q);
                Bfl[2] = pow(K,2)*Q*common_factor; //b0
                Bfl[1] = 2*pow(K,2)*Q*common_factor; //b1
                Bfl[0] = pow(K,2)*Q*common_factor; //b2
                Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
                Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
            }
        }
    }
    else {
        if(type == 1) { // HPF
            if(FILT_ORD_1PL == 2) { //1st order
                //printf("Calculating HPF for 1st order...\n");
                Bfl[1] =  1/(K+1); //b0
                Bfl[0] = -1/(K+1); //b1
                Afl[0] = (K-1)/(K+1); //a1
            }
            else {
                if(FILT_ORD_1PL == 3) { //2nd order
                    //printf("Calculating HPF for 2nd order...\n");
                    common_factor = 1/(pow(K,2)*Q + K + Q);
                    Bfl[2] = Q*common_factor; //b0
                    Bfl[1] = -2*Q*common_factor; //b1
                    Bfl[0] = Q*common_factor; //b2
                    Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
                    Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
                }
            }
        }
    }
    // check ranges and set leftShift amount
    int notRangesOK = checkRanges(FILT_ORD_1PL, Bfl, Afl, shiftLeft, fixedShift);
    if (notRangesOK == 1) {
        return 1;
    }
    // now all coefficients should be between 0 and 1
    for(int i=0; i<FILT_ORD_1PL; i++) {
        B[i] = (short) ( (int) (ONE_16b * Bfl[i]) >> *shiftLeft );
        A[i] = (short) ( (int) (ONE_16b * Afl[i]) >> *shiftLeft );
    }
    if(FILT_ORD_1PL == 2) {
        //printf("done! K: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", K, B[1], B[0], A[1], A[0]);
    }
    if(FILT_ORD_1PL == 3) {
        //printf("done! K: %f, common_factor: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", K, common_factor, B[2], B[1], B[0], A[2], A[1], A[0]);
    }

    return 0;
}

/*
int filter_coeff_hp_lp_32(int FILT_ORD_1PL, volatile _SPM int *B, volatile _SPM int *A, int Fc, float Q, volatile _SPM int *shiftLeft, int fixedShift, int type) {
    float K = tan(M_PI * Fc / Fs);// K is same for all
    float Bfl[FILT_ORD_1PL];
    float Afl[FILT_ORD_1PL];
    //init to 0
    for(int i=0; i< FILT_ORD_1PL; i++) {
        Bfl[i] = 0;
        Afl[i] = 0;
    }
    float common_factor;
    if(type == 0) { //LPF
        if(FILT_ORD_1PL == 2) { //1st order
            printf("Calculating LPF for 1st order...\n");
            Bfl[1] = K/(K+1); //b0
            Bfl[0] = K/(K+1); //b1
            Afl[0] = (K-1)/(K+1); //a1
        }
        else {
            if(FILT_ORD_1PL == 3) { //2nd order
                printf("Calculating LPF for 2nd order...\n");
                common_factor = 1/(pow(K,2)*Q + K + Q);
                Bfl[2] = pow(K,2)*Q*common_factor; //b0
                Bfl[1] = 2*pow(K,2)*Q*common_factor; //b1
                Bfl[0] = pow(K,2)*Q*common_factor; //b2
                Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
                Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
            }
        }
    }
    else {
        if(type == 1) { // HPF
            if(FILT_ORD_1PL == 2) { //1st order
                printf("Calculating HPF for 1st order...\n");
                Bfl[1] =  1/(K+1); //b0
                Bfl[0] = -1/(K+1); //b1
                Afl[0] = (K-1)/(K+1); //a1
            }
            else {
                if(FILT_ORD_1PL == 3) { //2nd order
                    printf("Calculating HPF for 2nd order...\n");
                    common_factor = 1/(pow(K,2)*Q + K + Q);
                    Bfl[2] = Q*common_factor; //b0
                    Bfl[1] = -2*Q*common_factor; //b1
                    Bfl[0] = Q*common_factor; //b2
                    Afl[1] = 2*Q*(pow(K,2)-1)*common_factor; //a1
                    Afl[0] = (pow(K,2)*Q - K + Q)*common_factor; //a2
                }
            }
        }
    }
    // check ranges and set leftShift amount
    int notRangesOK = checkRanges(FILT_ORD_1PL, Bfl, Afl, shiftLeft, fixedShift);
    if (notRangesOK == 1) {
        return 1;
    }
    // now all coefficients should be between 0 and 1
    for(int i=0; i<FILT_ORD_1PL; i++) {
        B[i] = (int) ( (long long int) (ONE_32b * Bfl[i]) >> *shiftLeft );
        A[i] = (int) ( (long long int) (ONE_32b * Afl[i]) >> *shiftLeft );
    }
    if(FILT_ORD_1PL == 2) {
        printf("done! K: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", K, B[1], B[0], A[1], A[0]);
    }
    if(FILT_ORD_1PL == 3) {
        printf("done! K: %f, common_factor: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", K, common_factor, B[2], B[1], B[0], A[2], A[1], A[0]);
    }

    return 0;
}
*/

__attribute__((always_inline))
int allpass_comb(int AP_BUF_LEN, _SPM int *pnt, short (*ap_buffer)[AP_BUF_LEN], volatile _SPM short *x, volatile _SPM short *y, _SPM short *g) {
    int accum[2];
    int ap_pnt = (*pnt + AP_BUF_LEN - 1) % AP_BUF_LEN;
    for(int i=0; i<2; i++) {
        y[i] = ap_buffer[i][ap_pnt] - ( ((int)x[i]*(*g)) >> 15 );
        accum[i] = ( (y[i]*(*g)) >> 15 ) + x[i];
        //check for overflow
        if(accum[i] > ONE_16b) {
            accum[i] = ONE_16b;
        }
        else {
            if(accum[i] < -ONE_16b) {
                accum[i] = -ONE_16b;
            }
        }
        ap_buffer[i][*pnt] = accum[i];
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

/*
//from 1/2! to 1/8!, represented as Q.15
const short MCLAURIN_FACTOR[7] = { 0x4000, 0x1555, 0x555, 0x111, 0x2d, 0x6, 0x1};

__attribute__((always_inline))
int distortion(volatile _SPM short *x, volatile _SPM short *y, _SPM int *accum) {
    int neg;
    if(x[0] > 0) {
        for(int j=0; j<2; j++) {
            x[j] = -x[j];
        }
        neg = 0;
    }
    else {
        neg = 1;
    }
    // calculate maclaurin series
    short x_pow[2];
    for(int j=0; j<2; j++) {
        //before: initialise to first value:
        x_pow[j] = x[j];
        accum[j] = -x[j];
        //after that, rest of values:
        for(int i=0; i<5; i++) { // 5 for MacLaurin order 6
            //increase power, divide by factor
            x_pow[j] = (x_pow[j] * x[j]) >> 15;
            accum[j] -= (x_pow[j] * MCLAURIN_FACTOR[i]) >> 15;
        }
        //set output
        if (neg == 1) { //negative sign
            y[j] = -accum[j];
        }
        else { //positive sign
            y[j] = accum[j];
        }
    }

    return 0;
}
*/

/*
__attribute__((always_inline))
int fuzz(volatile _SPM short *x, volatile _SPM short *y, volatile _SPM int *accum, const int K, const int KonePlus, const int shiftLeft) {
    int accum1;
    int accum2;
    for(int j=0; j<2; j++) {
        accum[0] = (KonePlus * x[j]);// >> 15;
        accum[1] = (K * abs(x[j])) >> 15;
        accum[1] = accum[1] + ((ONE_16b+shiftLeft) >> shiftLeft);
        accum[0] = accum[0]/accum[1];
        //reduce if it is poisitive only
        if (x[j] > 0) {
            y[j] = accum[0] - 1;
        }
        else {
            y[j] = accum[0];
        }
    }

    return 0;
}
*/

/*
__attribute__((always_inline))
int overdrive(volatile _SPM short *x, volatile _SPM short *y, volatile _SPM int *accum) {
    //THRESHOLD IS 1/3 = 0x2AAB
    //input abs:
    unsigned int x_abs[2];
    for(int j=0; j<2; j++) {
        x_abs[j] = abs(x[j]);
        if(x_abs[j] > (2 * 0x2AAB)) { // saturation : y = 1
            if (x[j] > 0) {
                y[j] = 0x7FFF;
            }
            else {
                y[j] = 0x8000;
            }
        }
        else {
            if(x_abs[j] > 0x2AAB) { // smooth overdrive: y = ( 3 - (2-3*x)^2 ) / 3;
                accum[j] = (0x17FFF * x_abs[j]) >> 15 ; // result is 1 sign + 17 bits
                accum[j] = 0xFFFF - accum[j];
                accum[j] = (accum[j] * accum[j]) >> 15;
                accum[j] = 0x17FFF - accum[j];
                accum[j] = (accum[j] * 0x2AAB) >> 15;
                if(x[j] > 0) { //positive
                    if(accum[j] > 32767) {
                        y[j] = 32767;
                    }
                    else {
                        y[j] = accum[j];
                    }
                }
                else { // negative
                    y[j] = -accum[j];
                }
            }
            else { // linear zone: y = 2*x
                y[j] = x[j] << 1;
            }
        }
    }

    return 0;
}
*/

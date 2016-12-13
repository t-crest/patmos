
#include "libaudio/audio.h"

/*
 * @file		Audio_setup.c
 * @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
 * @brief	Setting up all the registers in the audio interface
 */

/*
 * @brief		Writes the supplied data to the address register,
 sets the request signal and waits for the acknowledge signal.
 * @param[in]	addr	the address of which register to write to.
 Has to be 7 bit long.
 * @param[in]	data	the data thats supposed to be written.
 Has to be 9 Bits long
 * @reutrn		returns 0 if successful and a negative number if there was an error.
 */
int writeToI2C(char* addrC,char* dataC) {
  int addr = 0;
  int data = 0;

  //Convert binary String of address to int
  for(int i = 0; i < 7; i++) {
    addr *= 2;
    if (*addrC++ == '1') addr += 1;
  }

  //Convert binary String of data to int
  for(int i = 0; i < 9; i++) {
    data *= 2;
    if (*dataC++ == '1') data += 1;
  }

  //Debug info:
  printf("Sending Data: %i to address %i\n",data,addr);

  *i2cDataReg = data;
  *i2cAdrReg	= addr;
  *i2cReqReg	= 1;


  while(*i2cAckReg == 0) {
    printf("Waiting ...\n");
    //Maybe input something like a timeout ...
  }
  for (int i = 0; i<200; i++)  { *i2cReqReg=0; }

  printf("success\n");

  return 0;



}

/*
 * @brief	Sets the default values
 */

void setup(int guitar) {

  /*
  //----------Line in---------------------
  char *addrLeftIn  = "0000000";
  char *dataLineIn  = "100010111";	//disable Mute, Enable Simultaneous Load, LinVol: 10111 - Set volume to 23 (of 31)
  writeToI2C(addrLeftIn,dataLineIn);

  char *addrRigthIn = "0000001";
  writeToI2C(addrRigthIn,dataLineIn);
  */

  //----------Headphones---------------------
  char *addrLeftHead  = "0000010";
  char *dataHeadphone = "001111001";	// disable simultaneous loads, zero cross disable, LinVol: 1111001 (0db)
  writeToI2C(addrLeftHead,dataHeadphone);

  char *addrRightHead = "0000011";
  writeToI2C(addrRightHead,dataHeadphone);

  //--------Analogue Audio Path Control-----
  char *addrAnalogue  = "0000100";
  if(guitar == 0) {
      char *dataAnalogue = "000010010"; //DAC selected, rest disabled, MIC muted, Line input select to ADC
      writeToI2C(addrAnalogue,dataAnalogue);
  }
  else {
      char *dataAnalogueGuit = "000010101"; //MIC selected to ADC, MIC enabled, MIC boost enabled
      writeToI2C(addrAnalogue,dataAnalogueGuit);
  }


  //--------Digital Audio Path Control-----
  char *addrDigital  = "0000101";
  char *dataDigital  = "000000001";	//disable soft mute and disable de-emphasis, disable highpass filter
  writeToI2C(addrDigital,dataDigital);


  //---------Digital Audio Interface Format---------
  char *addrInterface  = "0000111";
  char *dataInterface = "000010011"; //BCLK not inverted, slave, right-channel-right-data, LRP = A mode for DSP, 16-bit audio, DSP mode
  writeToI2C(addrInterface,dataInterface);


  //--------Sampling Control----------------
  char *addrSample  = "0001000";
  char *dataSample  = "000000000"; //USB mode, BOSR=1, Sample Rate = 44.1 kHz both for ADC and DAC
  writeToI2C(addrSample,dataSample);

  printf("FINISHED SETUP!\n");
}


/*
 * @brief		changes the volume of the audio output.
 * @param[in]	vol 	in db. Has to be between +6 and -73
 * @reutrn		returns 0 if successful and a negative number if there was an error.
 */
int changeVolume(int vol) {
  //127--1111111 = +6dB
  //121--1111001 = 0 dB
  //48--0110000 = -73dB
  if ((vol<-73) || (vol>6)) {
    return -1;	//Invalid input.
  }

  //Since both Left Channel Zero Cross detect and simultaneous Load are disabled this can be send imideatly:
  //LEFT:
  *i2cDataReg = vol + 121;
  *i2cAdrReg	 = 2;
  *i2cReqReg	 = 1;
  while(*i2cAckReg == 0);
  for (int i = 0; i<200; i++)  { *i2cReqReg=0; }

  //RIGHT:
  *i2cDataReg = vol + 121;
  *i2cAdrReg	 = 3;
  *i2cReqReg	 = 1;
  while(*i2cAckReg == 0);
  for (int i = 0; i<200; i++)  { *i2cReqReg=0; }

  return 0;
}


int isPowerOfTwo (unsigned int x) {
 while (((x % 2) == 0) && x > 1) /* While x is even and > 1 */
   x /= 2;
 return (x == 1);
}

/*
 * @brief	sets the size of the input (ADC) buffer. Must be a power of 2
 * @param[in]	bufferSize	length of the buffer
 * @return	returns 0 if successful and a 1 if there was an error.
 */
int setInputBufferSize(int bufferSize) {
  if(isPowerOfTwo(bufferSize)) {
    printf("Input buffer size set to %d\n", bufferSize);
    *audioAdcBufferSizeReg = bufferSize;
    return 0;
  }
  else {
    printf("ERROR: Buffer Size must be power of 2\n");
    return 1;
  }
}

/*
 * @brief	 sets the size of the output (DAC) buffer. Must be a power of 2
 * @param[in]	 bufferSize	length of the buffer
 * @return	 returns 0 if successful and a 1 if there was an error.
 */
int setOutputBufferSize(int bufferSize) {
  if(isPowerOfTwo(bufferSize)) {
    printf("Output buffer size set to %d\n", bufferSize);
    *audioDacBufferSizeReg = bufferSize;
    return 0;
  }
  else {
    printf("ERROR: Buffer Size must be power of 2\n");
    return 1;
  }
}

/*
 * @brief	reads data from the input (ADC) buffer into Patmos
 * @param[in]	*l	pointer to left audio data
 * @param[in]	*r	pointer to right audio data
 * @return	returns 0 if successful and a 1 if there was an error.
 */
int getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r) {
  while(*audioAdcBufferEmptyReg == 1);// wait until not empty
  *audioAdcBufferReadPulseReg = 1; // begin pulse
  *audioAdcBufferReadPulseReg = 0; // end pulse
  *l = *audioAdcLReg;
  *r = *audioAdcRReg;
  return 0;
}

int getInputBuffer(volatile short *l, volatile short *r) {
  while(*audioAdcBufferEmptyReg == 1);// wait until not empty
  *audioAdcBufferReadPulseReg = 1; // begin pulse
  *audioAdcBufferReadPulseReg = 0; // end pulse
  *l = *audioAdcLReg;
  *r = *audioAdcRReg;
  return 0;
}

/*
 * @brief	writes data from patmos into the output (DAC) buffer
 * @param[in]	l	left audio data
 * @param[in]	r	right audio data
 * @return	returns 0 if successful and a 1 if there was an error.
 */
int setOutputBuffer(short l, short r) {
  //write data first: it will stay in AudioInterface, won't go to
  //AudioDacBuffer until the write pulse
  *audioDacLReg = l;
  *audioDacRReg = r;
  while(*audioDacBufferFullReg == 1); // wait until not full
  *audioDacBufferWritePulseReg = 1; // begin pulse
  *audioDacBufferWritePulseReg = 0; // end pulse

  return 0;
}

int setOutputBufferSPM(volatile _SPM short *l, volatile _SPM short *r) {
  //write data first: it will stay in AudioInterface, won't go to
  //AudioDacBuffer until the write pulse
  *audioDacLReg = *l;
  *audioDacRReg = *r;
  while(*audioDacBufferFullReg == 1); // wait until not full
  *audioDacBufferWritePulseReg = 1; // begin pulse
  *audioDacBufferWritePulseReg = 0; // end pulse

  return 0;
}



//-----------------------VARIABLES IN SCRATCHPAD MEMORY------------------------//


//---------------------------------------------------------------//


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
    printf("Storing sin array and frac array...\n");
    float zeiger;
    for(int i=0; i<SIZE; i++) {
        zeiger = (float)OFFSET + ((float)AMP)*sin(2.0*M_PI* i / SIZE);
        sinArray[i] = (int)floor(zeiger);
        fracArray[i] = (zeiger-(float)sinArray[i])*(pow(2,15)-1);
    }
    printf("Sin array and frac array storage done\n");

    return 0;
}

int storeSin(int *sinArray, int SIZE, int OFFSET, int AMP) {
    printf("Storing sin array...\n");
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
    printf("Sin array storage done\n");

    return 0;
}

int storeSinLong(long long int *sinArray, long long int SIZE, long long int OFFSET, long long int AMP) {
    printf("Storing LONG sin array...\n");
    for(long long int i=0; i<SIZE; i++) {
        sinArray[i] = OFFSET + AMP*sin(2.0*M_PI* i / SIZE);
    }
    printf("LONG Sin array storage done\n");

    return 0;
}

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
                printf("ERROR! coefficients are out of range, max is %f\n", maxVal);
                return 1;
            }
        }
        if(!fixedShift) {
            printf("Greatest coefficient found is %f, ", maxVal);
        }
    }
    while(maxVal > 1) { //loop until maxVal < 1
        *shiftLeft = *shiftLeft + 1; //here we shift right, but the IIR result will be shifted left
        maxVal--;
    }
    if(!fixedShift) {
        printf("shift left amount is %d\n", *shiftLeft);
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
        B[i] = (short) ( (int) (ONE_16b * Bfl[i]) >> *shiftLeft );
        A[i] = (short) ( (int) (ONE_16b * Afl[i]) >> *shiftLeft );
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
        B[i] = (short) ( (int) (ONE_16b * Bfl[i]) >> *shiftLeft );
        A[i] = (short) ( (int) (ONE_16b * Afl[i]) >> *shiftLeft );
    }
    if(FILT_ORD_1PL == 2) {
        printf("done! K: %f, b0: %d, b1: %d, a0, %d, a1: %d\n", K, B[1], B[0], A[1], A[0]);
    }
    if(FILT_ORD_1PL == 3) {
        printf("done! K: %f, common_factor: %f, b0: %d, b1: %d, b2 : %d, a0: %d, a1: %d, a2: %d\n", K, common_factor, B[2], B[1], B[0], A[2], A[1], A[0]);
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
int allpass_comb(int AP_BUFF_LEN, _SPM int *pnt, short (*ap_buffer)[2], volatile _SPM short *x, volatile _SPM short *y, _SPM short *g) {
    int accum[2];
    int ap_pnt = (*pnt + AP_BUFF_LEN - 1) % AP_BUFF_LEN;
    for(int i=0; i<2; i++) {
        y[i] = ap_buffer[ap_pnt][i] - ( ((int)x[i]*(*g)) >> 15 );
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
        ap_buffer[*pnt][i] = accum[i];
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

//----------------------------COMPLETE AUDIO FUNCTIONS---------------------------------//

/*            AUDIO EFFECT FUNCTIONS          */


void audioIn(struct AudioFX *audioP, volatile _SPM short *xP) {
    for(unsigned int i=0; i < *audioP->xb_size; i++) {
        getInputBufferSPM(&xP[i*2], &xP[i*2+1]);
    }
}

void audioOut(struct AudioFX *audioP, volatile _SPM short *yP) {
    for(unsigned int i=0; i < *audioP->yb_size; i++) {
        setOutputBufferSPM(&yP[i*2], &yP[i*2+1]);
    }
}

int audio_dry(volatile _SPM short *xP, volatile _SPM short *yP) {
    yP[0] = xP[0];
    yP[1] = xP[1];

    return 0;
}

int audio_dry_8samples(volatile _SPM short *xP, volatile _SPM short *yP) {
    for(int i=0; i<8; i++) {
        yP[i*2]   = xP[i*2];
        yP[i*2+1] = xP[i*2+1];
    }
    return 0;
}

int alloc_space(unsigned int BASE_ADDR, unsigned int LAST_ADDR) {
    int cpuid = get_cpuid();
    unsigned int ALLOC_AMOUNT = LAST_ADDR - BASE_ADDR;
    void _SPM *allocret = mp_alloc(ALLOC_AMOUNT);
    if(allocret == NULL) {
        if(cpuid == 0) {
            printf("ERROR ON SPM ALLOCATION\n");
        }

        return 1;
    }
    else {
        if(cpuid == 0) {
            printf("\n");
            printf("Base position at SPM of core %d is 0x%x\n", cpuid, BASE_ADDR);
            printf("%d bytes allocated in SPM of core %d\n", ALLOC_AMOUNT, cpuid);
            printf("Last free position at SPM of core %d is 0x%x\n", cpuid, LAST_ADDR);
            printf("-------------------DONE!-------------------\n");
        }

        return 0;
    }
}

unsigned int alloc_filter_vars(_SPM struct Filter *filtP, unsigned int LAST_ADDR, int Fc, float QorFb, int thisType) {

    //calculate filter coefficients (3rd order)
    filtP->type = thisType;
    if (filtP->type < 2) { //HP or LP
        filter_coeff_hp_lp(3, filtP->B, filtP->A, Fc, QorFb, &filtP->sftLft, 0, thisType); //type: HPF or LPF
    }
    else { // 2 or 3: BP or BR
        filter_coeff_bp_br(3, filtP->B, filtP->A, Fc, (int)QorFb,  &filtP->sftLft, 0);
    }

    filtP->pnt = 2;

    LAST_ADDR += (sizeof(struct Filter)); //+4 needed??

    return LAST_ADDR;
}

int audio_filter(_SPM struct Filter *filtP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //increment pointer
    filtP->pnt = ( filtP->pnt + 1 ) % 3;
    //first, read sample
    filtP->x_buf[filtP->pnt][0] = xP[0];
    filtP->x_buf[filtP->pnt][1] = xP[1];
    //then, calculate filter
    filterIIR_2nd(&filtP->pnt, filtP->x_buf, filtP->y_buf, filtP->accum, filtP->B, filtP->A, &filtP->sftLft);
    //check if it is BP/BR
    if(filtP->type == 2) { //BP
        filtP->accum[0] = ( (int)xP[0] - (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
        filtP->accum[1] = ( (int)xP[1] - (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
    }
    else {
        if(filtP->type == 3) { //BR
            filtP->accum[0] = ( (int)xP[0] + (int)filtP->y_buf[filtP->pnt][0] ) >> 1;
            filtP->accum[1] = ( (int)xP[1] + (int)filtP->y_buf[filtP->pnt][1] ) >> 1;
        }
        else { //HP or LP
            filtP->accum[0] = filtP->y_buf[filtP->pnt][0];
            filtP->accum[1] = filtP->y_buf[filtP->pnt][1];
        }
    }
    //set output
    yP[0] = (short)filtP->accum[0];
    yP[1] = (short)filtP->accum[1];

    return 0;
}

unsigned int alloc_vibrato_vars(_SPM struct Vibrato *vibrP, unsigned int LAST_ADDR) {

    //modulation arrays
    vibrP->audio_buf_pnt = malloc(VIBRATO_L * 2 * sizeof(short)); // short audio_buf[2][VIBRATO_L]
    vibrP->sin_array_pnt  = malloc(VIBRATO_P * sizeof(int)); // int sin_array[VIBRATO_P]
    vibrP->frac_array_pnt = malloc(VIBRATO_P * sizeof(short)); // short frac_array[VIBRATO_P]

    //modulation storage
    storeSinInterpol(vibrP->sin_array_pnt, vibrP->frac_array_pnt, VIBRATO_P, (VIBRATO_L*0.5), ((VIBRATO_L-1)*0.5));

    //empty buffer
    for(int i=0; i<VIBRATO_L; i++) {
        vibrP->audio_buf_pnt[0][i] = 0;
        vibrP->audio_buf_pnt[1][i] = 0;
    }

    //initialise vibrato variables
    vibrP->pnt = VIBRATO_L - 1; //start on top
    vibrP->v_pnt = 0;

    LAST_ADDR += (sizeof(struct Vibrato));

    return LAST_ADDR;
}


int audio_vibrato(_SPM struct Vibrato *vibrP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //update delay pointers
    vibrP->del = vibrP->sin_array_pnt[vibrP->v_pnt];
    vibrP->frac = vibrP->frac_array_pnt[vibrP->v_pnt];
    short frac1Minus = ONE_16b - vibrP->frac;
    vibrP->v_pnt = ( vibrP->v_pnt + 1 )%VIBRATO_P;
    //vibrato pointers:
    vibrP->audio_pnt   = (vibrP->pnt+vibrP->del)%VIBRATO_L;
    vibrP->n_audio_pnt = (vibrP->pnt+vibrP->del+1)%VIBRATO_L;
    for(int i=0; i<2; i++) { //stereo
        //first, read sample
        vibrP->audio_buf_pnt[i][vibrP->pnt] = xP[i];
        vibrP->accum[i] =  (vibrP->audio_buf_pnt[i][vibrP->n_audio_pnt] * (vibrP->frac));
        vibrP->accum[i] += (vibrP->audio_buf_pnt[i][vibrP->audio_pnt]   * (frac1Minus));
        yP[i] = vibrP->accum[i] >> 15;
    }
    //update input pointer
    if(vibrP->pnt == 0) {
        vibrP->pnt = VIBRATO_L - 1;
    }
    else {
        vibrP->pnt = vibrP->pnt - 1;
    };

    return 0;
}

unsigned int alloc_wahwah_vars(_SPM struct WahWah *wahP, unsigned int LAST_ADDR) {

    //shift left is fixed!
    wahP->sftLft = 1;

    //modulation arrays
    wahP->fc_array = malloc(WAHWAH_P * sizeof(int)); // int fc_array[WAHWAH_P]
    wahP->fb_array = malloc(WAHWAH_P * sizeof(int)); // int fb_array[WAHWAH_P]
    wahP->a_array  = malloc(WAHWAH_P * 3 * sizeof(short)); // short a_array[3][WAHWAH_P]
    wahP->b_array  = malloc(WAHWAH_P * 3 * sizeof(short)); // short b_array[3][WAHWAH_P]

    //store sin Arrays of Fc and Fb
    storeSin(wahP->fc_array, WAHWAH_P, WAHWAH_FC_CEN, WAHWAH_FC_AMP);
    storeSin(wahP->fb_array, WAHWAH_P, WAHWAH_FB_CEN, WAHWAH_FB_AMP);

    //calculate band-pass filter coefficients
    printf("calculating modulation coefficients...\n");
    for(int i=0; i<WAHWAH_P; i++) {
        filter_coeff_bp_br(3, wahP->B, wahP->A, wahP->fc_array[i], wahP->fb_array[i], &wahP->sftLft, 1);
        wahP->b_array[2][i] = wahP->B[2];
        wahP->b_array[1][i] = wahP->B[1];
        wahP->b_array[0][i] = wahP->B[0];
        wahP->a_array[1][i] = wahP->A[1];
        wahP->a_array[0][i] = wahP->A[0];
    }
    printf("calculation of modulation coefficients finished!\n");

    wahP->wah_pnt = 2;

    LAST_ADDR += (sizeof(struct WahWah));

    return LAST_ADDR;
}

int audio_wahwah(_SPM struct WahWah *wahP, volatile _SPM short *xP, volatile _SPM short *yP) {
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
    wahP->x_buf[wahP->pnt][0] = xP[0];
    wahP->x_buf[wahP->pnt][1] = xP[1];
    //then, calculate filter
    filterIIR_2nd(&wahP->pnt, wahP->x_buf, wahP->y_buf, wahP->accum, wahP->B, wahP->A, &wahP->sftLft);
    //Band-Pass stuff
    wahP->accum[0] = ( (int)xP[0] - (int)wahP->y_buf[wahP->pnt][0] ); // >> 1;
    wahP->accum[1] = ( (int)xP[1] - (int)wahP->y_buf[wahP->pnt][1] ); // >> 1;
    //mix with original: gains are fixed
    wahP->accum[0] = ( (int)(WAHWAH_WET_GAIN*wahP->accum[0]) >> 15 )  + ( (int)(WAHWAH_DRY_GAIN*xP[0]) >> 15 );
    wahP->accum[1] = ( (int)(WAHWAH_WET_GAIN*wahP->accum[1]) >> 15 )  + ( (int)(WAHWAH_DRY_GAIN*xP[1]) >> 15 );
    //set output
    yP[0] = (short)wahP->accum[0];
    yP[1] = (short)wahP->accum[1];

    return 0;
}

unsigned int alloc_tremolo_vars(_SPM struct Tremolo *tremP, unsigned int LAST_ADDR) {

    //modulation arrays
    tremP->mod_array  = malloc(TREMOLO_P * sizeof(int)); // int mod_array[TREMOLO_P]
    tremP->frac_array = malloc(TREMOLO_P * sizeof(short)); // short frac_array[TREMOLO_P]

    //initialise modulation array
    storeSinInterpol(tremP->mod_array, tremP->frac_array, TREMOLO_P, (ONE_16b*0.6), (ONE_16b*0.3));

     //pointers:
    tremP->pnt = 0;
    tremP->pnt_n = 1;

    LAST_ADDR += (sizeof(struct Tremolo));

    return LAST_ADDR;
}

int audio_tremolo(_SPM struct Tremolo *tremP, volatile _SPM short *xP, volatile _SPM short *yP) {
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
    yP[0] = (xP[0] * tremP->mod) >> 15;
    yP[1] = (xP[1] * tremP->mod) >> 15;

    return 0;
}

unsigned int alloc_chorus_vars(_SPM struct Chorus *chorP, unsigned int LAST_ADDR) {

    //initialise chorus variables
    //set gains:
    chorP->g[2] = ONE_16b * 0.45; //g0
    chorP->g[1] = ONE_16b * 0.4;  //g1
    chorP->g[0] = ONE_16b * 0.4;  //g2
    //set delays:
    chorP->del[2] = 0; // always d0 = 0

    //modulation arrays
    chorP->audio_buf  = malloc(CHORUS_L * 2 * sizeof(short)); // short audio_buf[2][CHORUS_L]
    chorP->mod_array1 = malloc(CHORUS_P1 * sizeof(int)); // int mod_array1[CHORUS_P1]
    chorP->mod_array2 = malloc(CHORUS_P2 * sizeof(int)); // int mod_array2[CHORUS_P2]

    //calculate modulation arrays
    storeSin(chorP->mod_array1, CHORUS_P1, ( CHORUS_L*0.6 ), ( CHORUS_L * 0.02) );
    storeSin(chorP->mod_array2, CHORUS_P2, ( CHORUS_L*0.4 ), ( CHORUS_L * 0.012) );

    //empty buffer
    for(int i=0; i<CHORUS_L; i++) {
        chorP->audio_buf[0][i] = 0;
        chorP->audio_buf[1][i] = 0;
    }

     //pointers:
    chorP->pnt = CHORUS_L - 1; //starts on top
    chorP->c1_pnt = 0;
    chorP->c2_pnt = 0;

    LAST_ADDR += (sizeof(struct Chorus));

    return LAST_ADDR;
}

int audio_chorus(_SPM struct Chorus *chorP, volatile _SPM short *xP, volatile _SPM short *yP) {
    // SINUSOIDAL MODULATION OF DELAY LENGTH
    chorP->del[0] = chorP->mod_array1[chorP->c1_pnt];
    chorP->del[1] = chorP->mod_array2[chorP->c2_pnt];
    chorP->c1_pnt = (chorP->c1_pnt + 1) % CHORUS_P1;
    chorP->c2_pnt = (chorP->c2_pnt + 1) % CHORUS_P2;
    //first, read sample
    chorP->audio_buf[0][chorP->pnt] = xP[0];
    chorP->audio_buf[1][chorP->pnt] = xP[1];
    //calculate AUDIO comb filter
    combFilter_2nd(CHORUS_L, &chorP->pnt, chorP->audio_buf, yP, chorP->accum, chorP->g, chorP->del);
    //update pointer
    if(chorP->pnt == 0) {
        chorP->pnt = CHORUS_L - 1;
    }
    else {
        chorP->pnt = chorP->pnt - 1;
    }

    return 0;
}

unsigned int alloc_delay_vars(_SPM struct IIRdelay *delP, unsigned int LAST_ADDR) {

    //initialise delay variables
    //set gains: for comb delay:
    delP->g[1] = ONE_16b;       // g0 = 1
    delP->g[0] = ONE_16b * 0.5; // g1 = 0.7
    //set delays:
    delP->del[1] = 0; // always d0 = 0
    delP->del[0] = DELAY_L - 1; // d1 = as long as delay buffer
    //pointer starts on top
    delP->pnt = DELAY_L - 1;

    //empty buffer
    for(int i=0; i<DELAY_L; i++) {
        delP->audio_buf[0][i] = 0;
        delP->audio_buf[1][i] = 0;
    }

    LAST_ADDR += (sizeof(struct IIRdelay));

    return LAST_ADDR;
}

int audio_delay(_SPM struct IIRdelay *delP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //first, read sample
    delP->audio_buf[0][delP->pnt] = xP[0];
    delP->audio_buf[1][delP->pnt] = xP[1];
    //calculate IIR comb filter
    combFilter_1st(DELAY_L, &delP->pnt, delP->audio_buf, yP, delP->accum, delP->g, delP->del);
    //replace content on buffer
    delP->audio_buf[0][delP->pnt] = yP[0];
    delP->audio_buf[1][delP->pnt] = yP[1];
    //update pointer
    if(delP->pnt == 0) {
        delP->pnt = DELAY_L - 1;
    }
    else {
        delP->pnt = delP->pnt -1;
    }

    return 0;
}

unsigned int alloc_overdrive_vars(_SPM struct Overdrive *odP, unsigned int LAST_ADDR) {

    LAST_ADDR += (sizeof(struct Overdrive));

    return LAST_ADDR;
}

int audio_overdrive(_SPM struct Overdrive *odP, volatile _SPM short *xP, volatile _SPM short *yP) {
    //THRESHOLD IS 1/3 = 0x2AAB
    //input abs:
    unsigned int x_abs[2];
    for(int j=0; j<2; j++) {
        x_abs[j] = abs(xP[j]);
        if(x_abs[j] > (2 * 0x2AAB)) { // saturation : y = 1
            if (xP[j] > 0) {
                yP[j] = 0x7FFF;
            }
            else {
                yP[j] = 0x8000;
            }
        }
        else {
            if(x_abs[j] > 0x2AAB) { // smooth overdrive: y = ( 3 - (2-3*x)^2 ) / 3;
                odP->accum[j] = (0x17FFF * x_abs[j]) >> 15 ; // result is 1 sign + 17 bits
                odP->accum[j] = 0xFFFF - odP->accum[j];
                odP->accum[j] = (odP->accum[j] * odP->accum[j]) >> 15;
                odP->accum[j] = 0x17FFF - odP->accum[j];
                odP->accum[j] = (odP->accum[j] * 0x2AAB) >> 15;
                if(xP[j] > 0) { //positive
                    if(odP->accum[j] > 32767) {
                        yP[j] = 32767;
                    }
                    else {
                        yP[j] = odP->accum[j];
                    }
                }
                else { // negative
                    yP[j] = -odP->accum[j];
                }
            }
            else { // linear zone: y = 2*x
                yP[j] = xP[j] << 1;
            }
        }
    }

    return 0;
}

unsigned int alloc_distortion_vars(_SPM struct Distortion *distP, unsigned int LAST_ADDR) {

    //initialise k, kOnePlus, shiftLeft:
    float amount = 0.9;
    distP->k = ( (2*amount)/(1-amount) ) * pow(2,15);
    distP->sftLft = 0;
    while(distP->k > ONE_16b) {
        distP->sftLft = distP->sftLft + 1;
        distP->k = distP->k >> 1;
    }
    distP->kOnePlus = (int)( ( (2*amount)/(1-amount) + 1 ) * pow(2,15) ) >> distP->sftLft;

    LAST_ADDR += (sizeof(struct Distortion));

    return LAST_ADDR;
}

int audio_distortion(_SPM struct Distortion *distP, volatile _SPM short *xP, volatile _SPM short *yP) {
    for(int j=0; j<2; j++) {
        distP->accum[0] = (distP->kOnePlus * xP[j]);// >> 15;
        distP->accum[1] = (distP->k * abs(xP[j])) >> 15;
        distP->accum[1] = distP->accum[1] + ((ONE_16b+distP->sftLft) >> distP->sftLft);
        distP->accum[0] = distP->accum[0] / distP->accum[1];
        //reduce if it is poisitive only
        if (xP[j] > 0) {
            yP[j] = distP->accum[0] - 1;
        }
        else {
            yP[j] = distP->accum[0];
        }
    }
    return 0;
}

int alloc_audio_vars(struct AudioFX *audioP, int FX_ID, fx_t FX_TYPE, con_t in_con, con_t out_con, unsigned int IN_SIZE, unsigned int OUT_SIZE, unsigned int P_AMOUNT, fst_t is_fst, lst_t is_lst) {
    /*
      LOCATION IN SPM
    */
    unsigned int BASE_ADDR = (unsigned int)mp_alloc(0);
    unsigned int LAST_ADDR;
    // FX ID
    const unsigned int ADDR_FXID = BASE_ADDR;
    LAST_ADDR                    = ADDR_FXID + sizeof(int);
    audioP->fx_id = (_SPM int * ) ADDR_FXID;
    *audioP->fx_id = FX_ID;
    // CPUID
    const unsigned int ADDR_CPUID  = LAST_ADDR;
    LAST_ADDR                      = ADDR_CPUID + sizeof(int);
    audioP->cpuid = ( _SPM int * ) ADDR_CPUID;
    *audioP->cpuid = get_cpuid();
    if(*audioP->cpuid == 0) {
        const unsigned int ADDR_IS_FST = LAST_ADDR;
        const unsigned int ADDR_IS_LST = ADDR_IS_FST + sizeof(int);
        LAST_ADDR                     = ADDR_IS_LST + sizeof(int);
        //SPM variables
        audioP->is_fst = ( _SPM fst_t * ) ADDR_IS_FST;
        audioP->is_lst = ( _SPM lst_t * ) ADDR_IS_LST;
        *audioP->is_fst = is_fst;
        *audioP->is_lst = is_lst;
    }
    // INPUT
    const unsigned int ADDR_IN_CON  = LAST_ADDR;
    const unsigned int ADDR_X_PNT   = ADDR_IN_CON + sizeof(int);
    const unsigned int ADDR_XB_SIZE = ADDR_X_PNT  + sizeof(int);
    //SPM variables
    audioP->in_con =  ( _SPM con_t *)          ADDR_IN_CON;
    audioP->x_pnt  =  ( _SPM unsigned int *)   ADDR_X_PNT;
    audioP->xb_size = ( _SPM unsigned int *)   ADDR_XB_SIZE;
    //init vars
    *audioP->in_con = in_con;
    *audioP->xb_size = IN_SIZE;
    //see what kind of node it is
    if (*audioP->in_con == NO_NOC) { //same core
        const unsigned int ADDR_X      = ADDR_XB_SIZE + sizeof(int);
        LAST_ADDR                     = ADDR_X       + IN_SIZE * 2 * sizeof(short);
        //SPM variables
        audioP->x      = ( volatile _SPM short *) ADDR_X;
        //initialise pointer values
        *audioP->x_pnt  = (int)audioP->x; // = ADDR_X;
    }
    else { //NoC
        const unsigned int ADDR_RECV_CP = ADDR_XB_SIZE + sizeof(int);
        LAST_ADDR                      = ADDR_RECV_CP + sizeof(int);
        //SPM variables
        audioP->recvChanP = ( _SPM unsigned int *) ADDR_RECV_CP;
    }
    // OUTPUT
    const unsigned int ADDR_OUT_CON = LAST_ADDR;
    const unsigned int ADDR_Y_PNT   = ADDR_OUT_CON + sizeof(int);
    const unsigned int ADDR_YB_SIZE = ADDR_Y_PNT   + sizeof(int);
    //SPM variables
    audioP->out_con = ( _SPM con_t *)        ADDR_OUT_CON;
    audioP->y_pnt   = ( _SPM unsigned int *) ADDR_Y_PNT;
    audioP->yb_size = ( _SPM unsigned int *) ADDR_YB_SIZE;
    //init vars
    *audioP->out_con = out_con;
    *audioP->yb_size = OUT_SIZE;
    if( (*audioP->out_con == NO_NOC) && (*audioP->is_lst == LAST) ) { //same core and last
        const unsigned int ADDR_Y          = ADDR_YB_SIZE    + sizeof(int);
        const unsigned int ADDR_LAST_INIT  = ADDR_Y          + OUT_SIZE * 2 * sizeof(short);
        const unsigned int ADDR_LAST_COUNT = ADDR_LAST_INIT  + sizeof(int);
        LAST_ADDR                          = ADDR_LAST_COUNT + sizeof(unsigned int);
        //SPM variables
        audioP->y          = ( volatile _SPM short *)        ADDR_Y;
        audioP->last_init  = ( _SPM int * )         ADDR_LAST_INIT;
        audioP->last_count = ( _SPM unsigned int *) ADDR_LAST_COUNT;
        //init values
        *audioP->y_pnt      = (int)audioP->y; // = ADDR_Y;
        if(LATENCY == 0) {
            *audioP->last_init = 0;
        }
        else {
            *audioP->last_init  = 1; //start: wait for latency
        }
        *audioP->last_count = 0; //start counting from 0
    }
    else { //NoC
        const unsigned int ADDR_SEND_CP = ADDR_YB_SIZE + sizeof(int);
        LAST_ADDR                      = ADDR_SEND_CP + sizeof(int);
        //SPM variables
        audioP->sendChanP = ( _SPM unsigned int *) ADDR_SEND_CP;
    }
    //PARAMETERS
    //Processing Type
    const unsigned int ADDR_PT   = LAST_ADDR;
    const unsigned int ADDR_P    = ADDR_PT   + sizeof(int);
    const unsigned int ADDR_RPR  = ADDR_P    + sizeof(int);
    const unsigned int ADDR_SPR  = ADDR_RPR  + sizeof(int);
    const unsigned int ADDR_PPSR = ADDR_SPR  + sizeof(int);
    LAST_ADDR                    = ADDR_PPSR + sizeof(int);
    //SPM variables
    audioP->pt   = ( _SPM pt_t *)         ADDR_PT;
    audioP->p    = ( _SPM unsigned int *) ADDR_P;
    audioP->rpr  = ( _SPM unsigned int *) ADDR_RPR;
    audioP->spr  = ( _SPM unsigned int *) ADDR_SPR;
    audioP->ppsr = ( _SPM unsigned int *) ADDR_PPSR;
    //init values
    *audioP->p = P_AMOUNT;
    //processing type:
    if(*audioP->xb_size == *audioP->yb_size) {
        *audioP->pt = XeY;
    }
    else {
        if(*audioP->xb_size > *audioP->yb_size) {
            *audioP->pt = XgY;
        }
        else {
            *audioP->pt = XlY;
        }
    }
    //Receives Per Run, Sends Per Run, Processings Per Send or Receive
    switch(*audioP->pt) {
    case XeY:
        *audioP->rpr  = 1;
        *audioP->spr  = 1;
        *audioP->ppsr = *audioP->xb_size / P_AMOUNT;
        break;
    case XgY:
        *audioP->rpr  = 1;
        *audioP->spr  = *audioP->xb_size / *audioP->yb_size;
        *audioP->ppsr = *audioP->yb_size / P_AMOUNT;
        break;
    case XlY:
        *audioP->rpr  = *audioP->yb_size / *audioP->xb_size;
        *audioP->spr  = 1;
        *audioP->ppsr = *audioP->xb_size / P_AMOUNT;
        break;
    }

    // FX TYPE
    const unsigned int ADDR_FX    = LAST_ADDR;
    LAST_ADDR                     = ADDR_FX   + sizeof(fx_t);
    //SPM variables
    audioP->fx    = ( _SPM fx_t *)         ADDR_FX;
    //init values
    *audioP->fx = FX_TYPE;
    const unsigned int ADDR_FX_PNT = LAST_ADDR;
    LAST_ADDR                      = ADDR_FX_PNT + sizeof(int);
    //SPM variables
    audioP->fx_pnt = ( _SPM unsigned int *) ADDR_FX_PNT;
    //switch between possible FX
    switch(*audioP->fx) {
    case DRY:
        //nothing to do
        break;
    case DRY_8S:
        //nothing to do
        break;
    case DELAY: ; //to create a scope
        _SPM struct IIRdelay *delayP;
        delayP = (_SPM struct IIRdelay *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)delayP; //points to function
        LAST_ADDR = alloc_delay_vars(delayP, LAST_ADDR);
        break;
    case OVERDRIVE: ;
        _SPM struct Overdrive *overdriveP;
        overdriveP = (_SPM struct Overdrive *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)overdriveP; //points to function
        LAST_ADDR = alloc_overdrive_vars(overdriveP, LAST_ADDR);
        break;
    case WAHWAH: ;
        _SPM struct WahWah *wahwahP;
        wahwahP = (_SPM struct WahWah *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)wahwahP; //points to function
        LAST_ADDR = alloc_wahwah_vars(wahwahP, LAST_ADDR);
        break;
    case CHORUS: ;
        _SPM struct Chorus *chorusP;
        chorusP = (_SPM struct Chorus *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)chorusP; //points to function
        LAST_ADDR = alloc_chorus_vars(chorusP, LAST_ADDR);
        break;
    case DISTORTION: ;
        _SPM struct Distortion *distortionP;
        distortionP = (_SPM struct Distortion *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)distortionP; //points to function
        LAST_ADDR = alloc_distortion_vars(distortionP, LAST_ADDR);
        break;
    case HP: ;
        _SPM struct Filter *hpfP;
        hpfP = (_SPM struct Filter *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)hpfP; //points to function
        LAST_ADDR = alloc_filter_vars(hpfP, LAST_ADDR, 5000, 0.707, 1);
        break;
    case LP: ;
        _SPM struct Filter *lpfP;
        lpfP = (_SPM struct Filter *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)lpfP; //points to function
        LAST_ADDR = alloc_filter_vars(lpfP, LAST_ADDR, 600, 0.707, 0);
        break;
    case BP: ;
        _SPM struct Filter *bpfP;
        bpfP = (_SPM struct Filter *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)bpfP; //points to function
        LAST_ADDR = alloc_filter_vars(bpfP, LAST_ADDR, 1000, 300, 2);
        break;
    case BR: ;
        _SPM struct Filter *brfP;
        brfP = (_SPM struct Filter *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)brfP; //points to function
        LAST_ADDR = alloc_filter_vars(brfP, LAST_ADDR, 500, 2000, 3);
        break;
    case VIBRATO: ;
        _SPM struct Vibrato *vibratoP;
        vibratoP = (_SPM struct Vibrato *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)vibratoP; //points to function
        LAST_ADDR = alloc_vibrato_vars(vibratoP, LAST_ADDR);
        break;
    case TREMOLO: ;
        _SPM struct Tremolo *tremoloP;
        tremoloP = (_SPM struct Tremolo *) LAST_ADDR;
        *audioP->fx_pnt = (unsigned int)tremoloP; //points to function
        LAST_ADDR = alloc_tremolo_vars(tremoloP, LAST_ADDR);
        break;
    default:
        printf("FX NOT IMPLEMENTED YET\n");
    }

    //update spm_alloc
    int retval = alloc_space(BASE_ADDR, LAST_ADDR);

    return retval;
}

int free_audio_vars(struct AudioFX *audioP) {
    //switch between possible FX
    switch(*audioP->fx) {
    case DELAY: ; //to create a scope
        _SPM struct IIRdelay *delP = (_SPM struct IIRdelay *)*audioP->fx_pnt;
        free(delP->audio_buf);
        break;
    case WAHWAH: ;
        _SPM struct WahWah *wahP = (_SPM struct WahWah *)*audioP->fx_pnt;
        free(wahP->fc_array);
        free(wahP->fb_array);
        free(wahP->a_array);
        free(wahP->b_array);
        break;
    case CHORUS: ;
        _SPM struct Chorus *chorP = (_SPM struct Chorus *)*audioP->fx_pnt;
        free(chorP->audio_buf);
        free(chorP->mod_array1);
        free(chorP->mod_array2);
        break;
    case VIBRATO: ;
        _SPM struct Vibrato *vibrP = (_SPM struct Vibrato *)*audioP->fx_pnt;
        free(vibrP->audio_buf_pnt);
        free(vibrP->sin_array_pnt);
        free(vibrP->frac_array_pnt);
        break;
    case TREMOLO: ;
        _SPM struct Tremolo *tremP = (_SPM struct Tremolo *)*audioP->fx_pnt;
        free(tremP->mod_array);
        free(tremP->frac_array);
        break;
    default:
        //nothing to free
        break;
    }

    return 0;
}

int audio_connect_same_core(struct AudioFX *srcP, struct AudioFX *dstP) {
    if ( (*srcP->out_con != NO_NOC) || (*dstP->in_con != NO_NOC) ) {
        printf("ERROR: IN/OUT CONNECTION TYPES\n");
        return 1;
    }
    if (*srcP->yb_size != *dstP->xb_size) {
        printf("ERROR: BUFFER SIZES DON'T MATCH: %d != %d\n", *srcP->yb_size, *dstP->xb_size);
        printf("sender: %d, receiver: %d\n", *srcP->fx_id, *dstP->fx_id);
        return 1;
    }

    *srcP->y_pnt = *dstP->x_pnt; //points to destination input
    return 0;
}

int audio_connect_to_core(struct AudioFX *srcP, const unsigned int sendChanID) {
    if (*srcP->out_con != NOC) {
        printf("ERROR IN CONNECTION\n");
        return 1;
    }
    else {
        *srcP->sendChanP = (unsigned int)mp_create_qport(sendChanID, SOURCE,
            (*srcP->yb_size * 4), CHAN_BUF_AMOUNT[sendChanID]); // ID, yb_size * 4 bytes, buf amount
        *srcP->y_pnt = (int)&((qpd_t *)*srcP->sendChanP)->write_buf;

        return 0;
    }
}

int audio_connect_from_core(const unsigned int recvChanID, struct AudioFX *dstP) {
    if (*dstP->in_con != NOC) {
        printf("ERROR IN CONNECTION\n");
        return 1;
    }
    else {
        *dstP->recvChanP = (unsigned int)mp_create_qport(recvChanID, SINK,
            (*dstP->xb_size * 4), CHAN_BUF_AMOUNT[recvChanID]); // ID, xb_size * 4 bytes, buf amount
        *dstP->x_pnt = (int)&((qpd_t *)*dstP->recvChanP)->read_buf;

        return 0;
    }
}

const int TIMEOUT = 5804;  // timeout ~256 samples
//const int TIMEOUT = 0;
//const int TIMEOUT = 0xFFFFF;

//int audio_process(struct AudioFX *audioP) __attribute__((section("text.spm")));
int audio_process(struct AudioFX *audioP) {
    int retval = 0;
    /* ---------X and Y locations----------- */
    volatile _SPM short * xP;
    volatile _SPM short * yP;
    if(*audioP->in_con == NO_NOC) { //same core : data=**x_pnt
        xP = (volatile _SPM short *)*audioP->x_pnt;
    }
    if(*audioP->out_con == NO_NOC) { //same core: data=**y_pnt
        yP = (volatile _SPM short *)*audioP->y_pnt;
    }

    else { //NoC: data=***y_pnt
        yP = (volatile _SPM short *)*(_SPM unsigned int *)*audioP->y_pnt;
        //yP = ((qpd_t *)*audioP->sendChanP)->write_buf;
    }

    /* ------------------SEND/PROCESS/RECEIVE---------------------*/

    unsigned int ind; //index used for each operation
    unsigned int offs; // can be x_offs or y_offs, depending on XgY or XlY

    switch(*audioP->pt) {
    case XeY:
        //check if it is 0, is last and needs to wait due to latency
        if( (*audioP->cpuid != 0) || (*audioP->is_lst == NO_LAST) || (*audioP->last_init == 0) ) {
            //printf("ID=%d: processing\n", *audioP->fx_id);
            //RECEIVE ONCE
            if(*audioP->in_con == NOC) { //receive from NoC
                if(mp_recv((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                    printf("RECV TIMED OUT!\n");
                    retval = 1;
                }
                //update X pointer after each recv
                xP = (volatile _SPM short *)*(_SPM unsigned int *)*audioP->x_pnt;
                //xP = ((qpd_t *)*audioP->recvChanP)->read_buf;
            }
            else { //same core
                if( (*audioP->cpuid == 0) && (*audioP->is_fst == FIRST) ) {
                    audioIn(audioP, xP);
                }
            }
            //PROCESS PPSR TIMES
            switch(*audioP->fx) {
            case DRY:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry(&xP[ind], &yP[ind]);
                }
                break;
            case DRY_8S:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry_8samples(&xP[ind], &yP[ind]);
                }
                break;
            case DELAY: ;
                _SPM struct IIRdelay *delP = (_SPM struct IIRdelay *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_delay(delP, &xP[ind], &yP[ind]);
                }
                break;
            case OVERDRIVE: ;
                _SPM struct Overdrive *odP = (_SPM struct Overdrive *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_overdrive(odP, &xP[ind], &yP[ind]);
                }
                break;
            case WAHWAH: ;
                _SPM struct WahWah *wahP = (_SPM struct WahWah *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_wahwah(wahP, &xP[ind], &yP[ind]);
                }
                break;
            case CHORUS: ;
                _SPM struct Chorus *chorP = (_SPM struct Chorus *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_chorus(chorP, &xP[ind], &yP[ind]);
                }
                break;
            case DISTORTION: ;
                _SPM struct Distortion *distP = (_SPM struct Distortion *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_distortion(distP, &xP[ind], &yP[ind]);
                }
                break;
            case HP:
            case LP:
            case BP:
            case BR: ;
                _SPM struct Filter *filtP = (_SPM struct Filter *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_filter(filtP, &xP[ind], &yP[ind]);
                }
                break;
            case VIBRATO: ;
                _SPM struct Vibrato *vibrP = (_SPM struct Vibrato *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_vibrato(vibrP, &xP[ind], &yP[ind]);
                }
                break;
            case TREMOLO: ;
                _SPM struct Tremolo *tremP = (_SPM struct Tremolo *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_tremolo(tremP, &xP[ind], &yP[ind]);
                }
                break;
            default:
                printf("effect not implemented yet\n");
                break;
            }
            //ACKNOWLEDGE ONCE AFTER PROCESSING
            if(*audioP->in_con == NOC) {
                if(mp_ack((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                    printf("ACK TIMED OUT!\n");
                    retval = 1;
                }
            }
            //SEND ONCE
            if(*audioP->out_con == NOC) { //send to NoC
                if(mp_send((qpd_t *)*audioP->sendChanP, TIMEOUT) == 0) {
                    printf("SEND TIMED OUT!\n");
                    retval = 1;
                }
            }
            else { //same core
                if( (*audioP->cpuid == 0) && (*audioP->is_lst == LAST) ) {
                    audioOut(audioP, yP);
                }
            }
        }
        //if it is last and needs to wait
        else {
            *audioP->last_count = *audioP->last_count + 1;
            //printf("increasing last_count, now is %u\n", *audioP->last_count);
            if(*audioP->last_count == LATENCY) {
                *audioP->last_init = 0;
                //printf("latency limit reached!\n");
            }
        }
        break;
    case XgY:
        //RECEIVE ONCE
        if(*audioP->in_con == NOC) { //receive from NoC
            if(mp_recv((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                //printf("RECV TIMED OUT!\n");
                retval = 1;
            }
            //update X pointer after each recv
            xP = (volatile _SPM short *)*(_SPM unsigned int *)*audioP->x_pnt;
            //xP = ((qpd_t *)*audioP->recvChanP)->read_buf;
        }
        //REPEAT SPR TIMES:
        for(unsigned int j=0;j<*audioP->spr; j++) {
            //PROCESS PPSR TIMES
            offs = 2 * j * (*audioP->yb_size);
            switch(*audioP->fx) {
            case DRY:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry(&xP[offs+ind], &yP[ind]);
                }
                break;
            case DRY_8S:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                   ind = 2 * i * (*audioP->p);
                    audio_dry_8samples(&xP[offs+ind], &yP[ind]);
                }
                break;
            case DELAY: ;
                _SPM struct IIRdelay *delP = (_SPM struct IIRdelay *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_delay(delP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case OVERDRIVE: ;
                _SPM struct Overdrive *odP = (_SPM struct Overdrive *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_overdrive(odP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case WAHWAH: ;
                _SPM struct WahWah *wahP = (_SPM struct WahWah *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_wahwah(wahP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case CHORUS: ;
                _SPM struct Chorus *chorP = (_SPM struct Chorus *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_chorus(chorP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case DISTORTION: ;
                _SPM struct Distortion *distP = (_SPM struct Distortion *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_distortion(distP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case HP:
            case LP:
            case BP:
            case BR: ;
                _SPM struct Filter *filtP = (_SPM struct Filter *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_filter(filtP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case VIBRATO: ;
                _SPM struct Vibrato *vibrP = (_SPM struct Vibrato *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_vibrato(vibrP, &xP[offs+ind], &yP[ind]);
                }
                break;
            case TREMOLO: ;
                _SPM struct Tremolo *tremP = (_SPM struct Tremolo *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_tremolo(tremP, &xP[offs+ind], &yP[ind]);
                }
                break;
            default:
                printf("effect not implemented yet\n");
                break;
            }
            //ACK: ONLY ONCE AT THE END
            if(j == (*audioP->spr - 1)) {
                if(*audioP->in_con == NOC) {
                    if(mp_ack((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                        //printf("ACK TIMED OUT!\n");
                        retval = 1;
                    }
                }
            }
            //SEND ONCE
            if(*audioP->out_con == NOC) { //send to NoC
                if(mp_send((qpd_t *)*audioP->sendChanP, TIMEOUT) == 0) {
                    //printf("SEND TIMED OUT!\n");
                    retval = 1;
                }
                //update Y pointer after each send
                yP = (volatile _SPM short *)*(_SPM unsigned int *)*audioP->y_pnt;
                //yP = ((qpd_t *)*audioP->sendChanP)->write_buf;
            }
        }
        break;
    case XlY:
        //REPEAT RPR TIMES:
        for(unsigned int j=0; j<*audioP->rpr; j++) {
            //RECEIVE ONCE
            if(*audioP->in_con == NOC) { //receive from NoC
                if(mp_recv((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                    //printf("RECV TIMED OUT!\n");
                    retval = 1;
                }
                //update X pointer after each recv
                xP = (volatile _SPM short *)*(_SPM unsigned int *)*audioP->x_pnt;
                //xP = ((qpd_t *)*audioP->recvChanP)->read_buf;
            }
            //PROCESS PPSR TIMES
            offs = 2 * j * (*audioP->xb_size);
            switch(*audioP->fx) {
            case DRY:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry(&xP[ind], &yP[offs+ind]);
                }
                break;
            case DRY_8S:
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry_8samples(&xP[ind], &yP[offs+ind]);
                }
                break;
            case DELAY: ;
                _SPM struct IIRdelay *delP = (_SPM struct IIRdelay *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_delay(delP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case OVERDRIVE: ;
                _SPM struct Overdrive *odP = (_SPM struct Overdrive *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_overdrive(odP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case WAHWAH: ;
                _SPM struct WahWah *wahP = (_SPM struct WahWah *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_wahwah(wahP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case CHORUS: ;
                _SPM struct Chorus *chorP = (_SPM struct Chorus *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_chorus(chorP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case DISTORTION: ;
                _SPM struct Distortion *distP = (_SPM struct Distortion *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_distortion(distP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case HP:
            case LP:
            case BP:
            case BR: ;
                _SPM struct Filter *filtP = (_SPM struct Filter *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_filter(filtP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case VIBRATO: ;
                _SPM struct Vibrato *vibrP = (_SPM struct Vibrato *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_vibrato(vibrP, &xP[ind], &yP[offs+ind]);
                }
                break;
            case TREMOLO: ;
                _SPM struct Tremolo *tremP = (_SPM struct Tremolo *)*audioP->fx_pnt;
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_tremolo(tremP, &xP[ind], &yP[offs+ind]);
                }
                break;
            default:
                printf("effect not implemented yet\n");
                break;
            }
            //ACK ONCE
            if(*audioP->in_con == NOC) {
                if(mp_ack((qpd_t *)*audioP->recvChanP, TIMEOUT) == 0) {
                    //printf("ACK TIMED OUT!\n");
                    retval = 1;
                }
            }
        }
        //SEND ONCE
        if(*audioP->out_con == NOC) { //send to NoC
            if(mp_send((qpd_t *)*audioP->sendChanP, TIMEOUT) == 0) {
                //printf("SEND TIMED OUT!\n");
                retval = 1;
            }
        }
        break;
    }

    return retval;
}


/*
int alloc_filter32_vars(struct Filter32 *filtP, int coreNumber, int Fc, float QorFb, int thisType) {
    printf("---------------FILTER INITIALISATION---------------\n");
    // LOCATION IN LOCAL SCRATCHPAD MEMORY
    const unsigned int FILT_X     = addr[coreNumber];
    const unsigned int FILT_Y     = FILT_X     + 2 * sizeof(short);
    const unsigned int FILT_ACCUM = FILT_Y     + 2 * sizeof(short);
    const unsigned int FILT_XBUF  = FILT_ACCUM + 2 * sizeof(long long int);
    const unsigned int FILT_YBUF  = FILT_XBUF  + 6 * sizeof(short); // 3rd ord, stereo
    const unsigned int FILT_A     = FILT_YBUF  + 6 * sizeof(short); // 3rd ord, stereo
    const unsigned int FILT_B     = FILT_A     + 3 * sizeof(int);
    const unsigned int FILT_PNT   = FILT_B     + 3 * sizeof(int);
    const unsigned int FILT_SLFT  = FILT_PNT   + sizeof(int);
    const unsigned int FILT_TYPE  = FILT_SLFT  + sizeof(int);

    //SPM variables
    filtP->x      = (volatile _SPM short *)      FILT_X;
    filtP->y      = (volatile _SPM short *)      FILT_Y;
    filtP->accum  = (volatile _SPM long long int *)        FILT_ACCUM;
    filtP->x_buf  = (volatile _SPM short (*)[2]) FILT_XBUF;
    filtP->y_buf  = (volatile _SPM short (*)[2]) FILT_YBUF;
    filtP->A      = (volatile _SPM int *)      FILT_A;
    filtP->B      = (volatile _SPM int *)      FILT_B;
    filtP->pnt    = (volatile _SPM int *)        FILT_PNT;
    filtP->sftLft = (volatile _SPM int *)        FILT_SLFT;
    filtP->type   = (volatile _SPM int *)        FILT_TYPE;

    //calculate filter coefficients (3rd order)
    *filtP->type = thisType;
    if (*filtP->type < 2) { //HP or LP
        filter_coeff_hp_lp_32(3, filtP->B, filtP->A, Fc, QorFb, filtP->sftLft, 0, thisType); //type: HPF or LPF
    }
    else { // 2 or 3: BP or BR
        filter_coeff_bp_br_32(3, filtP->B, filtP->A, Fc, (int)QorFb,  filtP->sftLft, 0);
    }
    //return new address
    int ALLOC_AMOUNT = alloc_space("FILTER", (FILT_TYPE + sizeof(int)), coreNumber);

    //store 1st samples:
    //first, fill filter buffer
    for(*filtP->pnt=0; *filtP->pnt < 2; *filtP->pnt = *filtP->pnt + 1) {
      getInputBufferSPM(&filtP->x_buf[*filtP->pnt][0], &filtP->x_buf[*filtP->pnt][1]);
    }

    return ALLOC_AMOUNT;
}

__attribute__((always_inline))
int audio_filter32(struct Filter32 *filtP){
    //increment pointer
    *filtP->pnt = ( (*(filtP->pnt)) + 1 ) % 3;
    //first, read sample
    filtP->x_buf[*filtP->pnt][0] = filtP->x[0];
    filtP->x_buf[*filtP->pnt][1] = filtP->x[1];
    //then, calculate filter
    filterIIR_2nd_32(*filtP->pnt, filtP->x_buf, filtP->y_buf, filtP->accum, filtP->B, filtP->A, *filtP->sftLft);
    //check if it is BP/BR
    if(*filtP->type == 2) { //BP
        filtP->accum[0] = ( (int)filtP->x[0] - (int)filtP->y_buf[*filtP->pnt][0] ) >> 1;
        filtP->accum[1] = ( (int)filtP->x[1] - (int)filtP->y_buf[*filtP->pnt][1] ) >> 1;
    }
    else {
        if(*filtP->type == 3) { //BR
            filtP->accum[0] = ( (int)filtP->x[0] + (int)filtP->y_buf[*filtP->pnt][0] ) >> 1;
            filtP->accum[1] = ( (int)filtP->x[1] + (int)filtP->y_buf[*filtP->pnt][1] ) >> 1;
        }
        else { //HP or LP
            filtP->accum[0] = filtP->y_buf[*filtP->pnt][0];
            filtP->accum[1] = filtP->y_buf[*filtP->pnt][1];
        }
    }
    //set output
    filtP->y[0] = (short)filtP->accum[0];
    filtP->y[1] = (short)filtP->accum[1];

    return 0;
}

int alloc_tremolo32_vars(struct Tremolo32 *tremP, int coreNumber) {
    printf("---------------TREMOLO INITIALISATION---------------\n");
    // LOCATION IN LOCAL SCRATCHPAD MEMORY
    const unsigned int TREM_X   = addr[coreNumber];
    const unsigned int TREM_Y   = TREM_X + 2 * sizeof(short);
    const unsigned int TREM_PNT = TREM_Y + 2 * sizeof(short);

    //SPM variables
    tremP->x      = ( volatile _SPM short *) TREM_X;
    tremP->y      = ( volatile _SPM short *) TREM_Y;
    tremP->pnt    = ( volatile _SPM int *)   TREM_PNT;

    //initialise modulation array
    storeSinLong(tremP->modArray, TREMOLO_P, (ONE_32b*0.6), (ONE_32b*0.3));

     //pointers:
    *tremP->pnt = 0;

    //return new address
    int ALLOC_AMOUNT = alloc_space("TREMOLO", (TREM_PNT + sizeof(int)), coreNumber);

    return ALLOC_AMOUNT;
}

__attribute__((always_inline))
int audio_tremolo32(struct Tremolo32 *tremP) {
    //update pointer
    *tremP->pnt = (*tremP->pnt + 1) % TREMOLO_P;
    //calculate output
    tremP->y[0] = (short)(( ((long long int)tremP->x[0]) * tremP->modArray[*tremP->pnt] ) >> 31);
    tremP->y[1] = (short)(( ((long long int)tremP->x[1]) * tremP->modArray[*tremP->pnt] ) >> 31);

    return 0;
}
*/

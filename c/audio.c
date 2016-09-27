#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include "audio.h"

#ifndef FILTER_ORDER_1PLUS
#define FILTER_ORDER_1PLUS 2
#endif

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

void setup() {
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
  char *dataAnalogue = "000010010"; //DAC selected, rest disabled, MIC muted, Line input select to ADC
  writeToI2C(addrAnalogue,dataAnalogue);


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

int getInputBuffer(short *l, short *r) {
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


int filterIIR(volatile _SPM int *pnt_i, volatile _SPM short (*x)[2], volatile _SPM short (*y)[2], volatile _SPM int *accum, volatile _SPM short *B, volatile _SPM short *A, int shiftLeft) {
  int pnt; //pointer for x_filter
  accum[0] = 0;
  accum[1] = 0;
  for(int i=0; i<FILTER_ORDER_1PLUS; i++) {
    pnt = (*pnt_i + i + 1) % FILTER_ORDER_1PLUS;
    // SIGNED SHIFT (arithmetical): losing a 6-bit resolution
    accum[0] += (B[i]*x[pnt][0]) >> 6;
    accum[0] -= (A[i]*y[pnt][0]) >> 6;
    accum[1] += (B[i]*x[pnt][1]) >> 6;
    accum[1] -= (A[i]*y[pnt][1]) >> 6;
  }
  //accumulator limits: [ (2^(30-6-1))-1 , -(2^(30-6-1)) ]
  //accumulator limits: [ 0x7FFFFF, 0x800000 ]
  // digital saturation
  for(int i=0; i<2; i++) {
    if (accum[i] > 0x7FFFFF) {
      accum[i] = 0x7FFFFF;
    }
    else {
      if (accum[i] < -0x800000) {
        accum[i] = -0x800000;
      }
    }
  }
  y[*pnt_i][0] = accum[0] >> (9-shiftLeft);
  y[*pnt_i][1] = accum[1] >> (9-shiftLeft);

  return 0;
}

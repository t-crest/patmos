#include "audio_singlecore.h"

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
 * @param[in]	guitar    used to select the input: line in or mic in
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
 * @return	returns 0 if successful
 */
int getInputBufferSPM(volatile _SPM short *l, volatile _SPM short *r) {
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
 * @return	returns 0 if successful
 */
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

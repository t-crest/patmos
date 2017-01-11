

#include "audio.h"
#include "dsp_algorithms.h"
#include "dsp_algorithms.c"
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

//----------------------------COMPLETE AUDIO FUNCTIONS---------------------------------//

/*            AUDIO EFFECT FUNCTIONS          */


void audioIn(struct AudioFX *audioP, volatile _SPM short *xP) {
    for(unsigned int i=0; i < *audioP->xb_size; i++) {
        getInputBufferSPM(&xP[i*2], &xP[i*2+1]);
        //printf("audio IN: %d, %d   [ 0x%x, 0x%x ] \n", xP[i*2], xP[i*2+1], (unsigned int)&xP[i*2], (unsigned int)&xP[i*2+1]);
    }
}

void audioOut(struct AudioFX *audioP, volatile _SPM short *yP) {
    for(unsigned int i=0; i < *audioP->yb_size; i++) {
        setOutputBufferSPM(&yP[i*2], &yP[i*2+1]);
        //printf("audio OUT: %d, %d   [ 0x%x, 0x%x ] \n", yP[i*2], yP[i*2+1], (unsigned int)&yP[i*2], (unsigned int)&yP[i*2+1]);
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

    //calculate filter coefficients (2nd order)
    filtP->type = thisType;
    if (filtP->type < 2) { //HP or LP
        filter_coeff_hp_lp(3, filtP->B, filtP->A, Fc, QorFb, &filtP->sftLft, 0, thisType); //type: HPF or LPF
    }
    else { // 2 or 3: BP or BR
        filter_coeff_bp_br(3, filtP->B, filtP->A, Fc, (int)QorFb,  &filtP->sftLft, 0);
    }

    filtP->pnt = 2;

    LAST_ADDR += (sizeof(struct Filter));

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
    for(int i=0; i<WAHWAH_P; i++) {
        filter_coeff_bp_br(3, wahP->B, wahP->A, wahP->fc_array[i], wahP->fb_array[i], &wahP->sftLft, 1);
        wahP->b_array[2][i] = wahP->B[2];
        wahP->b_array[1][i] = wahP->B[1];
        wahP->b_array[0][i] = wahP->B[0];
        wahP->a_array[2][i] = 0;
        wahP->a_array[1][i] = wahP->A[1];
        wahP->a_array[0][i] = wahP->A[0];
    }

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
    wahP->pnt = (wahP->pnt+1) % 3;
    //first, read sample
    wahP->x_buf[wahP->pnt][0] = xP[0];
    wahP->x_buf[wahP->pnt][1] = xP[1];
    //then, calculate filter
    filterIIR_2nd(&wahP->pnt, wahP->x_buf, wahP->y_buf, wahP->accum, wahP->B, wahP->A, &wahP->sftLft);
    //Band-Pass stuff
    wahP->accum[0] = ( (int)xP[0] - (int)wahP->y_buf[wahP->pnt][0] );
    wahP->accum[1] = ( (int)xP[1] - (int)wahP->y_buf[wahP->pnt][1] );
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

    //data and modulation arrays
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
    delP->g[0] = ONE_16b * 0.5; // g1 = 0.5
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

int alloc_audio_vars(struct AudioFX *audioP, int FX_ID, fx_t FX_TYPE, con_t in_con, con_t out_con, unsigned int RECV_AM, unsigned int SEND_AM, unsigned int IN_SIZE, unsigned int OUT_SIZE, unsigned int P_AMOUNT) {
    /*
      LOCATION IN SPM
    */
    unsigned int BASE_ADDR = (unsigned int)mp_alloc(0);
    unsigned int LAST_ADDR;
    // FX ID
    const unsigned int ADDR_FXID = BASE_ADDR;
    audioP->fx_id = (_SPM int * ) ADDR_FXID;
    *audioP->fx_id = FX_ID;
    // CPUID
    const unsigned int ADDR_CPUID  = ADDR_FXID + sizeof(int);
    audioP->cpuid = ( _SPM int * ) ADDR_CPUID;
    *audioP->cpuid = get_cpuid();
    // INPUT
    const unsigned int ADDR_IN_CON  = ADDR_CPUID + sizeof(int);
    const unsigned int ADDR_X_PNT   = ADDR_IN_CON + sizeof(int);
    const unsigned int ADDR_RECV_AM = ADDR_X_PNT  + sizeof(int)*RECV_AM;
    const unsigned int ADDR_XB_SIZE = ADDR_RECV_AM  + sizeof(int);
    //SPM variables
    audioP->in_con =  ( _SPM con_t *)          ADDR_IN_CON;
    audioP->x_pnt  =  ( _SPM unsigned int *)  ADDR_X_PNT;
    audioP->recv_am = ( _SPM unsigned int *)   ADDR_RECV_AM;
    audioP->xb_size = ( _SPM unsigned int *)   ADDR_XB_SIZE;
    //init vars
    *audioP->in_con = in_con;
    *audioP->recv_am = RECV_AM;
    *audioP->xb_size = IN_SIZE;
    //see what kind of node it is
    if ( (*audioP->in_con == SAME) || (*audioP->in_con == FIRST) ) { //same core or first
        const unsigned int ADDR_X      = ADDR_XB_SIZE + sizeof(int);
        LAST_ADDR                     = ADDR_X       + IN_SIZE * 2 * sizeof(short);
        //SPM variables
        audioP->x      = ( volatile _SPM short *) ADDR_X;
        //initialise pointer values
        *(audioP->x_pnt+0)  = (unsigned int)audioP->x; // = ADDR_X;
    }
    else { //NoC
        const unsigned int ADDR_RECV_CP = ADDR_XB_SIZE + sizeof(int);
        LAST_ADDR                      = ADDR_RECV_CP + sizeof(int)*RECV_AM;
        //SPM variables
        audioP->recvChanP = ( _SPM unsigned int *) ADDR_RECV_CP;
    }
    // OUTPUT
    const unsigned int ADDR_OUT_CON = LAST_ADDR;
    const unsigned int ADDR_Y_PNT   = ADDR_OUT_CON + sizeof(int);
    const unsigned int ADDR_SEND_AM = ADDR_Y_PNT   + sizeof(int)*SEND_AM;
    const unsigned int ADDR_YB_SIZE = ADDR_SEND_AM + sizeof(int);
    LAST_ADDR                       = ADDR_YB_SIZE + sizeof(int);
    //SPM variables
    audioP->out_con = ( _SPM con_t *)        ADDR_OUT_CON;
    audioP->y_pnt   = ( _SPM unsigned int *) ADDR_Y_PNT;
    audioP->send_am = ( _SPM unsigned int *) ADDR_SEND_AM;
    audioP->yb_size = ( _SPM unsigned int *) ADDR_YB_SIZE;
    //init vars
    *audioP->out_con = out_con;
    *audioP->send_am = SEND_AM;
    *audioP->yb_size = OUT_SIZE;
    if(*audioP->out_con == LAST) {
        const unsigned int ADDR_Y          = LAST_ADDR;
        const unsigned int ADDR_LAST_INIT  = ADDR_Y          + OUT_SIZE * 2 * sizeof(short);
        const unsigned int ADDR_LAST_COUNT = ADDR_LAST_INIT  + sizeof(int);
        LAST_ADDR                          = ADDR_LAST_COUNT + sizeof(unsigned int);
        //SPM variables
        audioP->y          = ( volatile _SPM short *)        ADDR_Y;
        audioP->last_init  = ( _SPM int * )         ADDR_LAST_INIT;
        audioP->last_count = ( _SPM unsigned int *) ADDR_LAST_COUNT;
        //init values
        *(audioP->y_pnt+0)      = (unsigned int)audioP->y; // = ADDR_Y;
        if(LATENCY == 0) {
            *audioP->last_init = 0;
        }
        else {
            *audioP->last_init  = 1; //start: wait for latency
        }
        *audioP->last_count = 0; //start counting from 0
    }
    else {
        if(*audioP->out_con == NOC) { //NoC
            const unsigned int ADDR_SEND_CP = LAST_ADDR;
            LAST_ADDR                      =  ADDR_SEND_CP + sizeof(int)*SEND_AM;
            //SPM variables
            audioP->sendChanP = ( _SPM unsigned int *) ADDR_SEND_CP;
        }
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
        //nothing to do
        break;
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
    if ( (*srcP->out_con != SAME) || (*dstP->in_con != SAME) ) {
        if(get_cpuid() == 0) {
            printf("ERROR: IN/OUT CONNECTION TYPES\n");
        }
        return 1;
    }
    if (*srcP->yb_size != *dstP->xb_size) {
        if(get_cpuid() == 0) {
            printf("ERROR: BUFFER SIZES DON'T MATCH: %d != %d\n", *srcP->yb_size, *dstP->xb_size);
            printf("sender: %d, receiver: %d\n", *srcP->fx_id, *dstP->fx_id);
        }
        return 1;
    }

    *(srcP->y_pnt+0) = *(dstP->x_pnt+0); //points to destination input
    return 0;
}

int audio_connect_to_core(struct AudioFX *srcP, const unsigned int sendChanID, unsigned int s_ind) {
    if (*srcP->out_con != NOC) {
        if(get_cpuid() == 0) {
            printf("ERROR IN CONNECTION\n");
        }
        return 1;
    }
    else {
        *(srcP->sendChanP+s_ind) = (unsigned int)mp_create_qport(sendChanID, SOURCE,
            (*srcP->yb_size * 4), CHAN_BUF_AMOUNT[sendChanID]); // ID, yb_size * 4 bytes, buf amount
        *(srcP->y_pnt+s_ind) = (int)&((qpd_t *)*(srcP->sendChanP+s_ind))->write_buf;
        if(get_cpuid() == 0) {
            printf("y_pnt[%d] address and sendChanP[%d] set\n", s_ind, s_ind);
        }
        return 0;
    }
}

int audio_connect_from_core(const unsigned int recvChanID, struct AudioFX *dstP, unsigned int r_ind) {
    if (*dstP->in_con != NOC) {
        if(get_cpuid() == 0) {
            printf("ERROR IN CONNECTION\n");
        }
        return 1;
    }
    else {
        *(dstP->recvChanP+r_ind) = (unsigned int)mp_create_qport(recvChanID, SINK,
            (*dstP->xb_size * 4), CHAN_BUF_AMOUNT[recvChanID]); // ID, xb_size * 4 bytes, buf amount
        *(dstP->x_pnt+r_ind) = (int)&((qpd_t *)*(dstP->recvChanP+r_ind))->read_buf;
        if(get_cpuid() == 0) {
            printf("x_pnt[%d] address and recvChanP[%d] set\n", r_ind, r_ind);
        }
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
    if(*audioP->in_con != NOC) { //same core : data=**x_pnt
        xP = (volatile _SPM short *)*(audioP->x_pnt+0);
    }
    if(*audioP->out_con != NOC) { //same core: data=**y_pnt
        yP = (volatile _SPM short *)*(audioP->y_pnt+0);
    }

    else { //NoC: data=***y_pnt
        yP = (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->y_pnt+0);
        //yP = ((qpd_t *)*audioP->sendChanP)->write_buf;
    }

    /* ------------------SEND/PROCESS/RECEIVE---------------------*/

    unsigned int ind; //index used for each operation
    unsigned int offs; // can be x_offs or y_offs, depending on XgY or XlY

    switch(*audioP->pt) {
    case XeY:
        //printf("%%%%\n ID: %d \n%%%%\n", *audioP->fx_id);
        //check if it is 0, is last and needs to wait due to latency
        if( (*audioP->cpuid != 0) || (*audioP->out_con == SAME) ||
            (*audioP->out_con == NOC) || (*audioP->last_init == 0) ) {
            //printf("ID=%d: processing\n", *audioP->fx_id);
            //RECEIVE ONCE
            if(*audioP->in_con == NOC) { //receive from NoC
                //receive from all recv channels
                for(int i=0; i<*audioP->recv_am; i++) {
                    if(mp_recv((qpd_t *)*(audioP->recvChanP+i), TIMEOUT) == 0) {
                        if(*audioP->cpuid == 0) {
                            printf("RECV TIMED OUT!\n");
                        }
                        retval = 1;
                    }
                }
                //update X pointer after each recv
                xP = (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->x_pnt+0/*((*audioP->recv_am)-1)*/);
                //after receiving, add all signals into recvChanel[0]
                int shift_am = *audioP->recv_am - 1;
                for(int i=1; i<(*audioP->recv_am); i++) {
                    volatile _SPM short * xnxtP =
                        (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->x_pnt+i);
                    for(int j=0; j<(*audioP->xb_size); j++) {
                        xP[2*j]   = (xP[2*j]>>shift_am)   + (xnxtP[2*j]>>shift_am);
                        xP[2*j+1] = (xP[2*j+1]>>shift_am) + (xnxtP[2*j+1]>>shift_am);
                    }
                }

            }
            else { //same core
                if( (*audioP->cpuid == 0) && (*audioP->in_con == FIRST) ) {
                    audioIn(audioP, xP);
                }
            }
            //PROCESS PPSR TIMES
            switch(*audioP->fx) {
            case DRY:
                //printf("PPSR %d\n", *audioP->ppsr);
                for(unsigned int i=0; i < *audioP->ppsr; i++) {
                    ind = 2 * i * (*audioP->p);
                    audio_dry(&xP[ind], &yP[ind]);
                    //printf("copied %d, %d   to %d, %d   [ 0x%x, 0x%x ] -> [ 0x%x, 0x%x ] \n", xP[ind], xP[ind+1], yP[ind], yP[ind+1], (unsigned int)&xP[ind], (unsigned int)&xP[ind+1], (unsigned int)&yP[ind], (unsigned int)&yP[ind+1]);
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
                if(get_cpuid() == 0) {
                    printf("effect not implemented yet\n");
                }
                break;
            }
            //ACKNOWLEDGE ONCE AFTER PROCESSING
            if(*audioP->in_con == NOC) {
                //acknowledge to all recv channels
                for(int i=0; i<*audioP->recv_am; i++) {
                    if(mp_ack((qpd_t *)*(audioP->recvChanP+i), TIMEOUT) == 0) {
                        if(*audioP->cpuid == 0) {
                            printf("ACK TIMED OUT!\n");
                        }
                        retval = 1;
                    }
                }
            }
            //SEND ONCE
            if(*audioP->out_con == NOC) { //send to NoC
                //before sending, copy send data to all send channel buffers
                for(int i=1; i<(*audioP->send_am); i++) {
                    volatile _SPM short * ynxtP =
                        (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->y_pnt+i);
                    for(int j=0; j<(*audioP->yb_size); j++) {
                        ynxtP[2*j] = yP[2*j];
                        ynxtP[2*j+1] = yP[2*j+1];
                    }
                }
                //send to all send channels
                for(int i=0; i<(*audioP->send_am); i++) {
                    if(mp_send((qpd_t *)*(audioP->sendChanP+i), TIMEOUT) == 0) {
                        if(*audioP->cpuid == 0) {
                            printf("SEND TIMED OUT!\n");
                        }
                        retval = 1;
                    }
                }
            }
            else { //same core
                if( (*audioP->cpuid == 0) && (*audioP->out_con == LAST) ) {
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
            if(mp_recv((qpd_t *)*(audioP->recvChanP+0), TIMEOUT) == 0) {
                //printf("RECV TIMED OUT!\n");
                retval = 1;
            }
            //update X pointer after each recv
            xP = (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->x_pnt+0);
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
                if(get_cpuid() == 0) {
                    printf("effect not implemented yet\n");
                }
                break;
            }
            //ACK: ONLY ONCE AT THE END
            if(j == (*audioP->spr - 1)) {
                if(*audioP->in_con == NOC) {
                    if(mp_ack((qpd_t *)*(audioP->recvChanP+0), TIMEOUT) == 0) {
                        //printf("ACK TIMED OUT!\n");
                        retval = 1;
                    }
                }
            }
            //SEND ONCE
            if(*audioP->out_con == NOC) { //send to NoC
                //before sending, copy send data to all send channel buffers
                for(int i=1; i<*audioP->send_am; i++) {
                    volatile _SPM short * ynxtP =
                        (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->y_pnt+i);
                    for(int j=0; j<(*audioP->yb_size); j++) {
                        ynxtP[2*j] = yP[2*j];
                        ynxtP[2*j+1] = yP[2*j+1];
                    }
                }
                //send to all send channels
                for(int i=0; i<*audioP->send_am; i++) {
                    if(mp_send((qpd_t *)*(audioP->sendChanP+i), TIMEOUT) == 0) {
                        //printf("SEND TIMED OUT!\n");
                        retval = 1;
                    }
                }
                //update Y pointer after each send
                yP = (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->y_pnt+0);
                //yP = ((qpd_t *)*audioP->sendChanP)->write_buf;
            }
        }
        break;
    case XlY:
        //REPEAT RPR TIMES:
        for(unsigned int j=0; j<*audioP->rpr; j++) {
            //RECEIVE ONCE
            if(*audioP->in_con == NOC) { //receive from NoC
                if(mp_recv((qpd_t *)*(audioP->recvChanP+0), TIMEOUT) == 0) {
                    //printf("RECV TIMED OUT!\n");
                    retval = 1;
                }
                //update X pointer after each recv
                xP = (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->x_pnt+0);
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
                if(get_cpuid() == 0) {
                    printf("effect not implemented yet\n");
                }
                break;
            }
            //ACK ONCE
            if(*audioP->in_con == NOC) {
                if(mp_ack((qpd_t *)*(audioP->recvChanP+0), TIMEOUT) == 0) {
                    //printf("ACK TIMED OUT!\n");
                    retval = 1;
                }
            }
        }
        //SEND ONCE
        if(*audioP->out_con == NOC) { //send to NoC
            //before sending, copy send data to all send channel buffers
            for(int i=1; i<*audioP->send_am; i++) {
                volatile _SPM short * ynxtP =
                    (volatile _SPM short *)*(_SPM unsigned int *)*(audioP->y_pnt+i);
                for(int j=0; j<(*audioP->yb_size); j++) {
                    ynxtP[2*j] = yP[2*j];
                    ynxtP[2*j+1] = yP[2*j+1];
                }
            }
            //send to all send channels
            for(int i=0; i<*audioP->send_am; i++) {
                if(mp_send((qpd_t *)*(audioP->sendChanP+0), TIMEOUT) == 0) {
                    //printf("SEND TIMED OUT!\n");
                    retval = 1;
                }
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

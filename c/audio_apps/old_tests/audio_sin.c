#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include <math.h>
#include "audio.h"
#include "audio.c"


/*
* @file		Audio_FirstSound.c
* @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
* @brief	Setting up all the registers in the audio interface
*/

int sineArray[220];



int cyclesStart = 0;
int nbrCycles = 0;
int sampleFreq = 1536; //256*6


/*
* @brief		Writes the supplied data to the address register,
				sets the request signal and waits for the acknowledge signal.
* @param[in]	addr	the address of which register to write to.
						Has to be 7 bit long.
* @param[in]	data	the data thats supposed to be written.
						Has to be 9 Bits long
* @reutrn		returns 0 if successful and a negative number if there was an error.
*/


void playSine() {
  //Fill sine array:
  for (int i = 0; i < 220; i++) {
    sineArray[i] = 16384*sin(2.0*M_PI* i /220);
    printf("Sin: %i\n",sineArray[i]);
  }

  for ( int j = 0; j < 600 ; j++) {
    for (int i = 0; i < 220; i++) {
      //waitSyncDac();
      setOutputBuffer(sineArray[i],sineArray[i]);
    }
  }

}


int main() {

  printf("CPU frequency: %d MHz\n", get_cpu_freq()/1000000);

	setup();

        setOutputBufferSize(32);

	*audioDacEnReg = 1;
	cyclesStart	= get_cpu_cycles();
	playSine();
	playSine();
//	playSine();
//	playSine();
//	playSine();
//	playSine();



/*
 //Simple sound:
  int counter = 0;
  int counter2 = 0;
  *audioDacEnReg = 1;
  while (counter2 < 400)
    {
      counter = 0;
      while(counter < 220)	{
	setOutputAudio(11241,23456);
	counter++;
      }
break;
      counter = 0;
      //if (counter % 20 == 0) printf("Hi %i\n",counter2);
      while(counter < 220)	{
	setOutputAudio(0,0);
	counter++;
      }
      counter2++;
    }

*/

  return 0;
}

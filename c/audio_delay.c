#include <machine/spm.h>
#include <machine/rtc.h>
#include <stdio.h>
#include "audio.h"
#include "audio.c"



/*
* @file		Audio_Demo.c
* @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
* @brief	Presentation demo.	
*/

short audioBufferL[52080] = {0}; //1 s maximum delay
short audioBufferR[52080] = {0}; //1 s maximum delay
int delayTime = 13020; // 250 ms


int main() {
	
	setup();

 
	short inL = 0;
	short inR = 0;
	*audioDacEnReg = 1;
	*audioAdcEnReg = 1;
	int length = sizeof(audioBufferL) / sizeof(short)-1;
	int delayPosition = 0;
	int p = 0;
	int delayLim = delayTime - 1;
	int currentKey = 0;
	int volume = 0;
	int volumeResult;

	while(*keyReg != 3)
	{
	  
	  switch(*keyReg) {
	  case 14:
	    if (currentKey != 14) {
	      currentKey = 14;
	      volume ++;
	      volumeResult = changeVolume(volume);
	      if(volumeResult == -1) {
		volume--;
		printf("Volume already at max: %i dB\n", volume);
	      }
	      else {
		printf("Volume %i dB\n", volume);
	      }
	    }
	    break;
	  case 13:
	    if (currentKey != 13) {
	      currentKey = 13;
	      volume--;
	      volumeResult = changeVolume(volume);
	      if(volumeResult == -1) {
		volume++;
		printf("Volume already at min: %i dB\n", volume);
	      }
	      else {
		printf("Volume %i dB\n", volume);
	      }
	    }
	    break;
	  case 6:
	    if (currentKey != 6) {
	      currentKey = 6;
	      delayTime = delayTime * 2;
	      if (delayTime > length) {
		delayTime = length;
		printf("Delay alreay at max: %i\n", delayTime);
	      }
	      else {
		printf("Delay samples: %i\n", delayTime);
	      }
			delayLim = delayTime - 1;
	    }
	    break;
	  case 5:
	    if (currentKey != 5) {
	      currentKey = 5;
	      if (delayTime > 1) {
		delayTime = (int)(delayTime/2);
		printf("Delay samples: %i\n", delayTime);
	      }
	      else {
		printf("Delay alreay at min: %i\n", delayTime);
	      }
			delayLim = delayTime - 1;
	    }
	    break;
	  default:			
		waitSyncDac();
	    currentKey = 0;
	    if(p >= delayTime) {
	      p=0;
	    }
	    getInputAudio(&inL,&inR);
	    if(p == delayLim) {
	      delayPosition = 0;
	    }
	    else {
	      delayPosition = p + 1;
	    }
	    //delayPosition = (p+1)%delayTime;			
	    audioBufferL[p] = inL + (short)(audioBufferL[delayPosition] >> 2);
		audioBufferR[p] = inR + (short)(audioBufferR[delayPosition] >> 2);
	    setOutputAudio(audioBufferL[p],audioBufferR[p]);	
	    p++;
	  }
	}
	return 0;
}




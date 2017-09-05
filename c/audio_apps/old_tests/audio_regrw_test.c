#include <machine/spm.h>
#include <stdio.h>
#include "audio.h"

/*
* @file		Audio_regReadWrite_test.c
* @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
* @brief	Testing whether in every register it can be written and read from. */


int main() {
	printf("Testprogram for the audio interface!\n");

	// Test program
	
	int loopNumber =0;
	for (int i = 0; i < 2; i++)
	{
		printf("Audio DAC L:\t%i\n"		,*audioDacLReg);
		printf("Audio DAC R:\t%i\n"		,*audioDacRReg);
		printf("Audio DAC Enable:\t%i\n",*audioDacEnReg);
		printf("Audio DAC Busy:\t%i\n"	,*audioDacBusyReg);
		printf("Audio DAC Req:\t%i\n"	,*audioDacReqReg);
		printf("Audio LRC :\t%i\n"		,*audioDacLrcReg);

		printf("Audio ADC L :\t%i\n"	,*audioAdcLReg);
		printf("Audio ADC R :\t%i\n"	,*audioAdcRReg);
		printf("Audio ADC Enable:\t%i\n",*audioAdcEnReg);
		printf("Audio ADC Busy:\t%i\n"	,*audioAdcBusyReg);
		printf("Audio ADC Req:\t%i\n"	,*audioAdcReqReg);
		printf("Audio LRC:\t%i\n"		,*audioAdcLrcReg);

		printf("I2C Data:\t%i\n"	,*i2cDataReg);
		printf("I2C Adr:\t%i\n"		,*i2cAdrReg);
		printf("I2C Ack:\t%i\n"		,*i2cAckReg);
		printf("I2C Req:\t%i\n"		,*i2cReqReg);

		*audioDacLReg 	=	1111;
		*audioDacRReg 	=	2222;
		*audioDacEnReg 	= 	1;
		*audioDacReqReg 	=	1;

		*audioAdcEnReg	= 1;
		*audioAdcReqReg = 1;

		*i2cDataReg 	=	333;
		*i2cAdrReg 	=	44;	
		*i2cReqReg 	=	1;
		printf("Changed values ...\n\n");
	}

    return 0;
}




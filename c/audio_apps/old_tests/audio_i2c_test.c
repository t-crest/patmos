#include <machine/spm.h>
#include <stdio.h>
#include "audio.h"

/*
* @file		control_reg_test.c
* @author	Daniel Sanz Ausin s142290 & Fabian Goerge s150957
* @brief	Testing I2C interface to write into WM8731 configuration registers	
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
int writeToI2C2(int addr,int data) {


	//temporary
	volatile _SPM int *i2cWrReg	= (volatile _SPM int *) PATMOS_IO_AUDIO+0x00B0;
	volatile _SPM int *i2cSDIN	= (volatile _SPM int *) PATMOS_IO_AUDIO+0x00C0;

	*i2cDataReg  = data;
	*i2cAdrReg = addr;
	*i2cReqReg = 1;

	while (*i2cAckReg == 0) {
	
		if (*i2cWrReg == 0) {
			*i2cSDIN = '0';
		}
		else {
			*i2cSDIN = '1';
		}
	}
	*i2cReqReg = 0;
	
	printf("Success ...\n");

	return 0;
}

int main() {
	
	printf("starting...\n");

	int addr = 123;
	int data = 444;

	printf("gonna write %d into %d\n", data, addr);
	writeToI2C2(addr, data);

	addr = 0;
	data = 12;

	printf("gonna write %d into %d\n", data, addr);
	writeToI2C2(addr, data);
	
	return 0;
}

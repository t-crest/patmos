/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2014 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

#include "includes.h"

/**
 * @file biss_api.c
 *
 * @brief Low level BiSS encoder interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup BISS BiSS Encoder interface
 *
 * @{
 */

//
// Low level write function for Biss interface
//
void Biss_Write(unsigned int base_addr, unsigned int reg, unsigned int data) {
	IOWR_32DIRECT(base_addr, reg, data);
}

//
// Low level read fiunction for Biss interface
//
unsigned int Biss_Read(unsigned int base_addr, unsigned int reg) {
	return IORD_32DIRECT(base_addr, reg);
}

//
// Reset the Biss interface
//
void Biss_Enc_Reset(unsigned int base_addr) {
	Biss_Write(base_addr, BISS_INSTR_ADR, BISS_INSTR_BREAK);
}


unsigned int Biss_Read_Master_Version(unsigned int base_addr) {
	return (Biss_Read(base_addr, BISS_REGCOM3_ADR) >> 24) + 36;
}

unsigned int Biss_Read_Master_Revision(unsigned int base_addr) {
	return (Biss_Read(base_addr, BISS_REGCOM3_ADR) >> 16) & 0xFF;
}

unsigned int Biss_Read_Supported_Slaves(unsigned int base_addr) {
int slaves = 0;

	for (slaves = 0; slaves < 8; slaves++ ) {
		Biss_Write(base_addr, BISS_SLCONFIG1_ADR + (4 * slaves), slaves);
		if ((Biss_Read(base_addr, BISS_SLCONFIG1_ADR + (4 * slaves)) != slaves) ||
				(Biss_Read(base_addr, BISS_SLCONFIG1_ADR)!=0) )
			break;
	}
	return slaves;
}


unsigned int Biss_Register_Access(unsigned int base_addr, unsigned int slaveum, unsigned int wnr,
		unsigned int address, unsigned int length) {

int REGCOM2_value;
unsigned int temp_reg_value;

int slave_id = 0;

slave_id = address >> 7; // If address > 0x7F then use slave_id to expand the addressing

//Store original value to restore later
REGCOM2_value = Biss_Read(base_addr, BISS_REGCOM2_ADR);


temp_reg_value = REGCOM2_value & 0xFF000700;
temp_reg_value = temp_reg_value | (0x14 <<16); //Freq divider = 80  - Slow down the clock for register accesses
temp_reg_value = temp_reg_value | (0x01);  //channel select = 1

temp_reg_value = temp_reg_value | (0x80 <<8); //cts=1 regvers=0
temp_reg_value = temp_reg_value | (slave_id <<11); //Slave ID = 1
Biss_Write(base_addr, BISS_REGCOM2_ADR, temp_reg_value);


//Write REG226 - WNR=wnr, REGADR(6:0)
//Write REG227 - , , REGNUM
temp_reg_value  = (((address & 0x7F) | ((wnr & 0x1)<<7))<< 16) | (((length-1) & 0x3F) << 24);
Biss_Write(base_addr, BISS_REGCOM1_ADR, temp_reg_value);


//Write chsel = 1, cts=1, regvers=1, slaveid=0

//Write reg 244 9 #INSTR = "100", AGS=1
//while (1) {
//Biss_Write(base_addr, BISS_INSTR_ADR, 0x09);
//Wait for EOT
int EOT = 0;
while (EOT == 0) {
	EOT = (Biss_Read(base_addr, BISS_STATUS_ADR) & 0x01); //EOT = bit 0
}
Biss_Write(base_addr, BISS_INSTR_ADR, 0x08); //AGS=0

int instr_val = Biss_Read(base_addr, BISS_INSTR_ADR);

//Test Reg0 in INSTRUCTION register
int i = 0;
while ((instr_val & 0x08)==8) {
	i++;
	OSTimeDlyHMSM(0, 0, 0, 1);
	instr_val = Biss_Read(base_addr, BISS_INSTR_ADR);
}
Biss_Write(base_addr, BISS_INSTR_ADR, 0x00);  //unset AGS

//Test REGEND=1 in STATUS register
int status_val = Biss_Read(base_addr, BISS_STATUS_ADR);
while (( status_val & 0x04)==0) {
	OSTimeDlyHMSM(0, 0, 0, 1);
	status_val = Biss_Read(base_addr, BISS_STATUS_ADR);
}

unsigned int nreg_err = (Biss_Read(base_addr, BISS_STATUS_ADR) & 0x08)==8;

while (EOT == 0) {
EOT = (Biss_Read(base_addr, BISS_STATUS_ADR) & 0x01); //EOT = bit 0
}
//Set back the original clock frequency
Biss_Write(base_addr, BISS_REGCOM2_ADR, REGCOM2_value);


return nreg_err;
};


unsigned int Biss_Configure_Slave_Simple(unsigned int base_addr) {
	int EOT = 0;

	//Reset all slaves
	int clearmem = 0;

	for (clearmem = 0; clearmem < 112/4; clearmem++ ) {
		Biss_Write(base_addr, 0x80 + (4 * clearmem), 0x00000000);
	}

	Biss_Write(base_addr, BISS_SLCONFIG1_ADR, (64+63) + ((128+0)<<8)); // SCDLEN=64, CRCPOLY=0);  //

	Biss_Write(base_addr, BISS_REGCOM2_ADR, 0x00030101); //Freq divider = 8 HOLDCDM = 1 ENMO=0 REGVERS=0 (Biss B)

	Biss_Write(base_addr, BISS_REGCOM3_ADR, 0x00007F82); //FreqAGS = Infinite  MO_BUSY = 128

	Biss_Write(base_addr, BISS_REGCOM4_ADR, 0x00000000); //SLAVELOC=1 SELSSI=0 BISSMOD=0

	//Execute INIT
	Biss_Write(base_addr, BISS_INSTR_ADR, 0x10); //INIT=1

	//OSTimeDlyHMSM(0, 0, 1, 0);
	OSTimeDlyHMSM(0, 0, 0, 500);

	//Wait for EOT
	while (EOT == 0) {
		EOT = (Biss_Read(base_addr, BISS_STATUS_ADR) & 0x01); //EOT = bit 0
	}

	//Set AGS bit
	Biss_Write(base_addr, BISS_INSTR_ADR, 0x0); //AGS = 0

	//Check auto-cycle frames
	int raw_status;
	int status_ok;
	raw_status = Biss_Read(base_addr, BISS_STATUS_ADR) & 0xFF;
	status_ok = ((raw_status | 0x27)!=255);
	if (status_ok == 0) {
//		return 1;	//error
	}



//	Biss_Read_Register(base_addr, 1, 0x0,16);
//	Biss_Read_Register(base_addr, 1, 0x60,8);
//	Biss_Read_Register(base_addr, 1, 0x78,8);
//	Biss_Read_Register(base_addr, 1, 0x0,64);
//	Biss_Read_Register(base_addr, 1, 0x40,64);


	Biss_Register_Access(base_addr, 1, 0, 0x78,8);


	//Decode Device EDS - Could skip this for now...

	//Calculate parameters

	//Stop Autocycle
	//Set AGS bit
	Biss_Write(base_addr, BISS_INSTR_ADR, 0x0); //AGS = 0

//	//Set freq, freqags, mo_busy, slave number
//
//	//Test config with single-cycle read
//	//Check error bits & decode
//


	return 0;
}

/*!
 * @}
 */

/*!
 * @}
 */

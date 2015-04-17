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

#ifndef Biss_API_H_
#define Biss_API_H_

/**
 * @file biss_api.h
 *
 * @brief Header file for low level BiSS encoder interface
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

/*****************************************************************************************/
/* address space of Biss                                                              */
/*****************************************************************************************/

//Sensor and Actuator Data (64-bits per sensor)
#define BISS_SCDATA1_LOW_ADR      	0x00
#define BISS_SCDATA1_HIGH_ADR      	0x04
#define BISS_SCDATA2_LOW_ADR      	0x08
#define BISS_SCDATA2_HIGH_ADR      	0x0C
#define BISS_SCDATA3_LOW_ADR      	0x10
#define BISS_SCDATA3_HIGH_ADR      	0x14
#define BISS_SCDATA4_LOW_ADR      	0x18
#define BISS_SCDATA4_HIGH_ADR      	0x1C
#define BISS_SCDATA5_LOW_ADR      	0x20
#define BISS_SCDATA5_HIGH_ADR      	0x24
#define BISS_SCDATA6_LOW_ADR      	0x28
#define BISS_SCDATA6_HIGH_ADR      	0x2C
#define BISS_SCDATA7_LOW_ADR      	0x30
#define BISS_SCDATA7_HIGH_ADR      	0x34
#define BISS_SCDATA8_LOW_ADR      	0x38
#define BISS_SCDATA8_HIGH_ADR      	0x3C

//Register Data
#define BISS_REGDATA1_ADR      		0x80
#define BISS_REGDATA2_ADR      		0x84
#define BISS_REGDATA3_ADR      		0x88
#define BISS_REGDATA4_ADR      		0x8C
#define BISS_REGDATA5_ADR      		0x90
#define BISS_REGDATA6_ADR      		0x94
#define BISS_REGDATA7_ADR      		0x98
#define BISS_REGDATA8_ADR      		0x9C
// Defined up to 64 REGDATAs

//Slave Configuration
#define BISS_SLCONFIG1_ADR     		0xC0
#define BISS_SLCONFIG2_ADR     		0xC4
#define BISS_SLCONFIG3_ADR     		0xC8
#define BISS_SLCONFIG4_ADR     		0xCC
#define BISS_SLCONFIG5_ADR     		0xD0
#define BISS_SLCONFIG6_ADR     		0xD4
#define BISS_SLCONFIG7_ADR     		0xD8
#define BISS_SLCONFIG8_ADR     		0xDC


//Register Communication Configuration & Master & Channel Configuration (fields are byte aligned)
#define BISS_REGCOM1_ADR     		0xE0
#define BISS_REGCOM2_ADR     		0xE4
#define BISS_REGCOM3_ADR     		0xE8
#define BISS_REGCOM4_ADR     		0xEC

//Status Information
#define BISS_STATUS_ADR     		0xF0

//Instruction Register
#define BISS_INSTR_ADR	     		0xF4

//Status Information2
#define BISS_STATUS2_ADR     		0xF8


#define BISS_INSTR_BREAK            0x80


//// STATUS bit macros
#define BISS_STATUS_EOT    			0x00000001
#define BISS_STATUS_REGEND 			0x00000004
#define BISS_STATUS_NREGERR			0x00000008
#define BISS_STATUS_NSCDERR			0x00000010
#define BISS_STATUS_NWDERR 			0x00000040
#define BISS_STATUS_NERR 			0x00000080
#define BISS_STATUS_SVALID1			0x00000100
#define BISS_STATUS_CDS				0x40000000
#define BISS_STATUS_CDMTO			0x80000000


// Function prototypes
void Biss_Reset(unsigned int base_addr);
void Biss_Enc_Reset(unsigned int base_addr);
void Biss_Write(unsigned int base_addr, unsigned int reg, unsigned int data);
unsigned int Biss_Read(unsigned int base_addr, unsigned int reg);

unsigned int Biss_Read_Master_Version(unsigned int base_addr);
unsigned int Biss_Read_Master_Revision(unsigned int base_addr);
unsigned int Biss_Read_Supported_Slaves(unsigned int base_addr);

unsigned int Biss_Configure_Slave_Simple(unsigned int base_addr);

unsigned int Biss_Register_Access(unsigned int base_addr, unsigned int slaveum, unsigned int wnr,
		unsigned int address, unsigned int length);


void Biss_Enc_Parameter_Read(unsigned int base_addr, int mrs, int adr, int *data);
void Biss_Enc_Parameter_Write(unsigned int base_addr, int mrs, int adr, int data);
void Biss_Enc_Send_Mode_Cmd(unsigned int base_addr, unsigned int mode, unsigned int mrs, unsigned int para);
int Biss_Enc_Read_Sensor_Data_Width(unsigned int base_addr);
int Biss_Enc_Rd_Sensor_Warning_Data(unsigned int base_addr);
int Biss_Enc_Rd_Sensor_Error_Data(unsigned int base_addr);
void Biss_Enc_Clear_Sensor_Errors(unsigned int base_addr);
void Biss_Enc_Clear_Sensor_Warnings(unsigned int base_addr);
void Biss_Enc_Tx_Pos(unsigned int base_addr);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* Biss_API_H_ */

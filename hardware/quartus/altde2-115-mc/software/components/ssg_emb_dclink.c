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
 * @file ssg_emb_dclink.c
 *
 * @brief DC Link interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup DCLINK DC Link Interface
 *
 * @brief DC Link voltage and current measurement
 *
 * The DC Link module uses sigma-delta ADCs to monitor the DC link current and voltage.
 *
 * @{
 */

/**
 * Setup the dc link component
 *
 * SYSID must have been decoded and pltform structure correctly populated
 */
void dc_link_setup(void)
{
	// Control parameters of DC-Link
	// DC Link ADC offset is scaled differently to the other ADCs. A value of 16384 offsets the
	// ADC reading correctly. A negative result (less than 50% 1s from the sigma-delta bitstream)
	// is clipped to 0 as the DC link voltage cannot be zero.
	// Result is scaled by half so full range is 15 bits equivalent
	if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
		// Altera power board
		// DC Link voltage is sensed across 820/1996000 divider
		// So voltage (Volts) is adc * 640 * 1996820 / 820 / 32767 / 1000
		// adc for given voltage is v * 820 * 32767 * 1000 / 640 / 1996820
		// Simplify to v * 82 * 32767 / 64 / 1996
		IOWR_16DIRECT(DOC_DC_LINK_P_BASE, DOC_DC_LINK_OFFSET    , 16384) ;			// sets 50% ADC bitstream to 0mV
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OFFSET      , 16384) ;
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OVERVOLTAGE , (platform.powerboard->overvoltage*82*32767)/(64*1996));
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_UNDERVOLTAGE, (platform.powerboard->undervoltage*82*32767)/(64*1996)) ;
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_K64         , 0) ;
	} else {
		// FalconEye power board
		// 400V DC Link acros 68/180068 divider => 151mV => 15000 counts
		// Equates to 37 counts/V
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OFFSET      , 16384) ;			// sets 50% ADC bitstream to 0mV
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OVERVOLTAGE , platform.powerboard->overvoltage*37) ;
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_UNDERVOLTAGE, platform.powerboard->undervoltage*37) ;
		IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_K64         , 0) ;
	}
}

void dc_link_chopper_setup(void)
{
    //---------------------------------------------------------------------------------------------------------------------
    // Control parameters of Chopper
    //---------------------------------------------------------------------------------------------------------------------
    IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_BRAKE_T     , 2024  ) ;
    IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_BRAKE_MAX   , 65535 ) ;
    IOWR_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_CHOPPER     , 65535 ) ;
}

/*
 * dc_link_read()
 *
 * Returns DC link voltage in V, current in mA
 */
void dc_link_read(short * voltage, short * current) {
	*current = 0;
	if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
		unsigned int i_tmp, v_tmp;
		// Altera Power board
		// ADCs are +/-320mV full scale 15 bit result so each count is 640/32767 mV
		// Current is sensed across 0R01 shunt so I = V/0.01 = V*100 mA
		// Multiply first to retain maximum precision
		// Ensuring intermediate result cannot overflow with a max adc reading of 16383
		// Also ensuring max result in mA (32000) fits in a short
		i_tmp = (unsigned int)IORD_16DIRECT(DOC_DC_LINK_P_BASE, DOC_DC_LINK_VBUS);
		*current = (short)((i_tmp*640*100)/32767);
		// DC Link voltage is sensed across 820/1996000 divider
		// So voltage is adc * 640 * 1996820 / 820 / 32767 / 1000
		// Musn't let intermediate result overflow
		// Simplify to adc * 64 * 1996 / 82 / 32767
		// With a max adc reading of 16383 this is OK
		v_tmp = (unsigned int)IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_VBUS);
		*voltage = (short)((v_tmp*64*1996)/(82*32767));
	} else {
		// Unused - No DC link current measurement
		*current = 0;
		// ADCs are +/-320mV full scale 15 bit result so each count is 640/32767
		// DC Link voltage is sensed across 68/180000 divider
		// So voltage is adc * 640 * 180068 / 68 / 32767 / 1000
		// Musn't let intermediate result overflow
		// Simplify to adc * 640 * 180 / 68 / 32767
		*voltage = (short)(((unsigned int)IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_VBUS)*640*180)/(68*32767));
	}
}

int dc_link_voltage_err_check(int base_address){
     return ((IORD_16DIRECT(base_address,0) >> 6)==4 );
}


void dc_link_read_config(short int id) {
	static unsigned short int last_offset = -1;
	unsigned short int offset = IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OFFSET);

	if (last_offset == offset) return; // If nothing changed do nothing

	debug_printf(DBG_INFO,
		"--->\tLAST_OFFSET\t%i\n\tOFFSET\t%i\n\tVBUS\t%i\n\tID\t%i\n",
		last_offset,offset,IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_VBUS),id);

	last_offset = offset;
	/*
	debug_printf(DBG_INFO,
		"--->\tOFFSET\t%i\n\tK64\t%i\n\tOVERVOLTAGE\t%i\n\tCHOPPER\t%i\n\tUNDERVOLTAGE\t%i\n\tBRAKE_T\t%i\n\tBRAKE_MAX\t%i\n\tVBUS\t%i\n\tBRAKELEVEL\t%i\n\tSTATUS\t%i\n",
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OFFSET),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_K64),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OVERVOLTAGE),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_CHOPPER),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_UNDERVOLTAGE),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_BRAKE_T),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_BRAKE_MAX),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_VBUS),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_BRAKELEVEL),
		IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_STATUS));
	*/
}


/*!
 * @}
 */

/*!
 * @}
 */

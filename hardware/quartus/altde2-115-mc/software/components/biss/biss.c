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
 * @file biss.c
 *
 * @brief BiSS encoder interface
 */

/*!
 * \addtogroup COMPONENTS Hardware Component Interfaces
 *
 * @brief Interface to hardware components used for motor control
 *
 * A number of hardware components are used to support the motor control application
 * for interfacing to external hardware (e.g. encoders) and for accelerating
 * software functions.
 *
 * @{
 */

/*!
 * \addtogroup BISS BiSS Encoder interface
 *
 * @brief Interface to BiSS encoders
 *
 * @{
 */

//
// Application specific initialisation of Biss interface and encoder
//
int Biss_Init(drive_params * dp) {
	unsigned int base_addr = dp->DOC_BISS_BASE_ADDR;

	// Reset the encoder
	Biss_Enc_Reset(base_addr);

	dp->encoder_version =  Biss_Read_Master_Version(base_addr);

	if (Biss_Configure_Slave_Simple(base_addr) == 1) {
		return 1;	//error
	}

	// Get sensor data
	Biss_Read_Sensor(dp);

	return 0;	//OK

}


//
// Read sensor data
//
void Biss_Read_Sensor(drive_params * dp) {
	int mphase_tmp;
	unsigned int base_addr = dp->DOC_BISS_BASE_ADDR;

	Biss_Register_Access(base_addr, 1, 0, 0x78,8);
	unsigned int biss_params = (Biss_Read(base_addr, BISS_REGDATA1_ADR) >> 24);
	unsigned int singleturn_bits = biss_params & 0x1F;
	unsigned int multiturn_lookup = (biss_params >> 5) & 0x7;

	dp->encoder_multiturn = 1;
	switch(multiturn_lookup) {
		case 0 : dp->encoder_multiturn_bits = 0; dp->encoder_multiturn = 0; break;
		case 1 : dp->encoder_multiturn_bits = 12; break;
		case 2 : dp->encoder_multiturn_bits = 16; break;
		case 3 : dp->encoder_multiturn_bits = 20; break;
		case 4 : dp->encoder_multiturn_bits = 24; break;
	}

	dp->encoder_length = singleturn_bits + dp->encoder_multiturn_bits;

	Biss_Register_Access(base_addr, 1, 0, 0xB0,2);
	mphase_tmp = Biss_Read(base_addr, BISS_REGDATA1_ADR);
	dp->mphase = (short)(mphase_tmp & 0xffff);

	dp->encoder_mask = (1<<(dp->encoder_length - dp->encoder_multiturn_bits)) - 1;
	dp->encoder_turns_mask = (1<<dp->encoder_multiturn_bits) -1;
	dp->encoder_singleturn_bits = dp->encoder_length - dp->encoder_multiturn_bits;
	dp->st_shift_phi_m = (dp->encoder_singleturn_bits - 16);			// 3 for 19 bit position

	//Setup BiSS Master to match decoded parameters
	Biss_Write(base_addr, BISS_SLCONFIG1_ADR, (64+(dp->encoder_length-1)+9) + ((0)<<8)); // SCDLEN=64, CRCPOLY=Disabled;  //

}

//
// Biss position read
//
void Biss_Read_Position(drive_params * dp) {
	int delta_phi;
	unsigned int positionCDS;
	unsigned int positionH;
	unsigned int positionL;
	unsigned int base_addr = dp->DOC_BISS_BASE_ADDR;


	positionH = Biss_Read(base_addr, BISS_SCDATA1_HIGH_ADR);
	positionL = Biss_Read(base_addr, BISS_SCDATA1_LOW_ADR);
	positionCDS = Biss_Read(base_addr, BISS_STATUS2_ADR);
	unsigned int extra_biss_bits = 9;
	int cds_bit = (positionCDS & 2 )>0;
	//	Biss_Write(base_addr, BISS_STATUS_ADR, 0xFFFFFFFF);	// Acknowledge read of position data

	if (dp->encoder_multiturn > 0) {
		dp->enc_turns = (positionH<<(32 - ((dp->encoder_singleturn_bits)+extra_biss_bits)) | positionL>>((dp->encoder_singleturn_bits)+extra_biss_bits))
							& dp->encoder_turns_mask;
	}
	// Adjust for actual 19 bit position data, masking the data bits
	dp->enc_data = ((cds_bit << ((dp->encoder_singleturn_bits)-1)) | (positionL >> extra_biss_bits)) & dp->encoder_mask;

	//dp->enc_data = dp->enc_data << 4;  //Scale up to 23-bits to match EnDat
	delta_phi = dp->enc_data - dp->enc_data_prev;

	if (delta_phi > 0x3ffff) {
		delta_phi -= 0x7ffff;
	} else if (delta_phi < -0x3ffff) {
		delta_phi += 0x7ffff;
	}

	// Position difference is 23 bits representing one turn.
	// v = (phi1 - phi0)/2^^23/(t1 - t0)
	// Result in rpm, so v = (phi1 - phi0)/2^^23/(t1 - t0)*60
	// ==>v = delta_phi*k ==> v = delta_phi*(1/8388608)*16000*60
	// ==>v = delta_phi**0.1144
	dp->speed_encoder = (1875*delta_phi)>>(14-SPEED_FRAC_BITS+(dp->encoder_singleturn_bits)-23);		//23 = Original EnDat  19= BiSS
//	if (abs(dp->speed_encoder) > 6000) {
//		debug_printf(DBG_INFO, "SP=%d ENC=%08X PREV=%08X DEL=%08X\n", dp->speed_encoder, dp->enc_data, dp->enc_data_prev, delta_phi);
//	}

	dp->phi_mech = (dp->enc_data >> (dp->st_shift_phi_m)) & 0xffff;

//	dp->enc_data = dp->enc_data << (23-dp->encoder_singleturn_bits);  //Shift the enc_data bits to 23-bits to match EnDat

	dp->enc_data_prev = dp->enc_data;

}

//
// Biss encoder offset calibration
//
void Biss_Service(drive_params * dp) {

	unsigned int base_addr = dp->DOC_BISS_BASE_ADDR;

    int    index      = 0;
    int    id_phase   = 0;
    int    write_cnt  = 0;
    int e_phase_read  = 0;
    int e_phase_write = 0;

    dp->enable_drive  = 0 ;
    dp->mpoles        = 0 ;
    dp->mphase        = 0 ;
    dp->phi_elec      = 0 ;
    dp->i_command_q   = 0 ;
    dp->i_command_d   = 700;


	dsm_reset(dp->DOC_SM_BASE_ADDR);

        debug_printf(DBG_INFO, "---> ------------------------------------\n");
        debug_printf(DBG_INFO, "---> STATE %i of Motor ( Motor OFF ) \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);

	OSTimeDlyHMSM(0, 0, 0, 100);
    IOWR_16DIRECT(dp->DOC_ADC_BASE_ADDR, ADC_D, 1);
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 1 ) ; // Pre-Charge  State
        debug_printf(DBG_INFO, "---> STATE %i of Motor \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
	OSTimeDlyHMSM(0, 0, 0, 100);
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 2 ) ;  // Pre-Run  State
        debug_printf(DBG_INFO, "---> STATE %i of Motor \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
	OSTimeDlyHMSM(0, 0, 0, 100);
    dp->enable_drive = 1;
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 3 ) ;  // Run - State
        debug_printf(DBG_INFO, "---> STATE %i of Motor ( Motor ON ) \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
        debug_printf(DBG_INFO, "---> Measurement and calculation of angle\n");

	//OSTimeDlyHMSM(0, 0, 1, 0);
	OSTimeDlyHMSM(0, 0, 0, 500);

    // Average encoder reading over 40ms to calculate commutation angle
    id_phase = 0;
    for( index=0 ; index<4096 ; index++ ){
        id_phase  += dp->phi_mech ;
		OSTimeDlyHMSM(0, 0, 0, 1);
    }
    id_phase = id_phase >> 12;

    // Some dodgy maths mixing un/signed different lengths relying on the way over/underflow will look like
    // a wraparound over a 2pi range
    // Multiply by 4 and mask gives us the electrical offset within a pi/2 quadrant
    e_phase_write  = -((id_phase*4) & 0xFFFF );
    // 0x4000 is 1/4 full scale which represents the pi/2 degree electrical phase offset required for commutation
    e_phase_write  = (e_phase_write + 0x4000) & 0xFFFF;
    e_phase_read = 0 ;


    //Disable PWM & sensor interrupts
    dsm_reset_to_idle(dp->DOC_SM_BASE_ADDR);
	OSTimeDlyHMSM(0, 0, 0, 100);

	Biss_Register_Access(base_addr, 1, 0, 0xB0,2);
	e_phase_read = Biss_Read(base_addr, BISS_REGDATA1_ADR) & 0xFFFF;

		debug_printf(DBG_INFO, "     Current commutation Angle  : 0x%X  0d%d\n",(short)e_phase_read, (short)e_phase_read);

    index=0;
    while(index<5 ){
        if ((short)e_phase_write!=(short)e_phase_read){

        	//Setup write value in shadow registers
        	Biss_Write(base_addr, BISS_REGDATA1_ADR, e_phase_write);
        	Biss_Write(base_addr, BISS_REGDATA2_ADR, 0);
        	//Write Biss Registers (1 Byte at a time)
        	Biss_Register_Access(base_addr, 1, 1, 0xB0,1);
			OSTimeDlyHMSM(0, 0, 0, 100);
        	Biss_Write(base_addr, BISS_REGDATA1_ADR, e_phase_write >> 8);
        	Biss_Register_Access(base_addr, 1, 1, 0xB1,1);
			OSTimeDlyHMSM(0, 0, 0, 100);
            //Zero out shadow registers
            Biss_Write(base_addr, BISS_REGDATA1_ADR, 0x0);
        	Biss_Write(base_addr, BISS_REGDATA2_ADR, 0x0);
        	//Read Biss Registers
        	Biss_Register_Access(base_addr, 1, 0, 0xB0,2);
			OSTimeDlyHMSM(0, 0, 0, 100);
        	e_phase_read = Biss_Read(base_addr, BISS_REGDATA1_ADR) & 0xFFFF;
            write_cnt++;
        }
       index++;
    }

        debug_printf(DBG_INFO, "---> ------------------------------------\n");
        debug_printf(DBG_INFO, "---> <Conrol value of phi>\n");
        debug_printf(DBG_INFO, "---> Phi Mech            : 0x%X 0d%d\n",id_phase,id_phase);
        debug_printf(DBG_INFO, "---> Phi Elec (4 poles)  : 0x%X 0d%d\n",id_phase*4, id_phase*4);
        debug_printf(DBG_INFO, "---> Phi Elec with 16 Bit: 0x%X 0d%d\n",(id_phase*4) & 0xFFFF, (id_phase*4) & 0xFFFF);
        debug_printf(DBG_INFO, "---> Phi Elec invers     : 0x%X 0d%d\n",(unsigned short)(-((id_phase*4) & 0xFFFF)), (unsigned short)(-((id_phase*4) & 0xFFFF)));
        debug_printf(DBG_INFO, "---> Phi Elec Offset     : 0x%X 0d%d\n",(unsigned short)(-((id_phase*4) & 0xFFFF)),(unsigned short)(-((id_phase*4) & 0xFFFF)));
        debug_printf(DBG_INFO, "---> ------------------------------------\n");
        debug_printf(DBG_INFO, "---> Write attempts      : %i\n",write_cnt);
        debug_printf(DBG_INFO, "---> ------------------------------------\n");
        debug_printf(DBG_INFO, "---> Write offset angle : 0x%X 0d%d \n",(short)e_phase_write, (short)e_phase_write);
        debug_printf(DBG_INFO, "---> Read  offset angle : 0x%X 0d%d \n",(short)e_phase_read, (short)e_phase_read);
        debug_printf(DBG_INFO, "---> ------------------------------------\n");
        if((short)e_phase_write==(short)e_phase_read){
            debug_printf(DBG_INFO, "---> Read and Write of angle in BiSS EEPROM was successful! \n");
        }else if((short)e_phase_write!=(short)e_phase_read){
            debug_printf(DBG_ERROR, "Error: ---> Read and Write of angle in BiSS-EEPROM was not successful! \n");
            debug_printf(DBG_ERROR, "       ---> Encoder is not working! \n");
        }

    IOWR_16DIRECT(dp->DOC_ADC_BASE_ADDR, ADC_D, 0);
    dsm_reset(dp->DOC_SM_BASE_ADDR);
    dp->enable_drive = 0 ;

        debug_printf(DBG_INFO, "---> Motor OFF \n");
        debug_printf(DBG_INFO, "---> ------------------------------------\n");
}

/*!
 * @}
 */

/*!
 * @}
 */

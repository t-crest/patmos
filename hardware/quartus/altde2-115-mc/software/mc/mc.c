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

/**
 * @file mc.c
 *
 * @brief Motor control demo functions
 */

#include "includes.h"

/*!
 * \addtogroup MC
 *
 * @{
 */

void init_dp(drive_params * dp) {
	dp[0].I_sat_limit = 3125;
	dp[0].V_sat_limit = 3000;

	dp[0].Iq_Kp = 400; //Iq proportional gain constant
	dp[0].Iq_Ki = 20; //Iq integral gain constant

	dp[0].Id_Kp = 400; //Id proportional gain constant
	dp[0].Id_Ki = 20; //Id integral gain constant

	dp[0].mpoles = 4; // Set motor pole pairs
	dp[0].mphase = 0;
	dp[0].i_command_d = 0;
	dp[0].i_command_q = 100;

	dp[0].encoder_length = 23;
	dp[0].speed_limit = 3000; // Setpoint / Initial Speed
	dp[0].speed_command = 200; // Setpoint / Initial Speed
	dp[0].speed_Kp = 20000; // Kp_speed proportional parameter Speed Control
	dp[0].speed_Ki = 500; //Ki_speed integral    parameter Speed Control
	dp[0].pos_Kp = 50;
	dp[0].pos_setpoint = 100000; // Position 23 Bit = 8388607 count = 360°
	dp[0].pos_err_limit = Bit23;
	dp[0].pos_limit = 50;
	dp[0].stepsize = 1;
	dp[0].openloop_test = OPEN_LOOP_INIT;
	dp[0].DOC_ADC_BASE_ADDR       = DRIVE0_DOC_ADC_BASE;
//km 	dp[0].DOC_ENDAT_BASE_ADDR     = DRIVE0_DOC_ENDAT_BASE;
	dp[0].DOC_BISS_BASE_ADDR      = DRIVE0_DOC_BISS_BASE;
	dp[0].DOC_PWM_BASE_ADDR       = DRIVE0_DOC_PWM_BASE;
	dp[0].DOC_SM_BASE_ADDR        = DRIVE0_DOC_SM_BASE;

#ifdef DRIVE1_DOC_ADC_BASE
	if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
		dp[1].I_sat_limit = 3125;
		dp[1].V_sat_limit = 3000;

		dp[1].Iq_Kp = 400; //Iq proportional gain constant
		dp[1].Iq_Ki = 20; //Iq integral gain constant

		dp[1].Id_Kp = 400; //Id proportional gain constant
		dp[1].Id_Ki = 20; //Id integral gain constant

		dp[1].mpoles = 4; // Set motor pole pairs
		dp[1].mphase = 0;
		dp[1].i_command_d = 0;
		dp[1].i_command_q = 100;

		dp[1].encoder_length = 23;
		dp[1].speed_limit = 3000; // Setpoint / Initial Speed
		dp[1].speed_command = 100; // Setpoint / Initial Speed
		dp[1].speed_Kp = 20000; //Kp_speed proportional parameter Speed Control
		dp[1].speed_Ki = 500; //Ki_speed integral    parameter Speed Control
		dp[1].pos_Kp = 50;
		dp[1].pos_setpoint = 100000; // Position 23 Bit = 8388607 count = 360°
		dp[1].pos_err_limit = Bit23;
		dp[1].pos_limit = 50;
		dp[1].stepsize = 1;
		dp[1].openloop_test = OPEN_LOOP_INIT;
		dp[1].DOC_ADC_BASE_ADDR       = DRIVE1_DOC_ADC_BASE;
//km 		dp[1].DOC_ENDAT_BASE_ADDR     = DRIVE1_DOC_ENDAT_BASE;
		dp[1].DOC_BISS_BASE_ADDR      = DRIVE1_DOC_BISS_BASE;
		dp[1].DOC_PWM_BASE_ADDR       = DRIVE1_DOC_PWM_BASE;
		dp[1].DOC_SM_BASE_ADDR        = DRIVE1_DOC_SM_BASE;

		dp[2].I_sat_limit = 3125;
		dp[2].V_sat_limit = 3000;

		dp[2].Iq_Kp = 400; //Iq proportional gain constant
		dp[2].Iq_Ki = 20; //Iq integral gain constant

		dp[2].Id_Kp = 400; //Id proportional gain constant
		dp[2].Id_Ki = 20; //Id integral gain constant

		dp[2].mpoles = 4; // Set motor pole pairs
		dp[2].mphase = 0;
		dp[2].i_command_d = 0;
		dp[2].i_command_q = 100;

		dp[2].encoder_length = 23;
		dp[2].speed_limit = 3000; // Setpoint / Initial Speed
		dp[2].speed_command = 100; // Setpoint / Initial Speed
		dp[2].speed_Kp = 20000; // 20000 Kp_speed poportional parameter Speed Control
		dp[2].speed_Ki = 500; // 500 Ki_speed integral    parameter Speed Control
		dp[2].pos_Kp = 50;
		dp[2].pos_setpoint = 100000; // Position 23 Bit = 8388607 count = 360°
		dp[2].pos_err_limit = Bit23;
		dp[2].pos_limit = 50;
		dp[2].stepsize = 1;
		dp[2].openloop_test = OPEN_LOOP_INIT;
		dp[2].DOC_ADC_BASE_ADDR       = DRIVE2_DOC_ADC_BASE;
//km 		dp[2].DOC_ENDAT_BASE_ADDR     = DRIVE2_DOC_ENDAT_BASE;
		dp[2].DOC_BISS_BASE_ADDR      = DRIVE2_DOC_BISS_BASE;
		dp[2].DOC_PWM_BASE_ADDR       = DRIVE2_DOC_PWM_BASE;
		dp[2].DOC_SM_BASE_ADDR        = DRIVE2_DOC_SM_BASE;


		dp[3].I_sat_limit = 3125;
		dp[3].V_sat_limit = 3000;

		dp[3].Iq_Kp = 400; //Iq proportional gain constant
		dp[3].Iq_Ki = 20; //Iq integral gain constant

		dp[3].Id_Kp = 400; //Id proportional gain constant
		dp[3].Id_Ki = 20; //Id integral gain constant

		dp[3].mpoles = 4; // Set motor pole pairs
		dp[3].mphase = 0;
		dp[3].i_command_d = 0;
		dp[3].i_command_q = 100;

		dp[3].encoder_length = 23;
		dp[3].speed_limit = 3000; // Setpoint / Initial Speed
		dp[3].speed_command = 100; // Setpoint / Initial Speed
		dp[3].speed_Kp = 20000; // 20000 Kp_speed poportional parameter Speed Control
		dp[3].speed_Ki = 500; // 500 Ki_speed integral    parameter Speed Control
		dp[3].pos_Kp = 50;
		dp[3].pos_setpoint = 100000; // Position 23 Bit = 8388607 count = 360°
		dp[3].pos_err_limit = Bit23;
		dp[3].pos_limit = 50;
		dp[3].stepsize = 1;
		dp[3].openloop_test = OPEN_LOOP_INIT;
		dp[3].DOC_ADC_BASE_ADDR       = DRIVE3_DOC_ADC_BASE;
//km 		dp[3].DOC_ENDAT_BASE_ADDR     = DRIVE3_DOC_ENDAT_BASE;
		dp[3].DOC_BISS_BASE_ADDR      = DRIVE3_DOC_BISS_BASE;
		dp[3].DOC_PWM_BASE_ADDR       = DRIVE3_DOC_PWM_BASE;
		dp[3].DOC_SM_BASE_ADDR        = DRIVE3_DOC_SM_BASE;

	}
#endif
}

/*!
 * @}
 */

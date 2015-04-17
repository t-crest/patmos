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
 * @file mc_debug.c
 *
 * @brief Interface functions for system console debug GUI
 */

#include "includes.h"

unsigned int dbg_level = DBG_ALL;

/*!
 * \addtogroup DEBUG
 *
 * @{
 */

/**
 * \addtogroup DEBUG_MSG
 *
 * @{
 */

/**
 * \endcond
 */
/*!
 * @}
 */

/*
 * init_debug (re-)initialize drive parameters for one axis
 */
void init_debug (int dn, drive_params * dp) {
    debug_write_status (dn, DOC_DBG_DEMO_MODE, 0);
	debug_write_status (dn, DOC_DBG_OL_EN, dp[dn].openloop_test);//MAX

    debug_write_status (dn, DOC_DBG_I_PI_KP, dp[dn].Id_Kp);
    debug_write_status (dn, DOC_DBG_I_PI_KI, dp[dn].Id_Ki);

    debug_write_status (dn, DOC_DBG_I_PI_FB_LIM, dp[dn].I_sat_limit);
    debug_write_status (dn, DOC_DBG_I_PI_OP_LIM, dp[dn].V_sat_limit);

    debug_write_status (dn, DOC_DBG_SPEED_PI_KP, dp[dn].speed_Kp);
    debug_write_status (dn, DOC_DBG_SPEED_PI_KI, dp[dn].speed_Ki);
    debug_write_status (dn, DOC_DBG_SPEED_PI_FB_LIM, dp[dn].speed_limit);
    debug_write_status (dn, DOC_DBG_SPEED_SETP0, dp[dn].speed_command);
    debug_write_status (dn, DOC_DBG_AXIS_SELECT, 0);
    debug_write_status (dn, DOC_DBG_FFT_PING_STATUS, 0);
    debug_write_status (dn, DOC_DBG_FFT_PONG_STATUS, 0);


    debug_write_status (dn, DOC_DBG_POS_SETP0, dp[dn].pos_setpoint);
    debug_write_status (dn, DOC_DBG_POS_MODE, dp[dn].enable_position_control);
    debug_write_status (dn, DOC_DBG_POS_SPEED, dp[dn].pos_limit);
    debug_write_status (dn, DOC_DBG_POS_PI_KP, dp[dn].pos_Kp);
    debug_write_status (dn, DOC_DBG_POS_SPDFF_KP, dp[dn].pos_spdff_Kp);

}

/*
 * poll_debug Put new values passed from system console GUI into drive parameters
 */
void poll_debug (int dn, drive_params * dp) {
    dp[dn].openloop_test = debug_read_command (dn, DOC_DBG_OL_EN);

    dp[dn].Id_Kp = debug_read_command (dn, DOC_DBG_I_PI_KP);
    dp[dn].Id_Ki = debug_read_command (dn, DOC_DBG_I_PI_KI);

    dp[dn].I_sat_limit = debug_read_command (dn, DOC_DBG_I_PI_FB_LIM);
    dp[dn].V_sat_limit = debug_read_command (dn, DOC_DBG_I_PI_OP_LIM);

    dp[dn].speed_Kp = debug_read_command (dn, DOC_DBG_SPEED_PI_KP);
    dp[dn].speed_Ki = debug_read_command (dn, DOC_DBG_SPEED_PI_KI);
    dp[dn].speed_limit = debug_read_command (dn, DOC_DBG_SPEED_PI_FB_LIM);
    dp[dn].speed_command = debug_read_command (dn, DOC_DBG_SPEED_SETP0) << SPEED_FRAC_BITS;
    //dp[dn].speed_command = debug_read_command (dn, DOC_DBG_SPEED_SETP1); //dummy
    dp[dn].pos_setpoint = debug_read_command (dn, DOC_DBG_POS_SETP0);

    int temp_pos_mode = debug_read_command (dn, DOC_DBG_POS_MODE);
    if (temp_pos_mode != dp[dn].enable_position_control) {
		//Reset the multi-turn position when switching to/from position mode
    	dp[dn].pos_temp = 0;
    }
    dp[dn].enable_position_control = temp_pos_mode;
    dp[dn].pos_limit = debug_read_command (dn, DOC_DBG_POS_SPEED) << SPEED_FRAC_BITS; //dummy
    dp[dn].pos_setpoint = debug_read_command (dn, DOC_DBG_POS_SETP0);
    dp[dn].pos_Kp = debug_read_command (dn, DOC_DBG_POS_PI_KP); //dummy
    dp[dn].pos_spdff_Kp = debug_read_command (dn, DOC_DBG_POS_SPDFF_KP); //dummy


}

/**
 * \addtogroup DEBUG_BUTTON
 *
 * @{
 */
/*
 * Get system console GUI soft button state from debug memory
 */
void debug_get_buttons(int offset, int num_buttons, int *buttons)
{
    int i;
    int val;
    unsigned int base_address = SYS_CONSOLE_DEBUG_RAM_BASE + offset;
    *buttons = 0;
    for (i=0;i<num_buttons;i++) {
        val = IORD_32DIRECT(base_address,i*4);
        *buttons = *buttons | ((val != 0) << i);
        IOWR_32DIRECT(base_address, i*4, 0);    //reset button value
    }
}

/*
 * @brief Return current soft button state
 */
int debug_button_pressed(int buttons, int button_num)
{
    return ((buttons & (1 << button_num))!= 0);
}
/*!
 * @}
 */

/*
 * Dump selected data to system console Tcl GUI
 */
void dump_data(drive_params * dp, int axis_select) {

    int dn = 0;


	static int dump_addr = 0;
	static int dump_mode = 0;

	int dump_trig_edge;
	int trigger_data_select;
	int trigger_value;
	int trigger_in = 0;
 	static int trigger_in_prev = 0;
	static int trigger_out = 0;
	static int trigger_out_prev = 0;

    //Trace occurs on selected axis in debug GUI
    dn = axis_select;

    //Update dump debug variables

    dump_mode = debug_read_command (0, DOC_DBG_DUMP_MODE);
    trigger_data_select = debug_read_command (0, DOC_DBG_TRIG_SEL);
    dump_trig_edge = debug_read_command (0, DOC_DBG_TRIG_EDGE);
    trigger_value = debug_read_command (0, DOC_DBG_TRIG_VALUE);


    if (trigger_out == 0) {
    	// Not already triggered
		switch (trigger_data_select) {
		case 0 :
			trigger_in = 0;
			trigger_out = 1; //Trigger always
			break;
		case 1 :
			trigger_in = dp[dn].Vu_PWM;
			break;
		case 2 :
			trigger_in = dp[dn].Vv_PWM;
			break;
		case 3 :
			trigger_in = dp[dn].Vw_PWM;
			break;
		case 4 :
			trigger_in = dp[dn].Iu;
			break;
		case 5 :
			trigger_in = dp[dn].Iw;
			break;
		case 6 :
			trigger_in = (0 - dp[dn].Iw - dp[dn].Iu);
			break;
		case 7 :
			trigger_in = dp[dn].speed_command_adjusted  >> SPEED_FRAC_BITS;
			break;
		case 8 :
			trigger_in = dp[dn].speed_encoder  >> SPEED_FRAC_BITS;
			break;
		case 9 :
			trigger_in = dp[dn].i_command_d;
			break;
		case 10 :
			trigger_in = dp[dn].Id;
			break;
		case 11 :
			trigger_in = dp[dn].i_command_q;
			break;
		case 12 :
			trigger_in = dp[dn].Iq;
			break;
		case 13 :
			trigger_in = dp[dn].pos_int;
			break;
		case 14 :
			trigger_in = dp[dn].pos_setpoint_adjusted;
			break;
		default :
			trigger_in = 0;
			trigger_out = 1; //Trigger always
			break;
		}

		switch (dump_trig_edge) {
		case 0 : //Equals
			if (trigger_in == trigger_value) {
				trigger_out = 1;
			}
			break;
		case 1 : //Rising edge
			if ((trigger_in > trigger_value)&&(trigger_in_prev <= trigger_value)) {
				trigger_out = 1;
			}
			break;
		case 2 :  //Falling edge
			if ((trigger_in < trigger_value)&&(trigger_in_prev >= trigger_value)) {
				trigger_out = 1;
			}
			break;
		case 3 : //Any Edge
			if (((trigger_in < trigger_value)&&(trigger_in_prev >= trigger_value))||
			((trigger_in > trigger_value)&&(trigger_in_prev <= trigger_value)))
			{
				trigger_out = 1;
			}
			break;
		default :
			trigger_out = 0;
			break;
		}
		trigger_in_prev = trigger_in;

		if (trigger_out == 1) {
			trigger_out_prev = 0;
		}

    }



    if (dp[dn].enable_drive == 0) {
        dump_addr = 0;
        dump_mode = 0;
        debug_write_status (0, DOC_DBG_DUMP_MODE, dump_mode);
    } else if ((dump_mode == 0) && (dump_addr < ((4096*16)-2))) {
        if (trigger_out) {

			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Vu_PWM);  //V_u
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Vv_PWM);  //V_v
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Vw_PWM);  //V_w
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Iu);  //I_u
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Iw);  //I_w
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, 0 - dp[dn].Iw - dp[dn].Iu);  //I_v
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].speed_command_adjusted);  //
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].speed_encoder);  //
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Id);  //
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].i_command_d);  //
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].Iq);  //
			dump_addr+=2;
			IOWR_16DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].i_command_q);  //
			dump_addr+=2;

			IOWR_32DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].pos_int);  //
			dump_addr+=4;
			IOWR_32DIRECT(SVM_DUMP_MEM_BASE, dump_addr, dp[dn].pos_setpoint_adjusted);  //
			dump_addr+=4;
#ifdef __NIOS__
		}
    } else {
        //stop dumping
        dump_mode = 1;
        debug_write_status (0, DOC_DBG_DUMP_MODE, dump_mode);
        dump_addr = 0;
        trigger_out = 0; //reset trigger
#endif
    }


}

void init_debug_output(void) {
	dbg_level = DBG_DEFAULT;
}

int debug_printf(unsigned int priority, char *format, ...) {
	va_list args;

	if (priority <= dbg_level) {
		va_start (args, format);
		vprintf(format, args);
		va_end(args);
	}
	return 0;
}

/**
 * @brief Task to manage buffer of debug output messages
 *
 * @param pdata not used
 */

/*!
 * @}
 */


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

#ifndef __MC_H_
#define __MC_H_

/**
 * @file mc.h
 *
 * @brief Header file for motor control demo
 */

#include "includes.h"

/*!
 * \addtogroup DEMOCONFIG Drive On Chip Motor Control configuration
 *
 * @brief Macros to configure the operation of the motor control reference design
 *
 * A number of macros are used to define the operation of the motor control demo.
 *
 * Additionally, the macros __NIOS__, __DOC_SOC_VXWORKS__ and __DOC_SOC_UCOSII__ are defined
 * for the Nios II HAL, VxWorks and uC/OS-II builds, respectively to control inclusion or
 * exclusion of code at compile time. The se macros are defined in the includes.h file for
 * each build.
 *
 * @{
 */

/** @name Axis Enable
 *
 * Set these defines to control how many axes are actually resent (i.e. how
 * many motors are connected) on the multi-axis power board
 *
 * @{
 */
#define FIRST_MULTI_AXIS 0	//!< First active axis on multi-axis platform
#define LAST_MULTI_AXIS  0	//!< Last active axis on multi-axis platform
/*!
 * @}
 */

/** @name Loopback
 *
 * Set one of these defines to override the default encoder type.
 *
 * @{
 */
//#define OVERRIDE_ENCODER SYSID_ENCODER_ENDAT
//#define OVERRIDE_ENCODER SYSID_ENCODER_BISS
/*!
 * @}
 */

/** @name Loopback
 *
 * Set this define if no power board is connected. Must also build FPGA image with NO_POWERBOARD defined if not
 * using a FalconEye powerboard.
 *
 * @{
 */
//#define LOOPBACK
/*!
 * @}
 */

/** @name Open Loop
 *
 * Set this define non zero for open loop control mode.
 *
 * @{
 */
#define OPEN_LOOP_INIT 1
/*!
 * @}
 */

/*!
 * @}
 */

/*!
 * \addtogroup MC Drive On Chip Motor Control
 *
 * @{
 */

/**
 * @brief Enumeration to control numeric processing
 *
 */
//kmtypedef enum {
	//kmSOFT_FIXP,		//!< Software fixed point
	//km	DSPBA_FIXP,		//!< Hardware fixed point
	//km	DSPBA_FLOATP,	//!< Hardware floating point
	//km	SOFT_FLOATP	//!< Software floating point
//km} calc_type_e;

/*!
 * \addtogroup MCDEFINES Motor Control Defines
 *
 * @{
 */


//Defines how many fractional bits are used for the internal representation of speed/rpm of the motor
//Design has only been tested with 4 and 0. If this values is changed then the equivalent value in
//the System Console doc_debug_gui.tcl script must be changed for the speed graphs to be scaled correctly.
/**
 * Defines how many fractional bits are used for the internal representation of speed/rpm of the motor
 * Design has only been tested with 4 and 0. If this values is changed then the equivalent value in
 * the System Console doc_debug_gui.tcl script must be changed for the speed graphs to be scaled correctly.
 */
#define SPEED_FRAC_BITS 4

//################################################################################################
// Enable encoder offset calibration
//################################################################################################
//#define ENCODER_SERVICE

// Default debug output level
#define DBG_DEFAULT DBG_INFO

#define PWMMAX       3125                   //!< 50 MHz -> 8 kHz IGBT switching frequency

#define word                    0xFFFF
#define dword                   0xFFFFFFFF
#define Bit23                   0x7fffff
#define Bit22                   0x3fffff

#define Bit16                   0x00ffff
#define Bit15                   0x007fff

/*!
 * @}
 */

extern char console_output;

extern unsigned int irq_counter;

typedef struct
{

	unsigned int     DOC_ADC_BASE_ADDR;
	unsigned int     DOC_ENDAT_BASE_ADDR;
	unsigned int     DOC_BISS_BASE_ADDR;
	unsigned int     DOC_PWM_BASE_ADDR;
	unsigned int     DOC_SM_BASE_ADDR;

	//################################################################################################
	//Variables of Iq and Id Controls
	//################################################################################################

	short I_sat_limit;				//!<Current saturation limit
	short V_sat_limit;				//!<Voltage saturation limit

	int     Id;						//!<Id - Direct Current
	int     Iq;						//!<Iq - Quadrature Current

	short   Iq_Kp;					//!<Iq proportional gain constant
	short   Iq_Ki;					//!<Iq integral gain constant

	short   Id_Kp;					//!<Id proportional gain constant
	short   Id_Ki;					//!<Id integral gain constant

	int     Valpha;					//!<Alpha Voltage
	int     Vbeta;					//!<Beta Voltage

	int     Vd_prop;				//!<Vd proportional gain variable
	int     Vd_integ;				//!<Vd integral gain variable
	int     Vd;          			//!<Vd output

	int     Vq_prop;				//!<Vq proportional gain variable
	int     Vq_integ;				//!<Vq integral gain variable
	int     Vq;						//!<Vq output

	int     Ialpha;					//!<Alpha Current
	int     Ibeta;					//!<Beta Current

	short   Iu;	                   	//!<Iu (feedback current input)
	short   Iw;						//!<Iw (feedback current input)

	int     Vu_PWM;					//!<Vu (U voltage setting for PWM)
	int     Vw_PWM;					//!<Vw (W voltage setting for PWM)
	int     Vv_PWM;					//!<Vv (V voltage setting for PWM)

	char    mpoles;    				//!<Set motor pole pairs
	short   mphase;					//!<Phase offset
	int     i_command_d;			//!<Direct Current Command
	int     i_command_q;			//!<Quadrature Current Command

	//Define PI controller structures
	pi_instance_q15 Id_PI;      	//!<FOC Direct Current PI controller
	pi_instance_q15 Iq_PI;      	//!<FOC Quadrature Current PI controller
	pi_instance_q15 Speed_PI;   	//!<Speed PI controller
	pi_instance_q15 Position_PI;	//!<Position PI controller

	//################################################################################################
	//Variables of Iq and Id Controls (For Floating Point)
	//################################################################################################

	float   Id_f;					//!<Id - Direct Current (floating point)
	float   Iq_f;					//!<Iq - Quadrature Current (floating point)

	float   Ialpha_f;				//!<Alpha Current (floating point)
	float   Ibeta_f;				//!<Beta Current (floating point)

	float   Iu_f;	                //!<Iu (feedback current input) (floating point)
	float   Iw_f;					//!<Iw (feedback current input) (floating point)

	float   Valpha_f;				//!<Alpha Voltage (floating point)
	float   Vbeta_f;				//!<Beta Voltage (floating point)

	float   Vd_f;          			//!<Vd output (floating point)
	float   Vq_f;					//!<Vq output (floating point)

	float   sin_phi_f;	           	//!<Sin Phi Electrical (floating point)
	float   cos_phi_f;            	//!<Cos Phi Electrical (floating point)
	
	//Define Floating Point PI controller structures
 	pi_instance_f Id_PI_f;      	//!<FOC Direct Current PI Controller (floating point)
 	pi_instance_f Iq_PI_f;      	//!<FOC Quadrature Current PI Controller (floating point)
 	pi_instance_f Speed_PI_f;   	//!<Speed PI Controller (floating point)
	pi_instance_f Position_PI_f;	//!<Position PI Controller (floating point)

	//################################################################################################
	//Angle variables
	//################################################################################################
	int             phi_mech;       //!<Mechanical Angle
	int             phi_mech_prev;	//!<Mechanical Angle
	int             phi;			//!<Angle
	unsigned short  phi_elec;		//!<Electrical Angle
	int             sin_phi_q15;	//!<Sin Phi Electrical
	int             cos_phi_q15;    //!<Cos Phi Electrical

	//################################################################################################
	//Current offset calculation variables
	//################################################################################################
	int     Offset_start_calc;
	int     Offset_U;
	int     Offset_W;
	short   Offset_err;

	//################################################################################################
	//Speed variables
	//################################################################################################
	//Variables of speed calculations
	//################################################################################################
	int     speed_encoder;			//!<Actual measured Speed
	int     speed_resolver;			//!<Unused


	//################################################################################################
	//Encoder Variables
	//################################################################################################
	short   enable_drive;			//!<Enables the PWM output
	short   enable_position_control;//!<Enables position control

	//################################################################################################
	//EnDat variables
	//################################################################################################
	int        encoder_version;			//!<Encoder version number
	int        encoder_length;			//!<Number of encoder bits
	int        encoder_mask;			//!<Bit mask for encoder bits
	int        encoder_turns_mask;		//!<Bit mask for encoder multi-turn bits
	int        encoder_multiturn;		//!<If encoder is multi-turn. value = 1 else 0
	int        encoder_multiturn_bits;	//!<Number of encoder multi-turn bits
	int        encoder_singleturn_bits;	//!<Number of encoder multi-turn bits
	int        st_shift_phi_m;
	int        enc_data;				//!<Encoder single & multi-turn data (current value)
	int        enc_data_prev;			//!<Encoder single & multi-turn data (previous value)
	int        enc_turns;				//!<Encoder multi-turn data

	int        position_encoder;		//!<Position value derived from encoder including any multi-turn info
	int        position_setpoint;		//!<Position setpoint (desired position)

	int        sensor_warning_bits;
	int        sensor_error_bits;

	//################################################################################################
	//Variables of speed control
	//################################################################################################
	int    speed_Kp;					//!<Kp_speed proportional parameter Speed Control
	int    speed_Ki;					//!<Ki_speed integral    parameter Speed Control
	int    speed_limit; 				//!<Speed is limited to this value
	int    speed_command;				//!<Speed request/command
	int    speed_command_adjusted;		//!<Speed request/command after adjustment
	int    pos_setpoint;				//!<Position setpoint
	int    pos_setpoint_adjusted;		//!<Position setpoint after adjustment
	int    pos_Kp;						//!<Position PI controller Kp proportional gain
    int    pos_spdff_Kp;//  ;
	int    pos_err_limit;				//!<Position PI controller error limit
	int    pos_limit;					//!<Position PI controller output limit

	//################################################################################################
	//Variables of position control
	//################################################################################################
    int    pos_temp;					//!<Position control variable
	int    enc_data_old;				//!<Position control variable
	int    pos_int;						//!<Position control variable
	int    count_pos;					//!<Position control variable
	int    sec;							//!<Position control variable
	int    pos_simulation;				//!<Position control variable
	int    stepsize;					//!<Position control variable

	//################################################################################################
	//Control variables
	//################################################################################################

	unsigned short status_word;
	unsigned short state_act, state_act_old;

	int openloop_test;

	int reset_control;							//!<Resets the FOC control algorithm (PI control & Filter)

} drive_params;

void init_dp(drive_params * dp);
void drive_irq(void* context);
void * get_dp(void);

/*!
 * @}
 */

#endif /* __MC_H_ */

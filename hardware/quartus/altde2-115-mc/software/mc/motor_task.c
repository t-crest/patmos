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

platform_t platform;	//km


/**
 * Initialise the supported families for this demo
 */
static const device_family_t supported_families [] = {{SYSID_CIV_DE2115, "Cyclone IV DE2115"}};	//km

/**
 * Initialise the supported powerboards for this demo
 */
static const powerboard_t supported_powerboards [] = {{SYSID_PB_ALT12_MULTIAXIS,  "Altera ALT12 Multi-axis", 4, FIRST_MULTI_AXIS, LAST_MULTI_AXIS, SYSID_ENCODER_BISS,  1<<SYSID_CIV_DE2115 | 1<<SYSID_CVE | 1<<SYSID_CVSX, 350, 450 }};	//km

/**
 * Initialise the supported encoder types for this demo
 */
//static const encoder_t supported_encoders [] = {{SYSID_ENCODER_BISS,  "BiSS",  Biss_Init,  Biss_Service,  Biss_Read_Position }};	//km
/*
 * Determine what hardware we are running on based on the FPGA SYSID component.
 */
int decode_sysid(unsigned int sysid_base_addr) {		//km
	int sys_id_errors = 0;
	int i;

//km	int device_family_bit = 0;
	int sys_id = IORD_32DIRECT(sysid_base_addr,0);
	

	//int sys_id = *((volatile int _IODEV *)sysid_base_addr);
	//int sys_id = *((volatile alt_u32 _IODEV *)( ((void _IODEV *)(((alt_u8*)sysid_base_addr) + (0)))));
	//asm volatile (" ");

	debug_printf(DBG_ALWAYS, "[DECODE SYSID] Decoding hardware platform from QSYS SYSID data : 0x%08X \n", sys_id);
	debug_printf(DBG_DEBUG, "[DECODE SYSID] SYSID_VERSION_MAJOR : 0x%02X \n", SYSID_VERSION_MAJOR(sys_id));
	debug_printf(DBG_DEBUG, "[DECODE SYSID] SYSID_VERSION_MINOR : 0x%02X \n", SYSID_VERSION_MINOR(sys_id));
	debug_printf(DBG_DEBUG, "[DECODE SYSID] SYSID_POWERBOARD_ID : 0x%02X \n", SYSID_POWERBOARD_ID(sys_id));
	debug_printf(DBG_DEBUG, "[DECODE SYSID] SYSID_DEVICE_FAMILY : 0x%02X \n", SYSID_DEVICE_FAMILY(sys_id));
	debug_printf(DBG_DEBUG, "[DECODE SYSID] SYSID_DESIGN_ID     : 0x%02X \n", SYSID_DESIGN_ID(sys_id));

	platform.version_major = SYSID_VERSION_MAJOR(sys_id);
	platform.version_minor = SYSID_VERSION_MINOR(sys_id);

	platform.design_id = SYSID_DESIGN_ID(sys_id);
	if (platform.design_id != 0xFE) {
		//design_id magic number should = 0xFE
		debug_printf(DBG_FATAL, "[DECODE SYSID] ERROR: Expected 0xFE in lower byte of SYSID data but received 0x%02X - check FPGA IMAGE!! \n", platform.design_id);
		sys_id_errors++;
	}

	debug_printf(DBG_ALWAYS, "[DECODE SYSID] Design Version : %d.%d\n", platform.version_major, platform.version_minor);

	// Decode the device family
//km	platform.device_family = NULL;
//km	for (i = 0; i < sizeof(supported_families)/sizeof(device_family_t); i++) {
//km		if (supported_families[i].sysid == SYSID_DEVICE_FAMILY(sys_id)) {
		platform.device_family = (device_family_t *)&supported_families[0];
//km			platform.device_family = (device_family_t *)&supported_families[i];
//km			device_family_bit = 1<<SYSID_DEVICE_FAMILY(sys_id);
//km		}
//km	}
//km	if (platform.device_family == NULL) {
//km		sys_id_errors++;
//km		debug_printf(DBG_FATAL, "[DECODE SYSID] ERROR: Unsupported device family\n");
//km	} else {
		debug_printf(DBG_ALWAYS, "[DECODE SYSID] FPGA Board     : %s\n", platform.device_family->name);
//km	}

	// Decode the powerboard type
//km	platform.powerboard = NULL;
//km	for (i = 0; i < sizeof(supported_powerboards)/sizeof(powerboard_t); i++) {
//km		if (supported_powerboards[i].sysid == SYSID_POWERBOARD_ID(sys_id)) {
		platform.powerboard = (powerboard_t *)&supported_powerboards[0];	//km
//km		platform.powerboard = (powerboard_t *)&supported_powerboards[i];
//km		}
//km	}

	debug_printf(DBG_ALWAYS, "[DECODE SYSID] Power Board    : %s\n", platform.powerboard->name);


	// Decode the encoder type
//km 	platform.encoder = NULL;
//km 	for (i = 0; i < sizeof(supported_encoders)/sizeof(encoder_t); i++) {
//		platform.encoder = (encoder_t *)&supported_encoders[0];
//km		platform.encoder = (encoder_t *)&supported_encoders[i];
//km		debug_printf(DBG_ALWAYS, "[DECODE SYSID] Encoder Type No.   : %d\n", i);	//km
//km 	}

//	debug_printf(DBG_ALWAYS, "[DECODE SYSID] Encoder Type   : %s\n", platform.encoder->name);

	debug_printf(DBG_ALWAYS, "[DECODE SYSID] %d axes available\n", platform.powerboard->axes);
	if ((platform.powerboard->first_axis <= (platform.powerboard->axes - 1))
			&& (platform.powerboard->last_axis <= (platform.powerboard->axes - 1))
			&& (platform.powerboard->first_axis <= platform.powerboard->last_axis)
			&& ((platform.powerboard->last_axis - platform.powerboard->first_axis) <= (platform.powerboard->axes - 1))) {
		for (i = 0; i < platform.powerboard->axes; i++) {
			debug_printf(DBG_ALWAYS, "[DECODE SYSID] Axis %d         : %s\n",
					i, (platform.powerboard->first_axis <= i) && (i <= platform.powerboard->last_axis) ? "Enabled" : "Disabled");
		}
		platform.first_drive = platform.powerboard->first_axis;
		platform.last_drive = platform.powerboard->last_axis;
		//debug_printf(DBG_ALWAYS, "[DECODE SYSID] First Drive %d : Last Drive %d\n", platform.first_drive, platform.last_drive);

	}
	else {
		sys_id_errors++;
		debug_printf(DBG_FATAL, "[DECODE SYSID] ERROR: Invalid axis selection\n");
	}

	if (sys_id_errors > 0) {
		debug_printf(DBG_FATAL, "[DECODE SYSID] ERROR: Unexpected data detected in SYS_ID - check FPGA IMAGE and software configuration!! \n");
	}
	else {
		debug_printf(DBG_DEBUG, "[DECODE SYSID] SYS_ID data successfully decoded\n");
	}

	if (sys_id_errors > 0) {
		debug_printf(DBG_FATAL, "[DECODE SYSID] ERRORS DETECTED: Fix ERROR messages above before continuing. Program stopped! \n\n");
	}
	return sys_id_errors;
}





/**
 * @file motor_task.c
 *
 * @brief Main motor control task and ISR
 */

/*!
 * \addtogroup MC
 *
 * @{
 */

//
// On Nios the ISR is linked in tightly coupled memory
//
void drive_irq(void)  __attribute__((naked)); // TODO: IRQ_SECTION; __attribute__((naked)) __attribute__((section(".text.spm")))

#define SPEED_SHIFT_INT 0
//#define LOOPBACK
#define OFFSET_ACCUM_ISR_COUNT  256
#define CURRENT_OFFSET_LIMIT    400

static drive_params dp[4]; // TODO: DRIVE_SECTION;

static int axis_select = 0;
static int latency = 0;
//km static calc_type_e enable_dspba = SOFT_FIXP;

static short dc_link_voltage = 0 ;            // DC Link Value
static short dc_link_current = 0 ;
static int runtime =   0 ;
static int cnt_IRQ =   0 ;
unsigned int irq_counter = 0;

void * get_dp(void) {
	return (void *)dp;
}


/**
 * Used during startup to determine ADC zero offsets which can then be used to
 * correct ADC readings in control loop.
 *
 * @param start		set to 1 to start offset accumulation
 * @param Offset_U	U phase offset accumulator
 * @param Offset_W	W phase offset accumulator
 * @param Iu		U phase current reading
 * @param Iw		W phase current reading
 */
static void inline adc_offset_accumulate(int * start, int * Offset_U, int * Offset_W, short Iu, short Iw) {
    if (*start > 0) {
        if (*start < (OFFSET_ACCUM_ISR_COUNT + 1)){
            *Offset_U +=  Iu ;
            *Offset_W +=  Iw ;
            (*start)++;
        } else {
            *start = 0;
        }
    }
}


/**
 * @brief Helper function for position control
 *
 * @param Position_PI
 * @param enc_data
 * @param enc_data_old
 * @param pos_temp
 * @param pos_int
 * @param pos_setpoint
 * @param pos_limit
 * @param speed_command
 * @param encoder_singleturn_bits
 */
static void position_control( pi_instance_q15 * Position_PI, int enc_data,  int * enc_data_old,
									 int * pos_temp,  int * pos_int, int pos_setpoint, int pos_limit,
									 int * speed_command, int encoder_singleturn_bits){
    int  delta_phi  ;
    int  shifted_enc_data = enc_data << (23-encoder_singleturn_bits);

    delta_phi = (shifted_enc_data - *enc_data_old);
    if((delta_phi) > Bit22){
        *pos_temp -= Bit23;
    }else if((delta_phi) < -Bit22){
        *pos_temp += Bit23;
    }
    *enc_data_old = shifted_enc_data;
    *pos_int      = (shifted_enc_data >> 7) + (*pos_temp >>7);

    Position_PI->feedback = *pos_int;
    Position_PI->setpoint = pos_setpoint;
    PI_control_q15(Position_PI,0);  //Direct current control
    * speed_command = (short)Position_PI->output;
}


/**
 * @brief Sample encoder inputs
 *
 * Helper function to read motor position and current for a single axis
 *
 * @param dp	Pointer to axis data structure
 */
static void sample_inputs(drive_params * dp, int dn) {
	// Read motor position from encoder
	platform.encoder->encoder_read_position_fn(dp);
	dp->phi_elec = PHI_ELECTRICAL(dp->mpoles, dp->phi_mech, dp->mphase);

	if (dp->enable_position_control) {

		dp->pos_setpoint_adjusted = dp->pos_setpoint;
		position_control(&dp->Position_PI,  dp->enc_data, &dp->enc_data_old, &dp->pos_temp, &dp->pos_int, dp->pos_setpoint, dp->pos_limit, &dp->speed_command,dp->encoder_singleturn_bits);

		dp->speed_command_adjusted = dp->speed_command + ((dp->speed_encoder * dp->pos_spdff_Kp)/256);
	} else {

		//position control NOT used but still calculates position values
		position_control(&dp->Position_PI,  dp->enc_data, &dp->enc_data_old, &dp->pos_temp, &dp->pos_int, dp->pos_setpoint_adjusted, dp->pos_limit, &dp->speed_command_adjusted,dp->encoder_singleturn_bits);
		//Set setpoint to actual measured position (not used but makes system console plot look better)
		dp->pos_setpoint_adjusted = dp->pos_int;
		dp->speed_command_adjusted = ABS_MAX(dp->speed_command, 3000 << SPEED_FRAC_BITS);
	}
	// Speed Control
	//dp->Speed_PI.feedback = ABS_MAX(dp->speed_encoder + dp->cmd_wave_test,3000);
	dp->Speed_PI.feedback = dp->speed_encoder;
	dp->Speed_PI.setpoint = dp->speed_command_adjusted;
	PI_control_q15(& dp->Speed_PI, dp->reset_control);  //Direct current control
	dp->i_command_q =  dp->Speed_PI.output;

	if ( dp->enable_drive == 0 ){
		dp->i_command_q = 0 ;
	}
    // Read motor U & W phase currents from ADC
	adc_read(dp->DOC_ADC_BASE_ADDR, &dp->Iu, &dp->Iw) ;

    // Accumulate ADC readings for offset calibration
	adc_offset_accumulate((int *)(&dp->Offset_start_calc), &dp->Offset_U, &dp->Offset_W, dp->Iu, dp->Iw);
}

/**
 * @brief process input data
 *
 * Helper function for ISR to process input data
 *
 * @param dp	Pointer to axis data structure
 */
static void process(drive_params * dp) {
	int dn = 0;

	for (dn = platform.first_drive; dn <= platform.last_drive; dn++) {
//km		if (enable_dspba == SOFT_FIXP) {
			// Software fixed point
//			PERF_BEGIN (PERFORMANCE_COUNTER_0_BASE, 2);
			dp[dn].sin_phi_q15 = SINE(dp[dn].phi_elec);
			dp[dn].cos_phi_q15 = COSINE(dp[dn].phi_elec);
			clark_transform_q10(dp[dn].Iu, dp[dn].Iw, &dp[dn].Ialpha, &dp[dn].Ibeta) ;
			park_transformation_q10(dp[dn].Ialpha, dp[dn].Ibeta, &dp[dn].Id, &dp[dn].Iq, dp[dn].sin_phi_q15, dp[dn].cos_phi_q15) ;
			dp[dn].Id_PI.feedback = dp[dn].Id;
			dp[dn].Id_PI.setpoint = dp[dn].i_command_d;
			PI_control_q15(&dp[dn].Id_PI, ((dp[dn].enable_drive == 0)|(dp[dn].reset_control == 1)) );  //Direct current control
			dp[dn].Iq_PI.feedback = dp[dn].Iq;
			dp[dn].Iq_PI.setpoint = dp[dn].i_command_q;
			PI_control_q15(&dp[dn].Iq_PI,((dp[dn].enable_drive == 0)|(dp[dn].reset_control == 1)) );  //Quadrature current control
			dp[dn].Vd = dp[dn].Id_PI.output;
			dp[dn].Vq = dp[dn].Iq_PI.output;
			inverse_park_q10(dp[dn].Vd,dp[dn].Vq,&dp[dn].Valpha,&dp[dn].Vbeta,dp[dn].sin_phi_q15,dp[dn].cos_phi_q15) ;
//			PERF_END (PERFORMANCE_COUNTER_0_BASE, 2);
//km		} else {
			//km			// Software floating point
			//km			PERF_BEGIN (PERFORMANCE_COUNTER_0_BASE, 2);
			//km			dp[dn].sin_phi_f = ((float)SINE(dp[dn].phi_elec) * SHIFTR15FLOAT);
			//km			dp[dn].cos_phi_f = ((float)COSINE(dp[dn].phi_elec) * SHIFTR15FLOAT);
//	                dp[dn].sin_phi_f = sinf((float)dp[dn].phi_elec * 2 * 3.1415926 * SHIFTR15FLOAT);
//	                dp[dn].cos_phi_f = cosf((float)dp[dn].phi_elec * 2 * 3.1415926 * SHIFTR15FLOAT);
			//km			dp[dn].Iu_f = ((float)dp[dn].Iu * SHIFTR10FLOAT);
			//km			dp[dn].Iw_f = ((float)dp[dn].Iw * SHIFTR10FLOAT);
			//km			clark_transform_f(dp[dn].Iu_f, dp[dn].Iw_f, &dp[dn].Ialpha_f, &dp[dn].Ibeta_f) ;
			//km			park_transformation_f(dp[dn].Ialpha_f,dp[dn].Ibeta_f,&dp[dn].Id_f,&dp[dn].Iq_f,dp[dn].sin_phi_f,dp[dn].cos_phi_f) ;
			//km			dp[dn].Id_PI_f.feedback = dp[dn].Id_f;
			//km			dp[dn].Id_PI_f.setpoint = ((float)dp[dn].i_command_d * SHIFTR10FLOAT);
			//km			PI_control_f(&dp[dn].Id_PI_f,((dp[dn].enable_drive == 0)|(dp[dn].reset_control == 1)) );  //Direct current control
			//km			dp[dn].Iq_PI_f.feedback = dp[dn].Iq_f;
			//km			dp[dn].Iq_PI_f.setpoint = ((float)dp[dn].i_command_q * SHIFTR10FLOAT);
			//km			PI_control_f(&dp[dn].Iq_PI_f,((dp[dn].enable_drive == 0)|(dp[dn].reset_control == 1)) );  //Quadrature current control
			//km			dp[dn].Vd_f = dp[dn].Id_PI_f.output;
			//km			dp[dn].Vq_f = dp[dn].Iq_PI_f.output;

			//km			dp[dn].Vq = (int)(dp[dn].Vq_f * SHIFTL10FLOAT);
			//km			dp[dn].Vd = (int)(dp[dn].Vd_f * SHIFTL10FLOAT);
			//km			dp[dn].Iq = (int)(dp[dn].Iq_f * SHIFTL10FLOAT);
			//km			dp[dn].Id = (int)(dp[dn].Id_f * SHIFTL10FLOAT);


			//km			inverse_park_f(dp[dn].Vd_f,dp[dn].Vq_f,&dp[dn].Valpha_f,&dp[dn].Vbeta_f,dp[dn].sin_phi_f,dp[dn].cos_phi_f) ;
			//km			PERF_END (PERFORMANCE_COUNTER_0_BASE, 2);

			//km			dp[dn].Valpha = (int) ( dp[dn].Valpha_f * SHIFTL10FLOAT);
			//km			dp[dn].Vbeta = (int) ( dp[dn].Vbeta_f  * SHIFTL10FLOAT);
//km			}

		// Space vector modulation for outputs
		svm(PWMMAX, dp[dn].Valpha, dp[dn].Vbeta, & dp[dn].Vu_PWM, & dp[dn].Vv_PWM, & dp[dn].Vw_PWM);
		// Write new PWM values to hardware ready for next cycle
		if ( dp[dn].enable_drive == 1 ){
			pwm_update(dp[dn].DOC_PWM_BASE_ADDR, dp[dn].Vu_PWM, dp[dn].Vv_PWM, dp[dn].Vw_PWM);
		} else {
			// Disabled axes set stationary by setting PWM to midpoint
			pwm_update(dp[dn].DOC_PWM_BASE_ADDR, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1);
		}
//km		if (dp[dn].reset_control == 1) {
//				debug_printf(DBG_INFO, "---> Axis %d: Reset Control\n",dn);
//km		}
		dp[dn].reset_control = 0;
	}
}


/**
 * @brief Interrupt Service Routine for motor control
 *
 * All of the critical processing occurs here. The IRQ occurs every PWM cycle, which is every 62.5us in the
 * stamdard implementation, as a result of the ADC conversion completion interrupt.
 *
 * It is assumed that the hardware design is such that the position encoder reading is also
 * available.
 *
 * Processing must complete in time for the new PWM value to be written to the hardware before
 * the next cycle starts.
 *
*/
void drive_irq(void) {
	exc_prologue();
	int dn = 0;
	//km	int dspba_foc_base = 0;


    //PERF_BEGIN (PERFORMANCE_COUNTER_0_BASE, 1);
	
	irq_counter++;

	// Disable channel 0 ADC IRQ
    IOWR_16DIRECT(dp[0].DOC_ADC_BASE_ADDR, ADC_IRQ_ACK,1);
/*
	if (dp[dn].openloop_test == 0) {
		// Closed loop control
		for (dn = platform.first_drive; dn <= platform.last_drive; dn++){
			sample_inputs(&dp[dn],dn);
		}

//km Only SOFT_FIXP is kept, other modes are outcommeded
		//km		if ((enable_dspba == SOFT_FIXP)||(enable_dspba == SOFT_FLOATP)) {
			// Software fixed or floating point:
			//		park and clark transforms on input readings
			//		PI control loop
			//		inverse transform on outputs
			process(dp);
			//km		}
		//km		else {
			// Use DSPBA hardware for fixed or floating point

			//km			dspba_foc_base = (enable_dspba == DSPBA_FIXP) ? FOC_FIXED_POINT_BASE : FOC_FLOATING_POINT_BASE;

			//km			PERF_BEGIN (PERFORMANCE_COUNTER_0_BASE, 2);

			// Optimized DSPBA access - load all 4 channels and wait
			//km			for (dn = platform.first_drive; dn <= platform.last_drive; dn++){
				//km				IOWR_32DIRECT(dspba_foc_base,axis_in_CHANNEL, dn);

				//Load input data
				//km				IOWR_32DIRECT(dspba_foc_base,Iu_INPUT, dp[dn].Iu);
				//km				IOWR_32DIRECT(dspba_foc_base,Iw_INPUT, dp[dn].Iw);
				//km				IOWR_32DIRECT(dspba_foc_base,phi_el_INPUT, dp[dn].phi_elec);
				//km				IOWR_32DIRECT(dspba_foc_base,reset_INPUT, ((dp[dn].enable_drive == 0)|(dp[dn].reset_control == 1)));
				//km				IOWR_32DIRECT(dspba_foc_base,torque_INPUT, dp[dn].i_command_q);

				//Load start bit
				//km				IOWR_32DIRECT(dspba_foc_base,DSPBA_START,1);

				//Read start bit - wait until it changes from last time
				//km				int j=0;
				//km				int status_out = 1;
				//km				while ((status_out != 0) & (j<100)) {
					//km					status_out = IORD_32DIRECT(dspba_foc_base, DSPBA_START);
					//km					j++;
					//km				}
				//km			}
			//Read finished bit - wait until it changes from last time
			//km			int j=0;
			//km			int status_out = 1;
			//km			while ((status_out != 0) & (j<100)) {
				//km			  status_out = IORD_32DIRECT(dspba_foc_base, DSPBA_BUSY);
			  //km			  j++;
			  //km			}

			//Read integer output data from each axis
			//km			PERF_END (PERFORMANCE_COUNTER_0_BASE, 2);

			//km			for (dn = platform.first_drive; dn <= platform.last_drive; dn++){
				//km				IOWR_32DIRECT(dspba_foc_base, axis_in_CHANNEL, dn);

				//km				if (enable_dspba == DSPBA_FIXP) {

					//km					dp[dn].Valpha =  IORD_32DIRECT(dspba_foc_base, Valpha_OUTPUT );
					//km					dp[dn].Vbeta  =  IORD_32DIRECT(dspba_foc_base, Vbeta_OUTPUT );
					//km				}
				//km				else {
					//km					dp[dn].Valpha =  IORD_32DIRECT(dspba_foc_base, Valpha_OUTPUT );
					//km					dp[dn].Vbeta  =  IORD_32DIRECT(dspba_foc_base, Vbeta_OUTPUT );

					//km				}
				// Space vector modulation for outputs
				//km				svm(PWMMAX, dp[dn].Valpha, dp[dn].Vbeta, & dp[dn].Vu_PWM, & dp[dn].Vv_PWM, & dp[dn].Vw_PWM);
				// Write new PWM values to hardware ready for next cycle
				//km				if ( dp[dn].enable_drive == 1 ){
					//km					pwm_update(dp[dn].DOC_PWM_BASE_ADDR, dp[dn].Vu_PWM, dp[dn].Vv_PWM, dp[dn].Vw_PWM);
					//km				} else {
					// Disabled axes set stationary by setting PWM to midpoint
					//km					pwm_update(dp[dn].DOC_PWM_BASE_ADDR, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1);
					//km				}
				//km				if (dp[dn].reset_control == 1) {
//					debug_printf(DBG_INFO, "---> Axis %d: DSPBA Reset Control\n",dn);
					//km				}
				//km				dp[dn].reset_control = 0;
				//km			}
			//km		}
	}
	else {

		*/
		// Open loop mode
		static unsigned int idx = 0;
		unsigned short x1 ;
		// Create fixed frequency SVM to turn motor without current or position feedback
		idx = idx+150;
		x1  = idx>>2;

        for (dn = platform.first_drive; dn <= platform.last_drive; dn++){
			// Read motor position from encoder
    //RBS 	platform.encoder->encoder_read_position_fn(&dp[dn]);
    //RBS   dp[dn].phi_elec = PHI_ELECTRICAL(dp[dn].mpoles, dp[dn].phi_mech, dp[dn].mphase);
    //RBS   position_control(&dp[dn].Position_PI,  dp[dn].enc_data, &dp[dn].enc_data_old, &dp[dn].pos_temp, &dp[dn].pos_int, dp[dn].pos_setpoint, dp[dn].pos_limit, &dp[dn].speed_command,dp[dn].encoder_singleturn_bits);
            // Position not used - for monitoring only

			// Read motor U & W phase currents from ADC
            adc_read(dp[dn].DOC_ADC_BASE_ADDR, &dp[dn].Iu, &dp[dn].Iw) ;
            adc_offset_accumulate((int *)(&dp[dn].Offset_start_calc), &dp[dn].Offset_U, &dp[dn].Offset_W, dp[dn].Iu, dp[dn].Iw);
            clark_transform_q10(dp[dn].Iu,dp[dn].Iw,&dp[dn].Ialpha,&dp[dn].Ibeta) ;
            // Ialpha & Ibeta not used - for monitoring only

            dp[dn].Vbeta = SINE(x1)>>8;
            dp[dn].Valpha = COSINE(x1)>>8;
			// Space vector modulation for outputs
			svm(PWMMAX, dp[dn].Valpha, dp[dn].Vbeta, &dp[dn].Vu_PWM, &dp[dn].Vv_PWM, &dp[dn].Vw_PWM);
			// Write new PWM values to hardware ready for next cycle
            if (dp[dn].enable_drive == 1){
				pwm_update(dp[dn].DOC_PWM_BASE_ADDR, dp[dn].Vu_PWM, dp[dn].Vv_PWM, dp[dn].Vw_PWM);
			} else {
				// Disabled axes set stationary by setting PWM to midpoint
				pwm_update(dp[dn].DOC_PWM_BASE_ADDR, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1, (PWMMAX+1)/2-1);
			}
		} // end for dn
//	}; // end of openloop mode code

	dn = 0;
	dn = axis_select;


	// Dump diagnostic data for system console GUI
//	dump_data(dp, axis_select);



	// Time critical processing is complete. Set semaphore to wake drive task


    // Count up seconds - 16kHz IRQ rate
    cnt_IRQ++;
    if(cnt_IRQ == 16000){
        cnt_IRQ = 0;
        runtime++;
    }
    // Re-enable channel 0 ADC interrupt
    IOWR_16DIRECT(dp[0].DOC_ADC_BASE_ADDR, ADC_IRQ_ACK,0);

    exc_epilogue();
}

//################################################################################################
//DC Link Voltage Error
//###################################################################################################
static void dc_link_voltage_check(void){
    dc_link_read(&dc_link_voltage, &dc_link_current);
	int dcl_status = IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_STATUS);
	debug_printf(DBG_INFO, "---> DC_Link Voltage Check : Undervoltage limit = %i V  Overvoltage limit = %i V  \n", platform.powerboard->undervoltage, platform.powerboard->overvoltage);

	#ifdef LOOPBACK
		dc_link_voltage = 380;
		debug_printf(DBG_INFO, "DC_Link : %i  V - PASSED\n",dc_link_voltage);
		return;
	#endif

	while ((dcl_status != 0) || (dc_link_voltage < platform.powerboard->undervoltage)) {
		// Wait 1 second
		//OSTimeDlyHMSM(0, 0, 1, 0);
		OSTimeDlyHMSM(0, 0, 0, 500);
		dc_link_read(&dc_link_voltage, &dc_link_current);
		dcl_status = IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_STATUS);
		debug_printf(DBG_ERROR, "---> DC Link Status = %i\n",dcl_status);

		if ((dcl_status != 0) || (dc_link_voltage < platform.powerboard->undervoltage)) {
			debug_printf(DBG_ERROR, "---> DC Link Error = %s %s %i V\n",
					dcl_status & DOC_DC_LINK_STATUS_OV_BIT?"Overvoltage" : "",
					dcl_status & DOC_DC_LINK_STATUS_UV_BIT?"Undervoltage" : "",
					dc_link_voltage);
			//unsigned int v_tmp = (unsigned int)IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_VBUS);
			//debug_printf(DBG_ERROR, "---> DC link read int = %i\n",v_tmp);
			//v_tmp = (v_tmp*64*1996) / (82*32767);
			//debug_printf(DBG_ERROR, "---> DC link read int = %i\n",v_tmp);
		}

		if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
			// Altera power board has DC link current sense
			debug_printf(DBG_INFO, "--->             : %i  mA \n",dc_link_current);
		}
		debug_printf(DBG_WARN, "---> Check power connection. \n");
	}
    //OSTimeDlyHMSM(0, 0, 1, 0);
    OSTimeDlyHMSM(0, 0, 0, 500);
	debug_printf(DBG_INFO, "DC_Link : %i  V - PASSED\n", dc_link_voltage);
	//Note that if the power board is not powered at all the ADCs are not powered and the reading here is invalid.
	//An error will be detected in the ADC calibration routine as no clocks are received from the ADCs.
}

//################################################################################################
//ADC IRQ Check & Offset Current Calculation
//################################################################################################
// Read current values in idle state and average over 4K readings
// If value too high give error.
// Write ADC offset value to ADC peripheral
// Check IRQ is running at desired rate
//################################################################################################
int adc_offset_calculation(void) {
	int dn = 0;
	int timeout = 0;
	short this_error;
	short error = 0;
	int status_word;

	debug_printf(DBG_INFO, "ADC Offset calc\n");
	for (dn = platform.first_drive; dn <= platform.last_drive; dn++) {
		//timeout = 0;
		this_error = 0;

		dp[dn].Offset_start_calc = 0;
		dp[dn].enable_drive = 0;

		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_U , 32767);
		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_W , 32767);

		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_D, 1);
		IOWR_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_CONTROL , 1 ); // Pre-Charge  State

		// Wait for 1 second
		//OSTimeDlyHMSM(0, 0, 1, 0);
		OSTimeDlyHMSM(0, 0, 0, 500);

		debug_printf(DBG_INFO, "---> --------------------------------------------------\n");
		debug_printf(DBG_INFO, "---> STATE %i of Motor \n", IORD_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_STATUS) >> 9);

		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_U , 32767);  // Zero offset
		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_W , 32767);

		dp[dn].Offset_U = 0;
		dp[dn].Offset_W = 0;
		// Set start for offset calculation in ISR
		dp[dn].Offset_start_calc = 1;

		//Wait for offset calculation to finish
		debug_printf(DBG_INFO, "---> Axis %d: Offset calc\n", dn);
		while (dp[dn].Offset_start_calc != 0) {
			debug_printf(DBG_INFO,"---> Offset U %i , Offset W %i\n",dp[dn].Offset_U,dp[dn].Offset_W);
			OSTimeDlyHMSM(0, 0, 0, 100);
			timeout++;
			if (timeout > 10) {
				this_error = 1;
				error = error + (1<<dn);
				break;
			}
		}


		if (this_error == 1) {
			debug_printf(DBG_ERROR, "---> --------------------------------------------------\n");
			debug_printf(DBG_ERROR, "---> Timeout in Offset Calc!\n");
			dp[dn].Offset_U = 0;
			dp[dn].Offset_W = 0;

			status_word = (0x0FFF & IORD_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_STATUS));
			debug_printf(DBG_ERROR, "---> status_word %04X\n",status_word);

			if (status_word & (STATUS_REG_ERR_OV|STATUS_REG_ERR_UV|STATUS_REG_ERR_IGBT)) {
				debug_printf(DBG_ERROR, "---> Drive State Machine Errors = %s %s %s\n",
						status_word&STATUS_REG_ERR_OV?"DC_Link_Overvoltage":"",
						status_word&STATUS_REG_ERR_UV?"DC_Link_Undervoltage":"",
						status_word&STATUS_REG_ERR_IGBT?"IGBT_Error":""
				);
				debug_printf(DBG_ERROR, "---> ADC calibration failed due to above Drive State Machine Errors!\n");
				debug_printf(DBG_ERROR, "---> --------------------------------------------------\n");
			}
		} else {
			dp[dn].Offset_U = dp[dn].Offset_U / OFFSET_ACCUM_ISR_COUNT;
			dp[dn].Offset_W = dp[dn].Offset_W / OFFSET_ACCUM_ISR_COUNT;
			debug_printf(DBG_INFO, "---> Offset U: %i \n", (short) dp[dn].Offset_U);
			debug_printf(DBG_INFO, "---> Offset W: %i \n", (short) dp[dn].Offset_W);

			if ((abs(dp[dn].Offset_U) > CURRENT_OFFSET_LIMIT) || (abs(
					dp[dn].Offset_W) > CURRENT_OFFSET_LIMIT)) {
				error = error + (1<<(platform.powerboard->axes + dn));
				debug_printf(DBG_ERROR, "Error: ---> Axis %d: Offset calc unsuccessful! \n", dn);
			} else {
				IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_U , 32767+(short)dp[dn].Offset_U);
				IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_OFFSET_W , 32767+(short)dp[dn].Offset_W);
				debug_printf(DBG_INFO, "---> Axis %d: Offset calc successful! \n", dn);
			}
		}
		dp[dn].enable_drive = 0;
		IOWR_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR, ADC_D, 0);
		IOWR_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_CONTROL , 0 ); // Init State
	}
	return error;
}

/**
 * Helper function to print diagnostics based on error state. Called from main loop.
 *
 * @param dn
 */
static void decode_error_state(int dn) {
	int error = 0;
	int state = ((0x0FFF & IORD_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_STATUS))) >> 9;
	
	if (state == 4) {
		debug_printf(DBG_ERROR, "Axis %d: Actual State is Error!\n", dn);
		if (dp[dn].status_word & STATUS_REG_ERR_OC) {
			debug_printf(DBG_ERROR, "--> Axis %d: ERROR: <Overcurrent>\n", dn);
			debug_printf(DBG_ERROR,
					"Axis %d: I_U:%d\t\tI_W:%d\t\t\n",
					dn,
					(short) IORD_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR,
							OC_CAPTURE_U),
					(short) IORD_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR,OC_CAPTURE_W));
			debug_printf(DBG_ERROR, "Axis %d: I_Ref:%d\n", (short) dn,
					IORD_16DIRECT(dp[dn].DOC_ADC_BASE_ADDR,ADC_I_PEAK));
			error = 1;
		}
		if (dp[dn].status_word & STATUS_REG_ERR_OV) {
			debug_printf(DBG_ERROR, "Axis %d: --> ERROR: <Overvoltage>\n", dn);
			dc_link_read(&dc_link_voltage, &dc_link_current);
			debug_printf(DBG_ERROR, "U_DC_LINK: %i Volt \n", dc_link_voltage);
			// Altera power board has DC link current sense
			debug_printf(DBG_ERROR, "--->         : %i mA\n", dc_link_current);
			error = 1;
		}
		if (dp[dn].status_word & STATUS_REG_ERR_UV) {
			debug_printf(DBG_ERROR, "Axis %d: --> ERROR: <DC-Link-Error>\n",
					dn);
			dc_link_read(&dc_link_voltage, &dc_link_current);
			debug_printf(DBG_ERROR, "U_DC_LINK: %i Volt \n", dc_link_voltage);
			if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
				// Altera power board has DC link current sense
				debug_printf(DBG_ERROR, "--->         : %i mA\n",dc_link_current);
			}
			error = 1;
		}
		if (dp[dn].status_word & STATUS_REG_ERR_CLOCK) {
			debug_printf(DBG_ERROR,
					"Axis %d: --> ERROR: <SD-Clock or DC-Link-Clock-Error>\n",
					dn);
			error = 1;
		}
		if (dp[dn].status_word & STATUS_REG_ERR_IGBT) {
			debug_printf(DBG_ERROR,
					"Axis %d: --> ERROR: <IGBT-Error (no voltage?)>\n", dn);
			error = 1;
		}
		if (dp[dn].status_word & STATUS_REG_ERR_CHOPPER) {
			debug_printf(DBG_ERROR, "Axis %d: --> ERROR: <Chopper>\n", dn);
		}
		if (error == 0) {
			debug_printf(DBG_ERROR, "Axis %d: --> ERROR: <Unknown> %d\n", dn,
					dp[dn].status_word);
		}
	} else {
		debug_printf(DBG_INFO, "Axis %d: --> No Error\n", dn);
	}
}

/**
 * Helper function called form main loop to decode the buttons in the Tcl GUI and
 * switch modes or change the speed command.
 *
 * @param buttons	software button state
 * @return			returns 1 if all drives should be reset
 */
static int soft_button_scan(int buttons) {
	int ret = 0;
	
	//Switch DSP modes between software & DSP builder
	//km	if (debug_button_pressed(buttons, 0)) {
	//km		debug_printf(DBG_INFO, "Control Loop Mode Now: ");
		//Reset the control loop
	//km		dp[0].reset_control = 1;
	//km    	dp[1].reset_control = 1;
	//km    	dp[2].reset_control = 1;
	//km    	dp[3].reset_control = 1;
    	//km		switch (enable_dspba) {
    	//km			case SOFT_FIXP :
	//km				enable_dspba = DSPBA_FIXP;
	//km				debug_printf(DBG_INFO, "Hardware Fixed Point\n");
				//km				break;
				//km			case DSPBA_FIXP :
				//km				enable_dspba = DSPBA_FLOATP;
				//km				debug_printf(DBG_INFO, "Hardware Floating Point\n");
				//km				break;
				//km			case DSPBA_FLOATP :

				// SOFT_FLOATP (Software floating point) not supported on Nios
				//km				enable_dspba = SOFT_FIXP;
				//km				debug_printf(DBG_INFO, "Software Fixed Point\n");

				//km				break;
				//km			case SOFT_FLOATP :
				//km				enable_dspba = SOFT_FIXP;
				//km				debug_printf(DBG_INFO, "Software Fixed Point\n");
				//km				break;
				//km			default :
				//km				break;
				//km		}
	//km	}

	//Reduce speed command setting
	if (debug_button_pressed(buttons, 1)) {
		if (dp[axis_select].state_act == 3){//SPC
			if (dp[axis_select].speed_command <= (-3000 << SPEED_FRAC_BITS))
				dp[axis_select].speed_command = (-3000 << SPEED_FRAC_BITS);
			else
				dp[axis_select].speed_command = dp[axis_select].speed_command - (100 << SPEED_FRAC_BITS);
		}
		debug_printf(DBG_INFO, "Speed Setpoint is: %i rpm\n",dp[axis_select].speed_command>>SPEED_FRAC_BITS);
		debug_write_status (axis_select, DOC_DBG_SPEED_SETP0, dp[axis_select].speed_command>>SPEED_FRAC_BITS);
	}

	//Increase speed command setting
	if (debug_button_pressed(buttons, 2)) {
		if (dp[axis_select].state_act == 3){//SPC
			if (dp[axis_select].speed_command >= (3000<<SPEED_FRAC_BITS))
				dp[axis_select].speed_command = (3000<<SPEED_FRAC_BITS);
			else
				dp[axis_select].speed_command = dp[axis_select].speed_command + (100<<SPEED_FRAC_BITS);
			debug_printf(DBG_INFO, "Speed Setpoint is: %i rpm\n",dp[axis_select].speed_command>>SPEED_FRAC_BITS);
			debug_write_status (axis_select, DOC_DBG_SPEED_SETP0, dp[axis_select].speed_command>>SPEED_FRAC_BITS);
		}

	}

	//Reset all drives
	if (debug_button_pressed(buttons, 3)) {
		ret = 1;
	}

	//Switch to open loop control (test purposes)
	if (debug_button_pressed(buttons, 4)) {
		   dp[axis_select].openloop_test = (dp[axis_select].openloop_test == 0)?1:0;
		   debug_write_status (axis_select, DOC_DBG_OL_EN, dp[axis_select].openloop_test);
		   //SPC
		   dp[axis_select].speed_command = 100<<SPEED_FRAC_BITS;
		   debug_write_status (axis_select, DOC_DBG_SPEED_SETP0, dp[axis_select].speed_command<<SPEED_FRAC_BITS);
	}
	
	return ret;
}

/**
 * Restart all drives.
 *
 * Helper function called from main loop.
 */
static void	restart_all_drives(void) {
	int dn;
	
	for (dn = platform.first_drive; dn <= platform.last_drive; dn++){ //MAX
	   dsm_reset(dp[dn].DOC_SM_BASE_ADDR);
	   dp[dn].enable_drive = 0 ;              // disable
	   //SPC
	   dp[dn].speed_command = 100<<SPEED_FRAC_BITS;        // set back to initial speed
	   debug_write_status (dn, DOC_DBG_SPEED_SETP0, dp[dn].speed_command>>SPEED_FRAC_BITS);
	}
}

/**
 * Helper function called for each axis from motor task to initialize drive data structure.
 *
 * @param dn	drive axis to be initialized
 */
static void	init_axis(int dn) {
	init_debug(dn, &dp[0]);

	//---------------------------------------------------------------------------------------------------------------------
	// Init PI controllers DSP
	//---------------------------------------------------------------------------------------------------------------------
	dp[dn].Id_PI.Kp = dp[dn].Id_Kp;
	dp[dn].Id_PI.Ki = dp[dn].Id_Ki;
	dp[dn].Id_PI.input_frac_bits = 10;
	dp[dn].Id_PI.feedback_limit = dp[dn].I_sat_limit;
	dp[dn].Id_PI.integrator_limit = dp[dn].V_sat_limit;
	dp[dn].Id_PI.output_limit = dp[dn].V_sat_limit;
	PI_reset_q15(&dp[dn].Id_PI);

	dp[dn].Iq_PI.Kp = dp[dn].Iq_Kp;
	dp[dn].Iq_PI.Ki = dp[dn].Iq_Ki;
	dp[dn].Iq_PI.input_frac_bits = 10;
	dp[dn].Iq_PI.feedback_limit = dp[dn].I_sat_limit;
	dp[dn].Iq_PI.integrator_limit = dp[dn].V_sat_limit;
	dp[dn].Iq_PI.output_limit = dp[dn].V_sat_limit;
	PI_reset_q15(&dp[dn].Iq_PI);

	dp[dn].Speed_PI.Kp = dp[dn].speed_Kp;
	dp[dn].Speed_PI.Ki = dp[dn].speed_Ki;
	dp[dn].Speed_PI.input_frac_bits = 12 + SPEED_FRAC_BITS;
	dp[dn].Speed_PI.feedback_limit = dp[dn].speed_limit;
	dp[dn].Speed_PI.integrator_limit = dp[dn].I_sat_limit;
	dp[dn].Speed_PI.output_limit = dp[dn].I_sat_limit;
	PI_reset_q15(&dp[dn].Speed_PI);

	dp[dn].Position_PI.Kp = dp[dn].pos_Kp;
	dp[dn].Position_PI.Ki = dp[dn].pos_spdff_Kp;
	dp[dn].Position_PI.input_frac_bits = 10 - SPEED_FRAC_BITS; //17;

	dp[dn].Position_PI.integrator_limit = 0;
	dp[dn].Position_PI.feedback_limit = 1073741824; //32000; //Max short_int

	dp[dn].Position_PI.output_limit = dp[dn].pos_limit;
	PI_reset_q15(&dp[dn].Position_PI);

	//---------------------------------------------------------------------------------------------------------------------
	// Init DSP Builder subsystems
	//---------------------------------------------------------------------------------------------------------------------
//	debug_printf(DBG_INFO, "Init DSP Builder subsystems\n");


//	INIT_DSPBA_DOC_Single_Axis_dspba_fixp_regs (FOC_FIXED_POINT_BASE, dp[dn].I_sat_limit, dp[dn].Id_Ki,
//												dp[dn].Id_Kp, dp[dn].V_sat_limit, dn);

//	INIT_DSPBA_DOC_Single_Axis_dspba_floatp_regs (FOC_FLOATING_POINT_BASE, dp[dn].I_sat_limit, dp[dn].Id_Ki,
//												 dp[dn].Id_Kp, dp[dn].V_sat_limit, dn);
}

/**
 * Helper function called for each axis on each pass of motor task loop. Writes debug status
 * to and reads inputs from system console GUI.
 *
 * @param dn	drive axis to be updated
 */
static void	update_axis(int dn) {
	debug_write_status (dn, DOC_DBG_DRIVE_STATE, dp[dn].state_act);
	debug_write_status (dn, DOC_DBG_RUNTIME, runtime);
	//km	debug_write_status (dn, DOC_DBG_DSP_MODE, enable_dspba);
	debug_write_status (dn, DOC_DBG_SPEED, dp[dn].speed_encoder >> SPEED_FRAC_BITS);
	debug_write_status (dn, DOC_DBG_POSITION, dp[dn].pos_int);
	debug_write_status (dn, DOC_DBG_LATENCY, latency);

	poll_debug (dn, &dp[0]);

//				INIT_DSPBA_DOC_Single_Axis_dspba_fixp_regs (FOC_FIXED_POINT_BASE,
//																	dp[dn].I_sat_limit,
//																	dp[dn].Id_Ki,
//																	dp[dn].Id_Kp,
//																	dp[dn].V_sat_limit,
//																	dn
//																	);
//
//				INIT_DSPBA_DOC_Single_Axis_dspba_floatp_regs (FOC_FLOATING_POINT_BASE,
//																	dp[dn].I_sat_limit,
//																	dp[dn].Id_Ki,
//																	dp[dn].Id_Kp,
//																	dp[dn].V_sat_limit,
//																	dn
//																	);

	dp[dn].Id_PI.Kp = dp[dn].Id_Kp;
	dp[dn].Id_PI.Ki = dp[dn].Id_Ki;
	dp[dn].Id_PI.feedback_limit = dp[dn].I_sat_limit;
	dp[dn].Id_PI.integrator_limit = dp[dn].V_sat_limit;
	dp[dn].Id_PI.output_limit = dp[dn].V_sat_limit;

	dp[dn].Iq_PI.Kp = dp[dn].Id_Kp;
	dp[dn].Iq_PI.Ki = dp[dn].Id_Ki;
	dp[dn].Iq_PI.feedback_limit = dp[dn].I_sat_limit;
	dp[dn].Iq_PI.integrator_limit = dp[dn].V_sat_limit;
	dp[dn].Iq_PI.output_limit = dp[dn].V_sat_limit;


	dp[dn].Speed_PI.Kp = dp[dn].speed_Kp;
	dp[dn].Speed_PI.Ki = dp[dn].speed_Ki;
	dp[dn].Speed_PI.feedback_limit = dp[dn].speed_limit;
	dp[dn].Speed_PI.integrator_limit = dp[dn].I_sat_limit;
	dp[dn].Speed_PI.output_limit = dp[dn].I_sat_limit;

	dp[dn].Position_PI.Kp = dp[dn].pos_Kp;
	dp[dn].Position_PI.Ki = 0;
	//dp[dn].Position_PI.feedback_limit = 1073741824; //32000; //Max short_int
	//dp[dn].Position_PI.integrator_limit = 0;
	dp[dn].Position_PI.output_limit = dp[dn].pos_limit;

}

/**
 * main loop task, initializes and loops forever.
 *
 * @param pdata
 */

void motor_task(void* pdata) {
	int buttons; //Store button values
	int dn; // drive number index
	int restart_drive = 0;
	int last_runtime = 0;

	memset(&platform, 0, sizeof(platform_t));
	// Determine what hardware we are running on
	if (decode_sysid(SYSID_0_BASE) > 0) {
		while (1) {
			OSTimeDlyHMSM(0, 0, 1, 0);
		}
	}

	init_sin_cos_tables();

	while (1) {
		//km		enable_dspba = SOFT_FIXP;
		init_dp(&dp[0]);
		//debug_get_buttons((17 * 4), 5, &buttons);

		// Initialise all drives
		// Leave this as 0 to platform.last_drive for debug interface
		for (dn = 0; dn <= platform.last_drive; dn++) { //MAX
			init_axis(dn);
		}

		//PERF_RESET (PERFORMANCE_COUNTER_0_BASE);

		debug_printf(DBG_INFO, "[Motor task] Reset drive state machines\n");
		// PWM modules must be setup for all channels up to platform.last_drive
		// to ensure consistent synchronisation
		// At present we only use IRQ from channel 0 ADC, but set them all
		// up here.
		for (dn = 0; dn <= platform.last_drive; dn++) {
			dsm_reset_to_idle(dp[dn].DOC_SM_BASE_ADDR);
			pwm_setup(dp[dn].DOC_PWM_BASE_ADDR,PWMMAX);
			adc_setup(dp[dn].DOC_ADC_BASE_ADDR);
		}

//#ifdef JTAG_UART_BASE
//		IOWR_ALTERA_AVALON_JTAG_UART_CONTROL(JTAG_UART_BASE,0);
//#endif

		//Configure the DC link
		debug_printf(DBG_INFO, "[Motor task] Configure DC link\n");
		dc_link_setup();
		OSTimeDlyHMSM(0, 0, 0, 1);
		unsigned short int tmp = IORD_16DIRECT(DOC_DC_LINK_BASE, DOC_DC_LINK_OVERVOLTAGE);
		debug_printf(DBG_INFO, "[Motor task] Overvoltage %i\n", tmp);
		debug_printf(DBG_INFO, "[Motor task] Overvoltage %i\n", platform.powerboard->overvoltage);

		//Check the DC link voltage measurement
		debug_printf(DBG_INFO, "[Motor task] Check DC Link\n");
		dc_link_voltage_check();
		debug_printf(DBG_INFO, "[Motor task] ---> Setting state to 6 \n");
		debug_write_status (0, DOC_DBG_DRIVE_STATE, 6); //passed voltage test
		debug_printf(DBG_INFO, "[Motor task] ---> ------------------------------------\n");
		debug_printf(DBG_INFO, "[Motor task] ---> DC_Link : %i V\n", dc_link_voltage);
		if (platform.powerboard->sysid == SYSID_PB_ALT12_MULTIAXIS) {
			// Altera power board has DC link current sense
			debug_printf(DBG_INFO, "[Motor task] --->         : %i mA\n",dc_link_current);
		}

		dc_link_chopper_setup();

/*
		// Initialise Encoders
		debug_printf(DBG_INFO, "[Motor task] %s Initialization\n", platform.encoder->name);
		for (dn = platform.first_drive; dn <= platform.last_drive; dn++){//MAX
			debug_printf(DBG_INFO, "[Motor task] --------------------------------------------------------------------------\n");
			debug_printf(DBG_INFO, "[Motor task] %s init for channel %d\n", platform.encoder->name, dn);
			if (platform.encoder->encoder_init_fn(&dp[dn]) != 0) {;
				debug_printf(DBG_ERROR, "[Motor task] %s communication failed for channel %d\n", platform.encoder->name, dn);
				while (1) {
					OSTimeDlyHMSM(0, 0, 0, 1);
				}
			} else {
				debug_printf(DBG_INFO, "[Motor task] %s parameters for channel %d\n", platform.encoder->name, dn);
				debug_printf(DBG_INFO, "[Motor task] Commutation-Angle : %i count\n", dp[dn].mphase);
				debug_printf(DBG_INFO, "[Motor task] %s actual      : %d  Bit\n", platform.encoder->name, dp[dn].encoder_length);
				debug_printf(DBG_INFO, "[Motor task] Multiturn para    : %i\n", dp[dn].encoder_multiturn);
				debug_printf(DBG_INFO, "[Motor task] Multiturn         : %i\tBit\n", dp[dn].encoder_multiturn_bits);
				debug_printf(DBG_INFO, "[Motor task] Singleturn        : %i\tBit\n", dp[dn].encoder_singleturn_bits);
				debug_printf(DBG_INFO, "[Motor task] Resolution mask   : 0x%x\n", dp[dn].encoder_mask);
				debug_printf(DBG_INFO, "[Motor task] Turns mask        : 0x%x\n", dp[dn].encoder_turns_mask);
			}
			if (dp[dn].encoder_version > 0)
				debug_printf(DBG_INFO, "[Motor task] Version of %s interface  V%i.2 \n", platform.encoder->name, dp[dn].encoder_version);
		};
*/
		debug_printf(DBG_INFO, "[Motor task] ---> Setting state to 7 \n");
		debug_write_status (0, DOC_DBG_DRIVE_STATE, 7); //passed EnDat test

		// Enable drive IRQ in CPU interrupt controller
		// TODO: setup drive_irq() as interrupt handler on patmos
		//alt_ic_isr_register(0, DRIVE0_DOC_ADC_IRQ, drive_irq, NULL, NULL);
		//exc_register(6,&drive_irq);
		exc_register(16+6,&drive_irq);

		// unmask interrupts
		intr_unmask_all();
		

		debug_printf(DBG_INFO, "[Motor task] ---> Interrupt service routine setup \n");

		//Enable the PWMs after setting up the EnDat / Biss encoders
		for (dn = 0; dn <= platform.last_drive; dn++) {
			dsm_reset(dp[dn].DOC_SM_BASE_ADDR);
			debug_printf(DBG_INFO, "[Motor task] ---> DSM setup \n");
			pwm_setup(dp[dn].DOC_PWM_BASE_ADDR,PWMMAX);
			debug_printf(DBG_INFO, "[Motor task] ---> PWM setup \n");
			adc_setup(dp[dn].DOC_ADC_BASE_ADDR);
			debug_printf(DBG_INFO, "[Motor task] ---> ADC setup \n");
		}
		debug_printf(DBG_INFO, "[Motor task] ---> IO devices setup \n");

		// enable interrupts
		intr_enable();

		if (adc_offset_calculation()>0) {
			restart_drive = 1;  //loop restarting the drive if error
		}
	
		if (restart_drive == 1) {
			debug_printf(DBG_ERROR, "[Motor task] ---> Failure detected in ADC calibration. Check power connection. \n");
		}
		
		// Normal operation task loop
		while(restart_drive == 0) {
			// Service all drives
			// Leave this as 0 to platform.last_drive for debug interface
			for (dn = 0; dn <= platform.last_drive; dn++){ //MAX
				update_axis(dn);
			}

		    //axis_select = debug_read_command (0, DOC_DBG_AXIS_SELECT);
		    axis_select = 0;

			// Debug Output of Motor Data once per second only in Run State
			if (dp[platform.first_drive].state_act == 3 ) {
				if (runtime != last_runtime) {
					last_runtime = runtime;
					//PERF_STOP_MEASURING (PERFORMANCE_COUNTER_0_BASE);
					//latency = small2_perf_get_latency  ((void *)PERFORMANCE_COUNTER_0_BASE, 2, AVALON_FREQ); //The FOC portion of the IRQ for 1 axis
					//latency = small2_perf_get_latency  ((void *)PERFORMANCE_COUNTER_0_BASE, 1, AVALON_FREQ); //The complete IRQ
					//PERF_RESET (PERFORMANCE_COUNTER_0_BASE);
					//PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);
				}
			}

			// Drive state machine
			for (dn = platform.first_drive; dn <= platform.last_drive; dn++){ //MAX
				if (dp[dn].state_act != 3 ) { //Disable Motor when not in Run State
					dp[dn].enable_drive = 0 ;
				}
				dp[dn].state_act_old  = dp[dn].state_act;
				dp[dn].status_word = (0x0FFF & IORD_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_STATUS));
				dp[dn].state_act	= dp[dn].status_word >> 9;
				if (dp[dn].state_act != dp[dn].state_act_old) {
					debug_printf(DBG_INFO, "Axis %d: State now %d  Status=%04X\n", dn, dp[dn].state_act, dp[dn].status_word);
				}

				switch (dp[dn].state_act) {
					case 0: /***Init*************/
						dp[dn].enable_drive = 0 ;                          // Reset current controller command values and SVM output voltage
						debug_write_status (dn, DOC_DBG_DRIVE_STATE, 0);
						runtime =0 ;
						cnt_IRQ =0 ;
						adc_overcurrent_enable(dp[dn].DOC_ADC_BASE_ADDR,0);     // overcurrent measurement disabled
						OSTimeDlyHMSM(0, 0, 0, 250);
						IOWR_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_CONTROL , 1) ;
						break;
						
					case 1: /***Precharge********/
						// go straight to state 2
						debug_write_status (dn, DOC_DBG_DRIVE_STATE, 1);
						OSTimeDlyHMSM(0, 0, 0, 250);
						IOWR_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_CONTROL, 2) ;
						break;
						
					case 2: /***Prerun**********/
						debug_write_status (dn, DOC_DBG_DRIVE_STATE, 2);
						adc_overcurrent_enable(dp[dn].DOC_ADC_BASE_ADDR,1); // enable overcurrent measure
						// go straight to state 3
						OSTimeDlyHMSM(0, 0, 0, 250);
						IOWR_16DIRECT(dp[dn].DOC_SM_BASE_ADDR, SM_CONTROL, 3) ;
						break;
						
					case 3: /***Run*************/
						dp[dn].enable_drive = 1 ;
						break;
						
					case 4:
						// Error state
						restart_drive = 1;
						decode_error_state(dn);
						OSTimeDlyHMSM(0, 0, 0, 10);
						break;
				}
			}
			OSTimeDlyHMSM(0, 0, 0, 100);

			restart_drive |= soft_button_scan(buttons);
		}	// while(restart_drive == 0)

		decode_error_state(0);
		decode_error_state(1);
		decode_error_state(2);
		decode_error_state(3);
		restart_all_drives();

		debug_printf(DBG_INFO, "\n[Motor task] Resetting drives please wait... \n");
		OSTimeDlyHMSM(0, 0, 3, 0);
		restart_drive = 0;
	}
}

/*!
 * @}
 */

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

#ifndef __MC_DSP_H_
#define __MC_DSP_H_

/**
 * @file mc_dsp.h
 *
 * @brief Header file for DSP functions for motor control
 */

#include "sin_cos_q15.h"

/*!
 * \addtogroup MC
 *
 * @{
 */

/*!
 * \addtogroup DSPFIXED Motor Control DSP Functions (Fixed Point)
 *
 * @{
 */

/**
 * PI Controller data structure
 */
typedef struct
{
	int output;           	//!< The output correction value A0 = Kp + Ki .
	int integrator;      	//!< The integral value.
	int Kp;           		//!< The proportional gain.
	int Ki;           		//!<  The integral gain.
	int input_frac_bits;  	//!< Fractional bits on setpoint + feedback
	int setpoint;
	int feedback;
	int feedback_limit;
	int integrator_limit;
	int output_limit;
} pi_instance_q15;


// Use macros for speed to avoid parameter passing. More critical for Nios II than HPS
#define SINE(a) (sin_q15((a)>>1))
#define COSINE(a) (cos_q15((a)>>1))

/**
 * Limits the input value (operand_a) to +/- the range limit (operand_b)
 *
 * @param a Input value
 * @param b Upper/lower range limit (saturation limit)
 * @return Input value after saturation
 */
#define ABS_MAX(a,b) (((a)>(b))?(b):((a)<-(b))?-(b):(a))

/**
 * Clarke Transform (alpha-beta transformation) is a space vector transformation of time-domain
 * signals (e.g. voltage, current, flux, etc) from a natural three-phase coordinate system (ABC/UVW) into a
 * stationary two-phase reference frame (alpha,beta).
 * The implementation of this transform is simplified due to the fact that U + V + W = 0 in a 3 phase motor,
 * so only U & W input vectors are used.
 *
 * @param i_u_q10 U input
 * @param i_w_q10 W input
 * @param[out] i_alpha_q10 alpha output
 * @param[out] i_beta_q10 beta output
 */
void clark_transform_q10(short i_u_q10, short i_w_q10, int *i_alpha_q10, int *i_beta_q10);

/**
 * Park Transformation is a space vector transformation of three-phase time-domain signals from a
 * stationary phase coordinate system (alpha,beta) to a rotating coordinate system (dq0).
 *
 * @param Ialpha_q10 Alpha input
 * @param Ibeta_q10 Beta input
 * @param[out] Id_q10 D
 * @param[out] Iq_q10 Q
 * @param sinVal_q15 Sine value for angle (w)
 * @param cosVal_q15 Cosine value for angle (w)
 */
void park_transformation_q10(int Ialpha_q10, int Ibeta_q10, int * Id_q10, int * Iq_q10, int sinVal_q15, int cosVal_q15);

/**
 * Inverse Park Transformation ( rotor - stator ) 2 axis coordinate system ( d , q )
 * ( time invariant ) --> 2 axis coordinate system ( alpha , beta )
 *
 * @param Vd_q10 D input
 * @param Vq_q10 Q input
 * @param[out] pValpha_q10 Alpha output
 * @param[out] pVbeta_q10 Beta output
 * @param sinVal_q15 Sine value for angle (w)
 * @param cosVal_q15 Cosine value for angle (w)
 */
void inverse_park_q10(int Vd_q10, int Vq_q10, int * pValpha_q10, int * pVbeta_q10, int sinVal_q15, int cosVal_q15);

/**
 * PI controller algorithm: Executes one time-step every execution
 * All coefficients, data inputs and outputs are contained in the S data structure
 *
 * @param S PI controller data structure
 * @param resetStateFlag
 */
void PI_control_q15(pi_instance_q15 * S, int resetStateFlag);

/**
 * PI controller reset
 *
 * @param S PI controller structure to reset
 */
void PI_reset_q15(pi_instance_q15 * S);

/**
 * Space Vector Modulation - Creates a switching sequence for the 3 voltage phases (UVW) of the motor that
 * are equivalent to the 2 phase a & b input vectors. Additional a third harmonic voltage waveform is added
 * to the SVM output to flatten the peaks and allows the maximum modulation index to be 1.15
 * The U,V,W outputs are limited to between 0 < voltage < pwm_max and are used directly by the PWM to indicate
 * which clock cycle the PWM switches high as the PWM counts up and then low again as the PWM counts down.
 *
 * @param pwm_max Sets the maximum value for ru, rv & rw output values.
 * @param u_a Input voltage vector a
 * @param u_b Input voltage vector b
 * @param [out] ru_out PWM switching threshold for U voltage phase
 * @param [out] rv_out PWM switching threshold for V voltage phase
 * @param [out] rw_out PWM switching threshold for W voltage phase
 */
void svm(int pwm_max, int u_a, int u_b, int * ru_out, int * rv_out, int * rw_out);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* __MC_DSP_H_ */


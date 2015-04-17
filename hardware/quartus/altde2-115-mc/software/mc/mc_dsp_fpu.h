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

#ifndef __MC_DSP_FPU_H_
#define __MC_DSP_FPU_H_

/**
 * @file mc_dsp_fpu.h
 *
 * @brief Header file for DSP functions for motor control
 */

/*!
 * \addtogroup MC
 *
 * This module is the basic motor control demo.
 *
 * @{
 */

/*!
 * \addtogroup DSPFLOAT Motor Control DSP Functions (Floating Point)
 *
 * This module is the basic motor control demo.
 *
 * @{
 */

// Macros to help convert Q format to float
#define SHIFTR10FLOAT (float)(1.0f /(1<<10))
#define SHIFTR15FLOAT (float)(1.0f /(1<<15))
#define SHIFTR17FLOAT (float)(1.0f /(1<<17))

// Macros to help convert float to Q format
#define SHIFTL10FLOAT (float)(1.0f *(1<<10))
#define SHIFTL15FLOAT (float)(1.0f *(1<<15))
#define SHIFTL17FLOAT (float)(1.0f *(1<<17))

/**
 * PI Controller data structure (floating point)
 */
typedef struct
{
	float output;       //!< The output correction value A0 = Kp + Ki .
	float integrator;   //!< The integral value. Stored from previous calculation.
	float Kp;           //!< The proportional gain coefficient.
	float Ki;           //!< The integral gain coefficient.
	float setpoint;		//!< The setpoint input. The (setpoint - feedback) is the error input the controller.
	float feedback;     //!< The feedback input.
	float feedback_limit;
	float integrator_limit;
	float output_limit;
} pi_instance_f;


/**
 * Limits the input value (operand_a) to +/- the range limit (operand_b)
 *
 * @param operand_a Input value
 * @param operand_b Upper/lower range limit (saturation limit)
 * @return Input value after saturation
 */static inline float abs_max_f(float operand_a, float operand_b)
{
	float result_SW;
	result_SW = (operand_a > operand_b) ? operand_b : (operand_a < -operand_b) ? -operand_b : operand_a;
	return result_SW;
}


/**
 * Clarke Transform (alpha-beta transformation) is a space vector transformation of time-domain
 * signals (e.g. voltage, current, flux, etc) from a natural three-phase coordinate system (ABC/UVW) into a
 * stationary two-phase reference frame (alpha,beta).
 * The implementation of this transform is simplified due to the fact that U + V + W = 0 in a 3 phase motor,
 * so only U & W input vectors are used.
 * @param i_u_q10 U input
 * @param i_w_q10 W input
 * @param[out] i_alpha_q10 alpha output
 * @param[out] i_beta_q10 beta output
 */static void inline clark_transform_f(
	  float   i_u_q10,
	  float   i_w_q10,
	  float   *i_alpha_q10,
	  float   *i_beta_q10
){
  *i_alpha_q10 = i_u_q10 ;
  *i_beta_q10  = ( ((float)-591/(float)1024) * (i_u_q10+(i_w_q10*2)) ) ;  //1/sqrt(3)*(iu+2*iv)
}

/**
 * Park Transformation is a space vector transformation of three-phase time-domain signals from a
 * stationary phase coordinate system (alpha,beta) to a rotating coordinate system (dq0).
 *
 * @param Ialpha_f Alpha input
 * @param Ibeta_f Beta input
 * @param[out] Id_f D
 * @param[out] Iq_f Q
 * @param sinVal_f Sine value for angle (w)
 * @param cosVal_f Cosine value for angle (w)
 */
static void inline park_transformation_f(
        float Ialpha_f,
        float Ibeta_f,
        float * Id_f,
        float * Iq_f,
        float sinVal_f,
        float cosVal_f)
{
    *Id_f = (  Ialpha_f * cosVal_f + Ibeta_f * sinVal_f ) ;
    *Iq_f = ( -Ialpha_f * sinVal_f + Ibeta_f * cosVal_f ) ;
}


//################################################################################################
//
//################################################################################################
/**
 * Inverse Park Transformation ( rotor - stator ) 2 axis coordinate system ( d , q )
 * ( timeinvariant ) --> 2 axis coordinate system ( alpha , beta )
 *
 * @param Vd_f D input
 * @param Vq_f Q input
 * @param[out] pValpha_f Alpha output
 * @param[out] pVbeta_f Beta output
 * @param sinVal_f Sine value for angle (w)
 * @param cosVal_f Cosine value for angle (w)
 */static void inline inverse_park_f(
                    float Vd_f,
                    float Vq_f,
                    float * pValpha_f,
                    float * pVbeta_f,
                    float sinVal_f,
                    float cosVal_f)
{
    *pValpha_f = ( Vd_f * cosVal_f - Vq_f * sinVal_f );
    *pVbeta_f  = ( Vd_f * sinVal_f + Vq_f * cosVal_f );
}

/**
 * PI controller algorithm: Executes one time-step every execution
 * All coefficients, data inputs and outputs are contained in the S data structure
 *
 * @param S PI controller data structure
 * @param resetStateFlag
 */
 static void inline PI_control_f(
  pi_instance_f * S,
  int resetStateFlag)
{
    float error_input;
    float proportional;

    error_input = S->setpoint - S->feedback;
    error_input = abs_max_f(error_input, S->feedback_limit);
    proportional = error_input * S->Kp;
    S->integrator += error_input * S->Ki;
    S->integrator = abs_max_f(S->integrator, S->integrator_limit);
    S->output = abs_max_f((proportional + S->integrator), S->output_limit);

  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    // Clear the output and integrator
    S->output = 0;
    S->integrator = 0;
  }

}


/**
 * PI controller reset
 * @param S PI controller structure to reset
 *
 */
 static void inline PI_reset_f(
  pi_instance_f * S)
{
    // Clear the output and integrator
    S->output = 0;
    S->integrator = 0;
}

#endif /* __MC_DSP_FPU_H_ */

 /*!
  * @}
  */

 /*!
 * @}
 */

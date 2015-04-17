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
 * @file mc_dsp.c
 *
 * @brief DSP functions for motor control
 */

#include "includes.h"

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

//################################################################################################
//Clark Transformation ( stator - stator ) 3 phase u , v , w - axis --> 2 axis coordinate system ( alpha , beta )
//################################################################################################
void clark_transform_q10(
          short   i_u_q10,
          short   i_w_q10,
           int     *i_alpha_q10,
           int     *i_beta_q10
  ){
      *i_alpha_q10 = i_u_q10 ;
      *i_beta_q10  = (-591*(i_u_q10+(i_w_q10<<1)))>>10 ;  //1/sqrt(3)*(iu+2*iv)
  }

//################################################################################################
//Park Transformation ( d / q ) ( stator - rotor ) 2 axis coordinate system ( alpha , beta ) --> 2 axis coordinate system ( d , q ) ( timeinvariant )
//################################################################################################
void park_transformation_q10(
                        int Ialpha_q10,
                        int Ibeta_q10,
                         int * Id_q10,
                         int * Iq_q10,
                        int sinVal_q15,
                        int cosVal_q15)
{
    *Id_q10 = (  Ialpha_q10 * cosVal_q15 + Ibeta_q10 * sinVal_q15 ) >> 15 ;
    *Iq_q10 = ( -Ialpha_q10 * sinVal_q15 + Ibeta_q10 * cosVal_q15 ) >> 15 ;
}

//################################################################################################
//Inverse Park Transformation ( rotor - stator ) 2 axis coordinate system ( d , q )( timeinvariant ) --> 2 axis coordinate system ( alpha , beta )
//################################################################################################
void inverse_park_q10(
                    int Vd_q10,
                    int Vq_q10,
                     int * pValpha_q10,
                     int * pVbeta_q10,
                    int sinVal_q15,
                    int cosVal_q15)
{
    *pValpha_q10 = ( Vd_q10 * cosVal_q15 - Vq_q10 * sinVal_q15 ) >> 15 ;
    *pVbeta_q10  = ( Vd_q10 * sinVal_q15 + Vq_q10 * cosVal_q15 ) >> 15 ;
}

//################################################################################################
//PI Controller
//################################################################################################
void PI_control_q15(
   pi_instance_q15 * S,
   int resetStateFlag)
{
    int error_input;
    int proportional;

    error_input = S->setpoint - S->feedback;
    error_input = ABS_MAX(error_input, S->feedback_limit);
    proportional = error_input * S->Kp;
    S->integrator += error_input * S->Ki;
    S->integrator = ABS_MAX(S->integrator, S->integrator_limit << S->input_frac_bits);
    S->output = ABS_MAX((proportional + S->integrator) >> S->input_frac_bits, S->output_limit);

  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    // Clear the output and integrator
    S->output = 0;
    S->integrator = 0;
  }

}

//################################################################################################
//PI Controller Reset
//################################################################################################
void PI_reset_q15(
   pi_instance_q15 * S)
{
    // Clear the output and integrator
    S->output = 0;
    S->integrator = 0;
}

//################################################################################################
// Current Control
//################################################################################################
void current_control( pi_instance_q15 * SId, pi_instance_q15 * SIq){

	PI_control_q15(SId,0);
	PI_control_q15(SIq,0);

};


//################################################################################################
// Space Vector Modulation - DC ( angle & absolute value ) --> Spacevector ( depends on switching time like PWM )
//################################################################################################
void svm(
                    int pwm_max,
                    int u_a,
                    int u_b,
                     int * ru_out,
                     int * rv_out,
                     int * rw_out
                    )
{
    const short     c0577 = 18919 ; // 32768 / sqrt(3)
    int             tmax  = pwm_max;
    int             ru,rv,rw ;
    u_b = (u_b * c0577) >> 15 ;  //Note: scale by 1/sqrt(3) for *u_b* only
    if ( u_a >= 0 ) {
        if ( u_b >= 0 ){
            if ( u_a < u_b ) goto sector_2 ;
            //   SVM sector 1
            rv =  u_a - u_b ;                  //   t1 =   u_a - u_b ;
            rw =  u_b + u_b ;                  //   t2 =   2 * u_b ;
            ru = (tmax + rv + rw) >> 1 ;
            rv = ru - rv ;
            rw = rv - rw ;
        } else {
            if ( u_a < -u_b ) goto sector_5 ;
            //   SVM sector 6
            rv = - u_b - u_b ;                  //   t1 =  -2 * u_b ;
            rw =   u_a + u_b ;                  //   t2 =  u_a + u_b ;
            ru = (tmax + rw+ rv) >> 1 ;
            rw = ru - rw ;
            rv = rw - rv ;
               }
    } else {
        if ( u_b >= 0 ){
            if ( -u_a >= u_b ) {
                //   SVM sector 3
                rw =   u_b + u_b ;                  //   t1 =   2 *u_b ;
                ru =  -u_a - u_b ;                  //   t2 = - u_a - u_b ;
                rv = (tmax + ru + rw) >> 1 ;
                rw = rv - rw ;
                ru = rw - ru;
            } else {
            	sector_2:   // SVM sector 2
                rw =    u_a + u_b ;                  //   t1 =   u_a + u_b ;
                ru =   -u_a + u_b ;                  //   t2 =  -u_a + u_b ;
                rv = (tmax + ru + rw) >> 1 ;
                ru = rv - ru ;
                rw = ru - rw ;
            }
        } else {
            if ( u_a < u_b ) {
                // SVM sector 4
                ru =  -u_a + u_b ;                  //   t1 = -u_a + u_b ;
                rv =  -u_b - u_b ;                  //   t2 =  -2 * u_b ;
                rw = (tmax + ru + rv) >> 1 ;
                rv = rw - rv ;
                ru = rv - ru ;
            } else {
            	sector_5:
                // SVM sector 5
                ru =  -u_a - u_b ;                  //   t1 = -u_a - u_b ;
                rv =   u_a - u_b ;                  //   t2 =  u_a - u_b ;
                rw = (tmax + ru + rv) >> 1 ;
                ru = rw - ru ;
                rv = ru - rv ;
            }
        }
    }

    if ( ru < 0 ) {
        ru = 0;
    } else if( ru > tmax ) {
        ru = tmax ;
    }
    *ru_out = ru;
    if ( rv < 0 ) {
        rv = 0 ;
    } else if ( rv > tmax ) {
        rv = tmax ;
    }
    *rv_out = rv;
    if ( rw < 0 ) {
        rw = 0 ;
    } else if( rw > tmax ) {
        rw = tmax ;
    }
    *rw_out = rw;

}

/*!
 * @}
 */

/*!
 * @}
 */

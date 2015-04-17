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
 * @file ssg_emb_pwm.c
 *
 * @brief PWM interface
 */

#include "includes.h"

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup PWM PWM Interface
 *
 * @brief Interface to PWM modules
 *
 * The PWM module sets the cycle time (default 62.5us) and generates the switching waveforms for
 * the IGBT modules on the power board.
 *
 * Outputs form the PWM module are used to trigger samples from the encoder inputs and ADCs.
 *
 * @{
 */

void pwm_setup(int base_address, int pwmmax)
{
    //segment(0); // INIT
    //---------------------------------------------------------------------------------------------------------------------
    //Parameters of PWM
    //---------------------------------------------------------------------------------------------------------------------
    IOWR_16DIRECT(base_address, PWM_MAX          , pwmmax     );
    IOWR_16DIRECT(base_address, PWM_BLOCK        , 100        );
    IOWR_16DIRECT(base_address, PWM_TRIGGER_UP   , pwmmax-480 );//was 120
    IOWR_16DIRECT(base_address, PWM_TRIGGER_DOWN , 480        );//was 120
}

void pwm_update(int base_address, int Vu_PWM, int Vv_PWM, int Vw_PWM)
{
    IOWR_16DIRECT(base_address, PWM_U, Vu_PWM);
    IOWR_16DIRECT(base_address, PWM_V, Vv_PWM);
    IOWR_16DIRECT(base_address, PWM_W, Vw_PWM);
}

/*!
 * @}
 */

/*!
 * @}
 */

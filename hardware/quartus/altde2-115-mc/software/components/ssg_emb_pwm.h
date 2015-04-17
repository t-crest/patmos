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

#ifndef SSG_EMB_PWM_H_
#define SSG_EMB_PWM_H_

/**
 * @file ssg_emb_pwm.h
 *
 * @brief Header file for PWM interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup PWM
 *
 * @{
 */

// PWM internal ADR OFFSET
#define PWM_U                        0x04   // w
#define PWM_V                        0x08   // w
#define PWM_W                        0x0C   // w
#define PWM_MAX                      0x10   // w
#define PWM_BLOCK                    0x14   // w
#define PWM_TRIGGER_UP               0x18   // w
#define PWM_TRIGGER_DOWN             0x1C   // w

#define PWM_STATUS                   0x20   // r
#define PWM_CARRIER                  0x24   // r

void pwm_setup(int base_address, int pwmmax);

void pwm_update(int base_address, int Vu_PWM, int Vv_PWM, int Vw_PWM);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /*SSG_EMB_PWM_H_*/

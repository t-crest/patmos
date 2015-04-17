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

#ifndef SSG_EMB_DSM_H_
#define SSG_EMB_DSM_H_

/**
 * @file ssg_emb_dsm.h
 *
 * @brief Header file for Drive State Machine interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup DSM
 *
 * @{
 */

//drive state machine
#define SM_CONTROL					 0x00 // rw
#define SM_STATUS					 0x04 // r
#define SM_RESET					 0x04 // w

#define STATUS_REG_ERR_OC       0x01        // Overcurrent
#define STATUS_REG_ERR_OV       0x02        // Overvoltage
#define STATUS_REG_ERR_UV       0x04        // Undervoltage
#define STATUS_REG_ERR_CLOCK    0x08        // Clock
#define STATUS_REG_ERR_IGBT     0x10        // IGBT
#define STATUS_REG_ERR_CHOPPER  0x20        // Chopper
#define STATUS_REG_ERR_STATUS   0x3F        // Status

void dsm_reset(int base_address);
void dsm_reset_to_idle(int base_address);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /*SSG_EMB_DSM_H_*/

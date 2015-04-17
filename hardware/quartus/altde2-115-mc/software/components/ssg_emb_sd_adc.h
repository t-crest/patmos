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

#ifndef SSG_EMB_SD_ADC_H_
#define SSG_EMB_SD_ADC_H_

/**
 * @file ssg_emb_sd_adc.h
 *
 * @brief Header file for ADC interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup SDADC
 *
 * @{
 */

//ADC internal ADR OFFSET
#define ADC_OFFSET_U                 0x04  // w
#define ADC_OFFSET_W                 0x08  // w
#define ADC_I_PEAK                   0x0C  // w
#define ADC_D                        0x10  // w
#define ADC_IRQ_ACK                  0x14  // w

//#define ADC_STATUS                   0x18  // r
#define ADC_I_U                      0x1C  // r
#define ADC_I_W                      0x20  // r
#define ADC_I_PEAK_RD                0x24  // r
#define OC_CAPTURE_U                 0x30  // r
#define OC_CAPTURE_W                 0x34  // r

void adc_setup(int base_address);

void adc_overcurrent_enable(int base_address, int enable);

void adc_read(int base_address, short *Iu, short *Iw);

#define adc_read_u(base_address) ((short)IORD_16DIRECT(base_address,ADC_I_U))
#define adc_read_w(base_address) ((short)IORD_16DIRECT(base_address,ADC_I_W))

/*!
 * @}
 */

/*!
 * @}
 */

#endif /*SSG_EMB_SD_ADC_H_*/

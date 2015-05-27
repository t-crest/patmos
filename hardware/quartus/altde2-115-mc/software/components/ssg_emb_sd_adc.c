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
 * @file ssg_emb_sd_adc.c
 *
 * @brief ADC interface
 */

#include "includes.h"

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup SDADC Sigma-Delta ADC Interface
 *
 * @brief Interface to sigma-delta ADCs
 *
 * The ADCs are used to measure the motor currents
 *
 * @{
 */

void adc_setup(int base_address)
{
    //---------------------------------------------------------------------------------------------------------------------
    //Parameters of ADC
    //---------------------------------------------------------------------------------------------------------------------
    IOWR_16DIRECT(base_address, ADC_OFFSET_U , 32767);
    IOWR_16DIRECT(base_address, ADC_OFFSET_W , 32767);
    IOWR_16DIRECT(base_address, ADC_I_PEAK   , 100  );
    IOWR_16DIRECT(base_address, ADC_D        , 0    );
    IOWR_16DIRECT(base_address, ADC_IRQ_ACK  , 1    );
}

/*
 * Control bits are confusingly named. Bits[1:0] need to be 01 to enable
 */
void adc_overcurrent_enable(int base_address, int enable)
{
    IOWR_16DIRECT(base_address, ADC_D , (enable&1) | IORD_16DIRECT(base_address,ADC_D));
}

void adc_read(int base_address, short *Iu, short *Iw)
{
    //#############################################################################################
    //Normal operation with speed control
    //#############################################################################################
    *Iu = (short) IORD_16DIRECT(base_address,ADC_I_U) ;
    *Iw = (short) IORD_16DIRECT(base_address,ADC_I_W) ;
}

/*!
 * @}
 */

/*!
 * @}
 */

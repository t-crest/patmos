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

#ifndef __INCLUDES_H_
#define __INCLUDES_H_

/**
 * @file 
 *
 * @brief Global include file for Nios II HAL application
 */

#include "system.h"
#include "io.h"

#define __NIOS__
#define MATH_SECTION __attribute__((section(".fast_math")))
#define TABLE_SECTION __attribute__((section(".table_data")))
#define IRQ_SECTION __attribute__((section(".exceptions")))
#define DRIVE_SECTION __attribute__((section (".drive_data")));
#define AVALON_FREQ ALT_CPU_CPU_FREQ

// Base address of buffer memory
#define SYSTEM_CONSOLE_BASE_ADRS	0x08000000

// Macro to convert uC/OS-II task delay system clock rate set to usleep
// Ignores hours and minutes which are never used
#define OSTimeDlyHMSM(h,m,s,l) usleep((s)*1000L*1000L+(l)*1000L)

/**
 * Low level hardware float read
 *
 * @param[in] BASE Base Address
 * @param[in] OFFSET Byte offset
 * @return float Read data
 */
#define IORD_FLOATDIRECT(BASE, OFFSET) \
	*((volatile float*)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET))))

/**
 * Low level hardware float write
 *
 * @param[in] BASE Base Address
 * @param[in] OFFSET Byte offset
 * @param[in] DATA Data to write
 */
#define IOWR_FLOATDIRECT(BASE, OFFSET, DATA) \
	*((volatile float*)(__IO_CALC_ADDRESS_DYNAMIC ((BASE), (OFFSET))))=(DATA)


// STANDARD LIBRARIES
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//  BSP
//#include "sys/alt_irq.h" 
//#include "altera_avalon_jtag_uart_regs.h" 
//#include "altera_avalon_performance_counter.h"

//  APP
// Motor control
#include "sin_cos_q15.h"
#include "mc_dsp.h"
#include "mc_dsp_fpu.h"
#include "mc.h"
#include "mc_debug.h"
#include "mc_pos_fb.h"
//#include "DOC_Single_Axis_dspba_fixp_regs.h"
//#include "DOC_Single_Axis_dspba_floatp_regs.h"
//km #include "platform.h"
#include "motor_task.h"	//km

// Motor control components
#include "ssg_emb_dclink.h"
#include "ssg_emb_dsm.h"
#include "ssg_emb_pwm.h"
#include "ssg_emb_sd_adc.h"
//km #include "endat_api.h"
//km #include "endat.h"
#include "biss_api.h"
#include "biss.h"

//#include "mc_nios_perf.h"

// Patmos specific libraries
#include <machine/exceptions.h>
#include <machine/rtc.h>

#endif	// __INCLUDES_H_


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

#ifndef __MC_DEBUG_H_
#define __MC_DEBUG_H_

/**
 * @file mc_debug.h
 *
 * @brief Header file for interface functions for system console debug GUI
 */

#include "includes.h"

/*!
 * \addtogroup DEBUG System Console Interface
 *
 * @brief Support for debug information via system console GUI and console
 *
 * @{
 */

#define DEBUG_ADDR_SPACE_PER_AXIS       64		//!< Size (32 but words) of debug memory for each axis

/*!
 * \addtogroup DEBUG_MEM_ADDR Debug Memory Map
 *
 * Word (32-bit) addresses for debug memory
 *
 * @{
 */
// (WORD) ADDRESS MAP FOR DEBUG MEMORY
// Write-only registers will polled only by System Console
// Read-only registers (including button registers) will be polled in main loop

// General Drive Status (Write only)
#define DOC_DBG_DRIVE_STATE             0
#define DOC_DBG_RUNTIME                 1
#define DOC_DBG_DSP_MODE                2
#define DOC_DBG_DEMO_MODE               3
#define DOC_DBG_LATENCY                 4
#define DOC_DBG_DUMP_MODE               5
#define DOC_DBG_TRIG_SEL                6
#define DOC_DBG_TRIG_EDGE               7
#define DOC_DBG_TRIG_VALUE              8
#define DOC_DBG_FFT_SELECT              9

// Drive Performance Status (Write only)
#define DOC_DBG_SPEED                   10
#define DOC_DBG_SPEED_FILT              11
#define DOC_DBG_POSITION                12
#define DOC_DBG_V_RMS                   13
#define DOC_DBG_I_RMS                   14
#define DOC_DBG_POWER_INST              15
#define DOC_DBG_POWER_CUM               16

// Drive Command Inputs (Read-only except for buttons handshake and fft status)
#define DOC_DBG_NUM_BUTTONS             8
#define DOC_DBG_BUTTONS                 17
#define DOC_DBG_OL_EN                   (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 1)
#define DOC_DBG_FFT0_PK_DET_LIM         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 2)
#define DOC_DBG_FFT1_PK_DET_LIM         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 3)

#define DOC_DBG_I_PI_TUNE_EN_UN         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 4)
#define DOC_DBG_I_PI_KP                 (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 5)
#define DOC_DBG_I_PI_KI                 (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 6)

#define DOC_DBG_SPEED_PI_TUNE_EN_UN     (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 7)
#define DOC_DBG_SPEED_PI_KP             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 8)
#define DOC_DBG_SPEED_PI_KI             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 9)

#define DOC_DBG_SPEED_SETP0             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 10)
#define DOC_DBG_UNUSED0			        (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 11)
#define DOC_DBG_SPEED_SW_PERIOD_UN      (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 12)

#define DOC_DBG_AXIS_SELECT             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 13)
#define DOC_DBG_FFT_PING_STATUS       (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 14)
#define DOC_DBG_FFT_PONG_STATUS       (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 15)

#define DOC_DBG_POS_SETP0               (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 16)
#define DOC_DBG_POS_SETP1               (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 17)
#define DOC_DBG_POS_SW_PERIOD_UN        (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 18)

#define DOC_DBG_I_PI_FB_LIM             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 19)
#define DOC_DBG_I_PI_OP_LIM             (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 20)

#define DOC_DBG_SPEED_PI_FB_LIM         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 21)
#define DOC_DBG_FILT_DC_GAIN_FL         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 22)

#define DOC_DBG_POS_MODE                (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 23)
#define DOC_DBG_POS_SPEED               (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 24)

#define DOC_DBG_POS_PI_KP               (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 25)
#define DOC_DBG_POS_SPDFF_KP            (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 26)


#define DOC_DBG_CMD_WAVE_PERIOD         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 27)
#define DOC_DBG_CMD_WAVE_OFFSET         (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 28)
#define DOC_DBG_CMD_WAVE_AMP_POS_FL     (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 29)
#define DOC_DBG_CMD_WAVE_AMP_SPD_FL     (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 30)
#define DOC_DBG_CMD_WAVE_TYPE           (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 31)

#define DOC_DBG_FILT_FN_HZ_FL           (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 32)
#define DOC_DBG_FILT_FD_HZ_FL           (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 33)
#define DOC_DBG_FILT_ZN_FL              (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 34)
#define DOC_DBG_FILT_ZD_FL              (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 35)
#define DOC_DBG_FILT_EN                 (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 36)

#define UNUSED_DOC_DBG_FFT_1_PING_STATUS       (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 37)
#define UNUSED_DOC_DBG_FFT_1_PONG_STATUS       (DOC_DBG_NUM_BUTTONS + DOC_DBG_BUTTONS + 38)
/*!
 * @}
 */

/**
 * \addtogroup DEBUG_BUTTON Debug Soft Buttons
 *
 * Soft button encodings for system console GUI buttons
 *
 * @{
 */
#define BUTTON_START            0x01        //!< Start
#define BUTTON_SPEED_DOWN       0x02        //!< Speed down
#define BUTTON_SPEED_UP         0x04        //!< Speed up
#define BUTTON_STOP             0x08        //!< Stop

/**
 * @brief Get system console GUI soft button state from debug memory
 *
 * @param[in] offset		Offset to first soft button location in debug memory
 * @param[in] num_buttons	Maximum number of buttons
 * @param[out] buttons		Pointer to button array
 */
void debug_get_buttons(int offset, int num_buttons, int *buttons);

/**
 * @brief Return current soft button state
 *
 * @param[in] buttons		Button array
 * @param[in] button_num	Button number to test
 *
 * @return	  0 = button not pressed
 */
int debug_button_pressed(int buttons, int button_num);
/*!
 * @}
 */

/*!
 * \addtogroup DEBUG_FUNCTIONS Debug Function Macros
 *
 * Macros for debug memory acces functions
 *
 * @{
 */
//#define debug_base_addr(dn, word_offset) (SYS_CONSOLE_DEBUG_RAM_BASE + (dn * DEBUG_ADDR_SPACE_PER_AXIS * 4) + (word_offset * 4))
//#define debug_write_status(dn, reg_word_addr, value) (IOWR_32DIRECT(debug_base_addr (dn,0),reg_word_addr * 4,value))
//#define debug_write_status_float(dn, reg_word_addr, value) (IOWR_FLOATDIRECT(debug_base_addr (dn,0),reg_word_addr * 4,value))
//#define debug_read_command(dn, reg_word_addr) (IORD_32DIRECT(debug_base_addr (dn,0),reg_word_addr * 4))
//#define debug_read_command_float(dn, reg_word_addr) (IORD_FLOATDIRECT(debug_base_addr (dn,0),reg_word_addr * 4))
#define debug_base_addr(dn, word_offset)
#define debug_write_status(dn, reg_word_addr, value)
#define debug_write_status_float(dn, reg_word_addr, value)
#define debug_read_command(dn, reg_word_addr) 
#define debug_read_command_float(dn, reg_word_addr) 

/*!
 * @}
 */

/**
 * @brief (Re-)initialize drive parameters for one axis
 *
 * @param dn index of axis being initialized
 * @param dp pointer to drive parameters structure
 */
void init_debug (int dn, drive_params * dp);

/**
 * @brief Put new values passed from system console GUI into drive parameters
 *
 * @param dn index of axis being written
 * @param dp pointer to drive parameters structure
 */
void poll_debug (int dn, drive_params * dp);

/**
 * @brief Dump selected data to system console Tcl GUI
 *
 * @param dp			pointer to drive_params struct
 * @param axis_select	axis to be dumped
 */
void dump_data(drive_params * dp, int axis_select);

/*!
 * \addtogroup DEBUG_MSG Debug Message Handling
 *
 * Mechanism to handle debug output to console
 *
 * @{
 */
#define MAX_DEBUG_MSG 256		//!< Maximum debug mesage length
#define MAX_DEBUG_QUEUE 10000	//!< Debug mesage queue depth

/*!
 * \addtogroup DEBUG_FILTER Debug output levels
 *
 * Values used to filter console output
 *
 * @{
 */
#define DBG_ALWAYS		0		//!< Always output
#define DBG_FATAL		10		//!< Fatal error
#define DBG_ERROR		20		//!< Non-fatal error
#define DBG_WARN		30		//!< Warning
#define DBG_INFO		40		//!< Information
#define DBG_PERF		50		//!< Performance data
#define DBG_DEBUG		60		//!< Debug
#define DBG_DEBUG_MORE	70		//!< More debug
#define DBG_ALL			100		//!< Everything
/*!
 * @}
 */

/**
 * @brief Current debug level
 */
extern unsigned int dbg_level;

/**
 * @brief Initialise the debug system
 */
void init_debug_output(void);

/**
 * @brief Thread safe, non-blocking, printf() for debug output to console
 *
 * If the priority filter value is less than the current dbg_level value then the output is added
 * to the queue to be handled later by the debug_task.
 *
 * @param priority	Priority filter value
 * @param format	Format string
 * @return			0 = OK, otherwise output could not be added to queue (queue full)
 */
int debug_printf(unsigned int priority, char *format, ...);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* __MC_DEBUG_H_ */

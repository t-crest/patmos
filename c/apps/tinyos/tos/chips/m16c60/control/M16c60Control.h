/*
 * Copyright (c) 2009 Communication Group and Eislab at
 * Lulea University of Technology
 *
 * Contact: Laurynas Riliskis, LTU
 * Mail: laurynas.riliskis@ltu.se
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of Communication Group at Lulea University of Technology
 *   nor the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * This file includes defines to be used together with the control interfaces
 * for the M16c/60 mcu.
 * 
 * @author Henrik Makitaavola <henrik.makitaavola@gmail.com>
 */

#ifndef __M16C60_CONTROL_H__
#define __M16C60_CONTROL_H__

#define UQ_M16C60_STOP_MODE_CONTROL "UQ_M16C60_STOP_MODE_CONTROL"
#define UQ_M16C60_SYSTEM_CLOCK_CONTROL "UQ_M16C60_SYSTEM_CLOCK_CONTROL"


/**
 * Input to SystemClockControl.minSpeed() and
 * M16c60Control.defaultSystemClock().
 */
typedef enum
{
  M16C60_DONT_CARE = 0x0,
  M16C60_SUB_CLOCK = 0x1,
  M16C60_MAIN_CLOCK_DIV_0 = 0x2,
  M16C60_MAIN_CLOCK_DIV_2 = 0x4,
  M16C60_MAIN_CLOCK_DIV_4 = 0x8,
  M16C60_MAIN_CLOCK_DIV_8 = 0x9,
  M16C60_MAIN_CLOCK_DIV_16 = 0xc,
  M16C60_PLL_CLOCK = 0xd,
} M16c60SystemClock;

/**
 * The different PLL multipliers supported by the M16c/60 mcu.
 */
typedef enum
{
  M16C60_PLL_2 = 0x1,
  M16C60_PLL_4 = 0x2,
  M16C60_PLL_6 = 0x3,
  M16C60_PLL_8 = 0x4
} M16c60PLLMultiplier;

#endif // __M16C60_CONTROL_H__


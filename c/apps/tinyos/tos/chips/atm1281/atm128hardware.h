/*
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.  By
 *  downloading, copying, installing or using the software you agree to
 *  this license.  If you do not agree to this license, do not download,
 *  install, copy or use the software.
 *
 *  Copyright (c) 2004-2005 Crossbow Technology, Inc.
 *  Copyright (c) 2002-2003 Intel Corporation.
 *  Copyright (c) 2000-2003 The Regents of the University  of California.
 *  Copyright (c) 2007, Vanderbilt University
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holder nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  Updated chips/atm128/atm128hardware.h with atm1281's MCU status and
 *  memory control registers.
 *
 *  @author Janos Sallai, Martin Turon, Jason Hill, Philip Levis, Nelson Lee, David Gay
 */


#ifndef _H_atmega128hardware_H
#define _H_atmega128hardware_H

#include <avr/io.h>
#if __AVR_LIBC_VERSION__ >= 10400UL
#include <avr/interrupt.h>
#else
#include <avr/interrupt.h>
#include <avr/signal.h>
#endif
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "atm128const.h"

/* We need slightly different defs than SIGNAL, INTERRUPT */
#define AVR_ATOMIC_HANDLER(signame) \
  void signame() __attribute__ ((signal)) @atomic_hwevent() @C()

#define AVR_NONATOMIC_HANDLER(signame) \
  void signame() __attribute__ ((interrupt)) @hwevent() @C()

/* Macro to create union casting functions. */
#define DEFINE_UNION_CAST(func_name, from_type, to_type) \
  to_type func_name(from_type x) { \
  union {from_type f; to_type t;} c = {f:x}; return c.t; }

// Bit operators using bit number
#define SET_BIT(port, bit)    ((port) |= _BV(bit))
#define CLR_BIT(port, bit)    ((port) &= ~_BV(bit))
#define READ_BIT(port, bit)   (((port) & _BV(bit)) != 0)
#define FLIP_BIT(port, bit)   ((port) ^= _BV(bit))
#define WRITE_BIT(port, bit, value) \
   if (value) SET_BIT((port), (bit)); \
   else CLR_BIT((port), (bit))

// Bit operators using bit flag mask
#define SET_FLAG(port, flag)  ((port) |= (flag))
#define CLR_FLAG(port, flag)  ((port) &= ~(flag))
#define READ_FLAG(port, flag) ((port) & (flag))

/* Enables interrupts. */
inline void __nesc_enable_interrupt() @safe() {
    sei();
}
/* Disables all interrupts. */
inline void __nesc_disable_interrupt() @safe() {
    cli();
}

/* Defines data type for storing interrupt mask state during atomic. */
typedef uint8_t __nesc_atomic_t;
__nesc_atomic_t __nesc_atomic_start(void);
void __nesc_atomic_end(__nesc_atomic_t original_SREG);

#ifndef NESC_BUILD_BINARY
/* @spontaneous() functions should not be included when NESC_BUILD_BINARY
   is #defined, to avoid duplicate functions definitions wheb binary
   components are used. Such functions do need a prototype in all cases,
   though. */

/* Saves current interrupt mask state and disables interrupts. */
inline __nesc_atomic_t
__nesc_atomic_start(void) @spontaneous() @safe()
{
    __nesc_atomic_t result = SREG;
    __nesc_disable_interrupt();
    asm volatile("" : : : "memory"); /* ensure atomic section effect visibility */
    return result;
}

/* Restores interrupt mask to original state. */
inline void
__nesc_atomic_end(__nesc_atomic_t original_SREG) @spontaneous() @safe()
{
  asm volatile("" : : : "memory"); /* ensure atomic section effect visibility */
  SREG = original_SREG;
}
#endif

/* Defines the mcu_power_t type for atm128 power management. */
typedef uint8_t mcu_power_t @combine("mcombine");


enum {
  ATM128_POWER_IDLE        = 0,
  ATM128_POWER_ADC_NR      = 1,
  ATM128_POWER_EXT_STANDBY = 2,
  ATM128_POWER_SAVE        = 3,
  ATM128_POWER_STANDBY     = 4,
  ATM128_POWER_DOWN        = 5,
};

/* Combine function.  */
mcu_power_t mcombine(mcu_power_t m1, mcu_power_t m2) @safe() {
  return (m1 < m2)? m1: m2;
}

/* MCU Status Register*/
typedef struct
{
  uint8_t porf  : 1;    //!< Power-on Reset Flag
  uint8_t extrf : 1;    //!< External Reset Flag
  uint8_t borf  : 1;    //!< Brown-out Reset Flag
  uint8_t wdrf  : 1;    //!< Watchdog Reset Flag
  uint8_t jtrf  : 1;    //!< JTAG Reset Flag
  uint8_t resv1 : 3;    //!< Reserved
} Atm128_MCUSR_t;

/* External Memory Control Register A*/
typedef struct
{
  uint8_t srw00 : 1;    //!< Wait-state Select Bits for Lower Sector
  uint8_t srw01 : 1;    //!< Wait-state Select Bits for Lower Sector
  uint8_t srw10 : 1;    //!< Wait-state Select Bits for Upper Sector
  uint8_t srw11 : 1;    //!< Wait-state Select Bits for Upper Sector
  uint8_t srl   : 3;    //!< Wait-state Sector Limit
  uint8_t sre   : 1;    //!< External SRAM/XMEM Enable
} Atm128_XMCRA_t;

/* External Memory Control Register B*/
typedef struct
{
  uint8_t xmm    : 3;    //!< External Memory High Mask
  uint8_t resv1  : 4;    //!< Reserved
  uint8_t xmbk   : 1;    //!< External Memory Bus-keeper Enable
} Atm128_XMCRB_t;

/* Floating-point network-type support.
   These functions must convert to/from a 32-bit big-endian integer that follows
   the layout of Java's java.lang.float.floatToRawIntBits method.
   Conveniently, for the AVR family, this is a straight byte copy...
*/

typedef float nx_float __attribute__((nx_base_be(afloat)));

inline float __nesc_ntoh_afloat(const void *COUNT(sizeof(float)) source) @safe() {
  float f;
  memcpy(&f, source, sizeof(float));
  return f;
}

inline float __nesc_hton_afloat(void *COUNT(sizeof(float)) target, float value) @safe() {
  memcpy(target, &value, sizeof(float));
  return value;
}

#endif //_H_atmega128hardware_H

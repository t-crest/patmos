// $Id: avrhardware.h,v 1.4 2010-06-29 22:07:50 scipio Exp $

/*
 * Copyright (c) 2000-2003 The Regents of the University  of California.  
 * All rights reserved.
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
 * - Neither the name of the copyright holders nor the names of
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
 *
 * Copyright (c) 2002-2003 Intel Corporation
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE     
 * file. If you do not find these files, copies can be found by writing to
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300, Berkeley, CA, 
 * 94704.  Attention:  Intel License Inquiry.
 */
/*
 *
 * Authors:             Jason Hill, Philip Levis, Nelson Lee
 *
 *
 */

/**
 * @author Jason Hill
 * @author Philip Levis
 * @author Nelson Lee
 */


#ifndef TOSH_AVRHARDWARE_H
#define TOSH_AVRHARDWARE_H

// check for a new-look avr-libc
#if defined(DTOSTR_ALWAYS_SIGN) && !defined(TOSH_NEW_AVRLIBC)
#define TOSH_NEW_AVRLIBC
#endif

#ifdef TOSH_NEW_AVRLIBC
#include <avr/io.h>
#if __AVR_LIBC_VERSION__ >= 10400UL
#include <avr/interrupt.h>
#else
#include <avr/interrupt.h>
#include <avr/signal.h>
#endif
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#ifndef sbi
/* avr-libc 1.2.3 doesn't include these anymore. */
#define sbi(port, bit) ((port) |= _BV(bit))
#define cbi(port, bit) ((port) &= ~_BV(bit))
#define inp(port) (port)
#define inb(port) (port)
#define outp(value, port) ((port) = (value))
#define outb(port, value) ((port) = (value))
#define inw(port) (*(volatile uint16_t *)&(port))
#define outw(port, value) ((*(volatile uint16_t *)&(port)) = (value))
#define PRG_RDB(addr) pgm_read_byte(addr)
#endif

#else
#include <io.h>
#include <sig-avr.h>
#include <interrupt.h>
#include <wdt.h>
#include <pgmspace.h>
#endif /* TOSH_NEW_AVRLIBC */

// check for version 3.3 of GNU gcc or later
#if ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3))
#define __outw(val, port) outw(port, val);
#endif

#ifndef __inw
#ifndef __SFR_OFFSET
#define __SFR_OFFSET 0
#endif /* !__SFR_OFFSET */
#define __inw(_port) inw(_port)

#define __inw_atomic(__sfrport) ({				\
	uint16_t __t;					\
	bool bStatus;					\
	bStatus = bit_is_set(SREG,7);			\
	cli();						\
	__t = inw(__sfrport);				\
	if (bStatus) sei();				\
	__t;						\
 })

#endif /* __inw */

#define TOSH_ASSIGN_PIN(name, port, bit) \
static inline void TOSH_SET_##name##_PIN() {sbi(PORT##port , bit);} \
static inline void TOSH_CLR_##name##_PIN() {cbi(PORT##port , bit);} \
static inline int TOSH_READ_##name##_PIN() \
  {return (inp(PIN##port) & (1 << bit)) != 0;} \
static inline void TOSH_MAKE_##name##_OUTPUT() {sbi(DDR##port , bit);} \
static inline void TOSH_MAKE_##name##_INPUT() {cbi(DDR##port , bit);} 

#define TOSH_ASSIGN_OUTPUT_ONLY_PIN(name, port, bit) \
static inline void TOSH_SET_##name##_PIN() {sbi(PORT##port , bit);} \
static inline void TOSH_CLR_##name##_PIN() {cbi(PORT##port , bit);} \
static inline void TOSH_MAKE_##name##_OUTPUT() {;} 

#define TOSH_ALIAS_OUTPUT_ONLY_PIN(alias, connector)\
static inline void TOSH_SET_##alias##_PIN() {TOSH_SET_##connector##_PIN();} \
static inline void TOSH_CLR_##alias##_PIN() {TOSH_CLR_##connector##_PIN();} \
static inline void TOSH_MAKE_##alias##_OUTPUT() {} \

#define TOSH_ALIAS_PIN(alias, connector) \
static inline void TOSH_SET_##alias##_PIN() {TOSH_SET_##connector##_PIN();} \
static inline void TOSH_CLR_##alias##_PIN() {TOSH_CLR_##connector##_PIN();} \
static inline char TOSH_READ_##alias##_PIN() {return TOSH_READ_##connector##_PIN();} \
static inline void TOSH_MAKE_##alias##_OUTPUT() {TOSH_MAKE_##connector##_OUTPUT();} \
static inline void TOSH_MAKE_##alias##_INPUT()  {TOSH_MAKE_##connector##_INPUT();} 

// We need slightly different defs than SIGNAL, INTERRUPT
#define TOSH_SIGNAL(signame)					\
  void signame() __attribute__ ((signal)) @spontaneous() @C()

#define TOSH_INTERRUPT(signame)				\
  void signame() __attribute__ ((interrupt)) @spontaneous() @C()

/* Watchdog Prescaler
 */
enum {
  TOSH_period16 = 0x00, // 47ms
  TOSH_period32 = 0x01, // 94ms
  TOSH_period64 = 0x02, // 0.19s
  TOSH_period128 = 0x03, // 0.38s
  TOSH_period256 = 0x04, // 0.75s
  TOSH_period512 = 0x05, // 1.5s
  TOSH_period1024 = 0x06, // 3.0s
  TOSH_period2048 = 0x07 // 6.0s
};

void TOSH_wait()
{
  asm volatile("nop");
  asm volatile("nop");
}

// atomic statement runtime support

/* typedef uint8_t __nesc_atomic_t; */

/* __nesc_atomic_t __nesc_atomic_start(void); */
/* void __nesc_atomic_end(__nesc_atomic_t oldSreg); */

/* #ifndef NESC_BUILD_BINARY */

/* inline __nesc_atomic_t __nesc_atomic_start(void) @spontaneous() */
/* { */
/*   __nesc_atomic_t result = inp(SREG); */
/*   cli(); */
/*   return result; */
/* } */

/* inline void __nesc_atomic_end(__nesc_atomic_t oldSreg) @spontaneous() */
/* { */
/*   outp(oldSreg, SREG); */
/* } */

/* #endif */

/* inline void __nesc_atomic_sleep() */
/* { */
/*   /\* Atomically enable interrupts and sleep *\/ */
/*   sei();  // Make sure interrupts are on, so we can wake up! */
/*   asm volatile ("sleep"); */
/*   TOSH_wait(); */
/* } */


/* inline void __nesc_enable_interrupt() { */
/*   sei(); */
/* } */

/* inline void __nesc_disable_interrupt() { */
/*   cli(); */
/* } */

#endif //TOSH_AVRHARDWARE_H

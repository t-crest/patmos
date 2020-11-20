/*
 * Copyright (c) 2011 Lulea University of Technology
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
 */

/**
 * @author Henrik Makitaavola <henrik.makitaavola@gmail.com>
 */

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <m16c60hardware.h>
#include <m16chardware.h>

typedef uint32_t in_flash_addr_t;
typedef uint32_t ex_flash_addr_t;

static inline void wait( uint16_t dt ) {
  uint16_t i;
  for (i = 0; i < dt; ++i) {
    TOSH_wait();
  }
}

// LED assignments
TOSH_ASSIGN_PIN(RED_LED, 3, 6);
TOSH_ASSIGN_PIN(GREEN_LED, 3, 7);
TOSH_ASSIGN_PIN(YELLOW_LED, 3, 4);

// Flash assignments
TOSH_ASSIGN_PIN(FLASH_IN,  4, 0);
TOSH_ASSIGN_PIN(FLASH_OUT,  4, 1);
TOSH_ASSIGN_PIN(FLASH_CLK,  4, 2);
TOSH_ASSIGN_PIN(FLASH_CS, 4, 5);
TOSH_ASSIGN_PIN(FLASH_VCC, 3, 2);

void TOSH_SET_PIN_DIRECTIONS(void)
{
  TOSH_MAKE_RED_LED_OUTPUT();
  TOSH_MAKE_YELLOW_LED_OUTPUT();
  TOSH_MAKE_GREEN_LED_OUTPUT();
      
  TOSH_MAKE_FLASH_VCC_OUTPUT();
  TOSH_CLR_FLASH_VCC_PIN();
}

// TODO(henrik) Insert correct value
enum {
  VTHRESH = 0x0, // 0V
};

#endif  // __HARDWARE_H__





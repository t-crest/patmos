/*
 * Copyright (c) 2009 Stanford University.
 * All rights reserved.
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
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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
 * Definitions of stuff specific to the SAM3U-EK board.
 * Includes definitions for the SAM3U MCU.
 *
 * @author Wanja Hofer <wanja@cs.fau.de>
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#define __NEED_BKPT__
#define __NEED_NOP__

#include "sam3uhardware.h"

// #define this so we don't doubly define time_t with conflicting types
// in SD card implementation Time.h file
#define __time_t_defined
#include <sys/types.h>

#ifndef PLATFORM_BAUDRATE
#define PLATFORM_BAUDRATE (9600)
#endif

#define IRQ_PRIO_UDPHS    (0x81)
#define IRQ_PRIO_TWI1     (0x82)
#define IRQ_PRIO_TWI0     (0x83)
#define IRQ_PRIO_DMAC     (0x84)
#define IRQ_PRIO_ADC12B   (0x85)
#define IRQ_PRIO_PIO      (0x86)
#define IRQ_PRIO_SPI      (0x87)
#define IRQ_PRIO_UART     (0x88)
#define IRQ_PRIO_USART0   (0x89)
#define IRQ_PRIO_USART1   (0x90)
#define IRQ_PRIO_USART2   (0x91)
#define IRQ_PRIO_HSMCI    (0x92)

#endif // HARDWARE_H

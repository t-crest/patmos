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
 * M16c60 UART typedefs.
 * 
 * @author Henrik Makitaavola <henrik.makitaavola@gmail.com>
 */

#ifndef __M16C60_UART_H__
#define __M16C60_UART_H__

typedef enum
{
  M16C60_UART_MODE_OFF,
  M16C60_UART_MODE_UART_8BITS,
  M16C60_UART_MODE_SPI0,
  M16C60_UART_MODE_SPI1,
  M16C60_UART_MODE_SPI2,
  M16C60_UART_MODE_SPI3,
  M16C60_UART_MODE_I2C,
} m16c60_uart_mode;

typedef enum
{
  M16C60_UART_COUNT_SOURCE_F1_2 = 0x0,
  M16C60_UART_COUNT_SOURCE_F8 = 0x1,
  M16C60_UART_COUNT_SOURCE_F32 = 0x2,
} m16c60_uart_count_source;

typedef enum {
  TOS_UART_300,
  TOS_UART_600,
  TOS_UART_1200,
  TOS_UART_2400,
  TOS_UART_4800,
  TOS_UART_9600,
  TOS_UART_19200,
  TOS_UART_38400,
  TOS_UART_57600,
} uart_speed_t;

typedef enum {
  TOS_UART_OFF,
  TOS_UART_RONLY,
  TOS_UART_TONLY,
  TOS_UART_DUPLEX
} uart_duplex_t;

typedef enum {
  TOS_UART_PARITY_NONE,
  TOS_UART_PARITY_EVEN,
  TOS_UART_PARITY_ODD
} uart_parity_t;

typedef enum {
  TOS_UART_STOP_BITS_1,
  TOS_UART_STOP_BITS_2
} uart_stop_bits_t;


#endif  // __M16C60_UART_H__


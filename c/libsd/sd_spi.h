/*
   Copyright 2017 Max Rishoej Pedersen
   All rights reserved.

   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * Driver for SD host controller using the SPI protocol.
 *
 * Authors: Max Rishoej (maxrishoej@gmail.com)
 */

#ifndef SD_SPI_DRIVER_H_
#define SD_SPI_DRIVER_H_

#include <stdint.h>

// Error codes
typedef enum {
  SD_SUCCESS,
  SD_BADRES,
  SD_CLK_DIVISOR, // Clock rate not an even divisor
  SD_CLK_HIGH,    // Clock rate too high
  SD_GOIDLE,      // Could not GO_IDLE
  SD_TIMEOUT,
  SD_SENDIFCOND_BADRES,
  SD_SENDOPCOND_BADRES,
  SD_SENDOPCOND_UNABLE,
  SD_CRC_BADRES,
  SD_BLKLEN_BADRES,
  SD_BADCRC,
  SD_WR,
} SDErr;

// Last response from SD Card
// TODO: Fill in missing types
typedef union {
  uint8_t r1;
  uint8_t r7[5];
} SDResponse;

// --- SPI related ---
SDErr spi_set_clockrate(const uint32_t rate);
uint8_t spi_send(const uint8_t dat);
void spi_clear();

// --- SD related ---
typedef struct {
  uint32_t block_sz;
} SDInfo;

SDErr sd_init();
SDErr sd_read_single_block(uint32_t addr, uint8_t *buffer, uint32_t block_sz);
SDErr sd_write_single_block(uint32_t addr, uint8_t *buffer, uint32_t block_sz);
SDErr sd_info();

uint8_t sd_cmd(uint8_t cmd, uint8_t arg0, uint8_t arg1,
               uint8_t arg2, uint8_t arg3, uint8_t crc);

// Debug methods
void sd_print_block(uint8_t *block, uint32_t block_sz);
void sd_print_r1(uint8_t r);

#endif

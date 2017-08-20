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

#include <stdio.h> // Just for debugging
#include <stdint.h>
#include <machine/spm.h>
#include <machine/rtc.h>

#include "sd_spi.h"

// Pointers to host controller registers
volatile _SPM int *sd_ptr        = (volatile _SPM int *) 0xF00b0000;
volatile _SPM int *sd_cs_ptr     = (volatile _SPM int *) 0xF00b0004;
volatile _SPM int *sd_en_ptr     = (volatile _SPM int *) 0xF00b0008;
volatile _SPM int *sd_clkdiv_ptr = (volatile _SPM int *) 0xF00b000c;

// Timeout limits when waiting for card
// No reason not to have them fairly large
#define ACMD41_MAX_TRIES  (1000)
#define SD_WRITE_MAX_WAIT (1000000)
#define SD_READ_MAX_WAIT  (1000000)
#define SD_BUSY_MAX_WAIT  (1000000)

#define SD_DATA_BEGIN_TOKEN (0xFE)

// Global SD info
SDInfo sd_global_info;
SDResponse sd_res; // Last response from card

// TODO: Bound loops
uint8_t spi_send(const uint8_t dat) {
  while(*sd_en_ptr != 0);
  *sd_ptr = (int)dat;
  while(*sd_en_ptr != 0);
  return *sd_ptr;
}

// Sets the clock rate of the controller
SDErr spi_set_clockrate(const uint32_t rate) {
  uint32_t patmos_rate = get_cpu_freq();

  if (patmos_rate % rate != 0) {
    /*
    printf("Invalid clock rate (%ld)! Must be divisor of CPU rate (%ld)\n",
           rate, patmos_rate);
    */
    return SD_CLK_DIVISOR;
  }
  if (rate > patmos_rate / 2) {
    /*
    printf("Invalid clock rate (%ld)! Must be smaller than or equal to"
           "half the CPU rate (%ld)\n", rate, patmos_rate);
    */
    return SD_CLK_HIGH;
  }

  uint32_t clkdiv = (patmos_rate / rate) / 2;
  while(*sd_en_ptr != 0);
  *sd_clkdiv_ptr = clkdiv;

  return SD_SUCCESS;
}

// Clears SPI buffers
void spi_clear() {
  *sd_cs_ptr = 1;
  spi_send(0xFF);
  *sd_cs_ptr = 0;
}

// Sends a SD command, returning the 1-byte response (R1)
uint8_t sd_cmd(uint8_t cmd, uint8_t arg0, uint8_t arg1,
               uint8_t arg2, uint8_t arg3, uint8_t crc)
{
  spi_clear(); // Ensure SPI buffers of SD card are cleared

  spi_send(cmd | 0b01000000); // Commands always begin with 01
  spi_send(arg0);
  spi_send(arg1);
  spi_send(arg2);
  spi_send(arg3);
  spi_send(crc | 0b00000001); // Commands always end with 1

  // Wait for response
  uint8_t response; // R1 response
  int n;
  for (n = 0; n < 10; n++) {
    response = spi_send(0xFF);
    if (response != 0xFF)
      break;
  }
  return response; // 0xFF if no response received
}

// Sends ACMD41 / SD_SEND_OP_COND
// hcs =  0 -> Initialize without HCS
// hcs != 0 -> Initialize with HCS
SDErr sd_send_op_cond_cmd(uint8_t hcs) {
  int i = 0;

  hcs = hcs == 0 ? 0x00 : 0x40; // Adjust parameter

  for (i = 0; i < ACMD41_MAX_TRIES; i++) {
    sd_res.r1 = sd_cmd(55, 0, 0, 0, 0, 0xFF); // APP_CMD
    sd_res.r1 = sd_cmd(41, hcs, 0, 0, 0, 0xFF); // ACMD41 (SD_SEND_OP_COND)

    if (sd_res.r1 == 0) { // Success. Card initialized.
      /*
      printf("ACMD41 successful after %d tries %s HCS\n", i,
             hcs ? "with" : "without");
      */
      return SD_SUCCESS;
    }
    else if (sd_res.r1 != 0x01) { // Bad response.
      /*
      printf("Error: %02X (%d)\n", res, i);
      sd_print_r1(res);
      */
      return SD_SENDOPCOND_BADRES;
    }
  }

  return SD_SENDOPCOND_UNABLE;
}

// (DEBUG) Prints a block of data in hex
void sd_print_block(uint8_t *block, uint32_t block_sz) {
  int w = 16; // Uint8_Ts per rows
  int h = block_sz / w;

  int i, j;
  for (i = 0; i < h; i++) {
    printf("%03d: ", i*w);
    for (j = 0; j < w; j++) {
      printf("%02X ", block[i*w + j]);
    }
    printf("\n");
  }
}

// (DEBUG) Prints a quickly understandable format of a R1 response
void sd_print_r1(uint8_t r) {
  if ((r & 0x80) != 0) printf("Invalid ");
  if ((r & 0x40) != 0) printf("Param ");
  if ((r & 0x20) != 0) printf("Addr ");
  if ((r & 0x10) != 0) printf("EraseSeq ");
  if ((r & 0x08) != 0) printf("Crc ");
  if ((r & 0x04) != 0) printf("Cmd ");
  if ((r & 0x02) != 0) printf("EraseReset ");
  if ((r & 0x01) != 0) printf("Idle");
  printf("\n");
}

// Reads a block of data from the card using CMD17 (READ_SINGLE_BLOCK)
SDErr sd_read_single_block(uint32_t addr, uint8_t *buffer, uint32_t block_sz) {
  int i;

  // Send CMD17 (READ_SINGLE_BLOCK)
  // Assumes CRC disabled
  sd_res.r1 = sd_cmd(17, addr >> 24, addr >> 16, addr >> 8, addr, 0xFF);
  if (sd_res.r1 != 0) {
    return SD_BADRES;
  }

  // Wait for data begin token
  for (i = 0; i < SD_READ_MAX_WAIT; i++) {
    sd_res.r1 = spi_send(0xFF);
    if (sd_res.r1 == SD_DATA_BEGIN_TOKEN)
      break;
  }
  if (i == SD_READ_MAX_WAIT) {
    return SD_TIMEOUT;
  }

  // Read data block
  for (i = 0; i < block_sz; i++) {
    buffer[i] = spi_send(0xFF); // Exchange byte
  }

  // Read CRC16
  spi_send(0xFF);
  spi_send(0xFF);

  return SD_SUCCESS;
}

// Writes a block of data to the card using CMD24 (WRITE_BLOCK)
SDErr sd_write_single_block(uint32_t addr, uint8_t *buffer, uint32_t block_sz) {
  int i;

  // Send CMD24 (WRITE_BLOCK)
  // Assumes CRC disabled
  sd_res.r1 = sd_cmd(24, addr >> 24, addr >> 16, addr >> 8, addr, 0xFF);
  if (sd_res.r1 != 0) {
    return SD_BADRES;
  }

  // Send a Start Block Token
  sd_res.r1 = spi_send(SD_DATA_BEGIN_TOKEN);

  // Send data block
  for (i = 0; i < block_sz; i++) {
    spi_send(buffer[i]);
  }

  // Wait for data response token
  for (i = 0; i < SD_WRITE_MAX_WAIT; i++) {
    sd_res.r1 = spi_send(0xFF);
    if (sd_res.r1 != 0xFF) // Wait for a response token
      break;
  }
  if (i == SD_WRITE_MAX_WAIT) {
    //printf("Timed out waiting for response!\n");
    return SD_TIMEOUT;
  }

  // Parse data response token
  switch (sd_res.r1 & 0x1F) {
  case 0b00101: // Data accepted
    break;
  case 0b01011: // Data rejected due to CRC
    return SD_BADCRC;
  case 0b01101: // Data rejected due to Write Error
    return SD_WR;
  default:
    //printf("Bad data response: %02X\n", res);
    return SD_BADRES;
  }

  // Read busy
  for (i = 0; i < SD_BUSY_MAX_WAIT; i++) {
    sd_res.r1 = spi_send(0xFF);
    if (sd_res.r1 != 0x00) // Busy holds the data line low
      break;
  }
  if (i == SD_BUSY_MAX_WAIT) {
    //printf("Timed out waiting for non-busy!\n");
    return SD_TIMEOUT;
  }

  return SD_SUCCESS;
}

SDErr sd_init() {
  int i;

  // Set clock rate to 400kHz
  SDErr errsv = spi_set_clockrate(400000); // Save so available for printing
  if (errsv != SD_SUCCESS) {
    return errsv;
  }

  // Set CS high for wake-up phase
  *sd_cs_ptr = 1;

  // Send 80 dummy bits == 10 dummy bytes
  for (i = 0; i < 10; i++) {
    spi_send(0xFF);
  }

  // Set CS low to begin sending commands
  *sd_cs_ptr = 0;

  // Send GO_IDLE
  //printf("Initializing card\n");
  sd_res.r1 = sd_cmd(0, 0, 0, 0, 0, 0x94); // GO_IDLE
  if (sd_res.r1 != 1) {
    //printf("Error: %02X\n", sd_res.r1);
    return SD_GOIDLE;
  }

  //printf("Card entered SPI Mode\n");

  // Send CMD8 (SEND_IF_COND)
  const uint8_t volt_range = 0x01; // 0x01 = 2.7V - 3.6V
  const uint8_t check_pattern = 0xAA; // Constant
  //printf("Sending CMD8\n");
  sd_res.r7[0] = sd_cmd(8, 0, 0, volt_range, check_pattern, 0x86);
  //printf("Initial: ");
  //sd_print_r1(sd_res.r7[0]);
  for (i = 0; i < 4; i++) {
    sd_res.r7[1 + i] = spi_send(0xFF);
    //printf("%02X ", sd_res.r7[1 + i]);
  }
  //printf("\n");

  // Verify response
  if (sd_res.r7[0] != 0x01 ||
      sd_res.r7[1] != 0x00 || sd_res.r7[2] != 0x00 ||
      sd_res.r7[3] != volt_range || sd_res.r7[4] != check_pattern) {

    /*
    printf("Invalid R7 response to CMD8: ");
    for (i = 0; i < 5; i++) {
      printf("%02X ", sd_res.r7[i]);
    }
    printf("\n");
    */

    return SD_SENDIFCOND_BADRES;
  }

  // Send ACMD41  (SD_SEND_OP_COND)
  //printf("Sending ACMD41\n");

  // First try with HCS
  if (SD_SUCCESS != sd_send_op_cond_cmd(1)) {
    errsv = sd_send_op_cond_cmd(0); // On failure, try without
    if (SD_SUCCESS != errsv) {
      //printf("Unable to initialize card\n");
      return errsv;
    }
  }

  //printf("Card initialized to Data Transfer Mode\n");

  // Disable CRC - CMD59
  /*
  printf("Disabling CRC\n");
  sd_res.r1 = sd_cmd(59, 0, 0, 0, 0, 0xFF);
  if (sd_res.r1 != 0) {
    printf("Unable to disable CRC: ");
    sd_print_r1(sd_res.r1);
    printf("\n");
    return SD_CRC_BADRES;
  }
  */

  // TODO: Read OCR of card (CMD58)

  // Set block length - CMD16
  //printf("Setting block length to 512 bytes\n");
  sd_res.r1 = sd_cmd(16, 0, 0, 0x02, 0, 0xFF); // Block Length = 512 bytes = 0x0200
  if (sd_res.r1 != 0) {
    /*
    printf("Unable to set block length: ");
    sd_print_r1(res);
    printf("\n");
    */
    return SD_BLKLEN_BADRES;
  }
  sd_global_info.block_sz = 512;

  // Increase clock rate to maximum
  errsv = spi_set_clockrate(20000000); // 20MHz
  if (errsv != SD_SUCCESS) {
    //printf("Unable to increase clock rate");
    return errsv;
  }

  return SD_SUCCESS;
}

SDErr sd_info(SDInfo *sdinfo) {
  *sdinfo = sd_global_info;
  return SD_SUCCESS;
}

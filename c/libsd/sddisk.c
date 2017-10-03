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
 * Generic disk interface for use with file system module.
 * Uses SD card as disk.
 *
 * Authors: Max Rishoej (maxrishoej@gmail.com)
 */

#include "sddisk.h"
#include "sd_spi.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

// Global disk info
DiskInfo disk_global_info = { .initialized = 0 };

// Initializes disk
// Returns 0 on success, -1 on failure
int disk_init() {
  // NOTE: In case of failure inspect res which is an SDErr (sd_spi.h)
  int res = sd_init();
  if (SD_SUCCESS != res) {
    errno = EIO;
    return -1;
  }

  SDInfo sdinfo;
  sd_info(&sdinfo);
  disk_global_info.initialized = 1;
  disk_global_info.block_sz = sdinfo.block_sz;

  errno = 0;
  return 0;
}

// Copies over disk info
int disk_info(DiskInfo *dinfo) {
  *dinfo = disk_global_info;
  return disk_global_info.initialized ? 0 : 1;
}

// Read a number of blocks from the disk
// Returns 0 on success, -1 on failure
int disk_read(uint32_t block, uint8_t *buf, uint32_t num_blocks) {
  const uint32_t blksz = disk_global_info.block_sz;
  uint32_t i;

  // Read all whole sectors
  for (i = 0; i < num_blocks; i++) {
    if (SD_SUCCESS != sd_read_single_block(block + i, buf + i * blksz, blksz)) {
      errno = EIO;
      return -1;
    }
  }

  errno = 0;
  return 0;
}

// Write a number of blocks to the disk
// Returns 0 on success, -1 on failure
int disk_write(uint32_t block, uint8_t *buf, uint32_t num_blocks) {
  SDErr res = SD_SUCCESS;
  const uint32_t blksz = disk_global_info.block_sz;
  uint32_t i;

  // Write all whole sectors
  for (i = 0; i < num_blocks; i++) {
    res = sd_write_single_block(block + i, buf + i * blksz, blksz);
    if (SD_SUCCESS != res) {
      errno = EIO;
      return -1;
    }
  }

  errno = 0;
  return 0;
}

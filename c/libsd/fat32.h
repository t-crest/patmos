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
 * Library for accessing files on FAT32 formatted disk.
 *
 * Authors: Max Rishoej (maxrishoej@gmail.com)
 */

#ifndef FAT32_H_
#define FAT32_H_

#include <stdint.h>
#include <sys/types.h>

typedef struct {
  // Partition
  uint32_t vol_begin_addr;

  // General FAT
  uint32_t bytes_per_sector;
  uint32_t sectors_per_cluster;
  uint32_t num_res_sectors;
  uint32_t num_fats;
  uint32_t num_dir_entries;
  uint32_t num_total_sectors;
  uint32_t media_desc_type;
  uint32_t sectors_per_fat;

  // FAT32 only
  uint32_t root_dir_cluster;

  // Derived
  uint32_t fat_begin_addr;
  uint32_t cluster_begin_addr;
  uint32_t entries_per_sector;

  // Calculate address of root dir
  uint32_t root_dir_addr;
} FatPartitionInfo;

// Index of a directory entry
typedef struct {
  uint32_t cluster;
  uint32_t sector;
  uint32_t index;
} FatDirEntryIdx;

// File on a FAT32 volume
typedef struct {
  uint8_t free; // Is the file free?
  uint32_t pos; // Seek position in file
  FatDirEntryIdx dir_idx; // Index of the file
  uint32_t start_cluster; // First cluster of file. Stored for speed.
  uint32_t current_cluster; // Current cluster that pos is in
  uint32_t size; // Size of file. Could be read from dir_idx
} FatFile;

int fat_load_partition_info(uint8_t idx, FatPartitionInfo *pinfo);
int fat_load_first_partition_info(FatPartitionInfo *pinfo);

int fat_init(const FatPartitionInfo *pinfo);

int fat_open(const char *path, int oflag);
int fat_close(int fd);

int fat_write(int fd, uint8_t *buf, uint32_t sz);
int fat_read(int fd, uint8_t *buf, uint32_t sz);

off_t fat_lseek(int fd, off_t pos, int whence);

int fat_unlink(const char *path);

int fat_mkdir(const char *path);
int fat_rmdir(const char *path);

#endif

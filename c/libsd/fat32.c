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

#include "fat32.h"
#include "sddisk.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>

#include <stdio.h>

// --- Defines ---

// Internally useful exit codes
#define FAT_SUCCESS (0)
#define FAT_FAIL (-1)

// Constants relevant for reading partition information
#define PTABLE_BEGIN (446)
#define PTABLE_ENTRY_SIZE (16)
#define PTABLE_TYPE_OFFSET (4)
#define PTABLE_VOL_ADDR_OFFSET (8)

// Supported parition type
#define FAT_PARTITION_SIG_CHS (0x0B)
#define FAT_PARTITION_SIG_LBA (0x0C)

// Maximum number of open files at a time
#define FAT_MAX_FILES (16)

#define FAT_MAX_SHORT_PATH (81) // 80 path + trailing NUL
#define FAT_MAX_LONG_PATH (261) // 260 path + trailing NUL
#define FAT_MAX_LONG_NAME (255) // No trailing NUL

// Offset of file descriptors, to avoid clashing with stdin / stdout / stderr
#define FAT_FD_OFFSET (3)

// Values of directory entries
#define FAT_DIR_ENTRY_FREE (0xE5)
#define FAT_DIR_ENTRY_LAST (0x00)
#define FAT_LAST_LONG      (0x40)

// Macros for checking FAT table values
#define FAT_TV_LAST(v) ((v) >= 0x0FFFFFF8)
#define FAT_TV_BAD(v) ((v) == 0x0FFFFFF7)
#define FAT_TV_FREE(v) ((v) == 0)

#define FAT_DIR_ENTRY_WIDTH (32) // Width of a directory entry in bytes

// Attributes of directory entries
#define FAT_DIRECTORY (0x10)
#define FAT_ARCHIVE   (0x20)

const uint32_t FAT_LONG_NAME_ENTRY = 0
  | 0x01  // Read Only
  | 0x02  // Hidden
  | 0x04  // System
  | 0x08; // VolumeID

// --- Fields ---

FatFile fat_open_files[FAT_MAX_FILES]; // All files of the system

// Buffers
uint16_t long_name16[FAT_MAX_LONG_NAME];
uint8_t long_name8[FAT_MAX_LONG_NAME];

FatPartitionInfo fat_pinfo; // Loaded FatPartitionInfo.
int32_t fat_initialized = 0; // Has the library been initiliazed?

// --- Internal functions ---
static inline uint32_t umin(uint32_t a, uint32_t b) {
  return a < b ? a : b;
}

int fat_read_single_block(unsigned int addr, uint8_t *buffer) {
  if (0 == disk_read(addr, buffer, 1)) {
    return FAT_SUCCESS;
  }

  errno = EIO;
  return FAT_FAIL;
}

int fat_write_single_block(unsigned int addr, uint8_t *buffer) {
  if (0 == disk_write(addr, buffer, 1)) {
    return FAT_SUCCESS;
  }

  errno = EIO;
  return FAT_FAIL;
}

// Reads a little-endian value from a buffer
inline uint32_t fat_get_uint32(uint8_t *buf) {
  return buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
}
inline uint16_t fat_get_uint16(uint8_t *buf) {
  return buf[0] + (buf[1] << 8);
}
inline uint8_t fat_get_uint8(uint8_t *buf) {
  return buf[0];
}

// Sets a little-endian value in buffer
inline void fat_set_uint32(uint32_t v, uint8_t *buf) {
  buf[0] = (uint8_t)v;
  buf[1] = (uint8_t)(v >> 8);
  buf[2] = (uint8_t)(v >> 16);
  buf[3] = (uint8_t)(v >> 24);
}
inline void fat_set_uint16(uint16_t v, uint8_t *buf) {
  buf[0] = (uint8_t)v;
  buf[1] = (uint8_t)(v >> 8);
}
inline void fat_set_uint8(uint16_t v, uint8_t *buf) {
  buf[0] = (uint8_t)v;
}

// Reads byte array from a read sector
inline void fat_get_string(uint8_t *buf, uint8_t *str, uint32_t sz) {
  for (; sz > 0; --sz) {
    *str++ = *buf++;
  }
}

// Retrieves the attribute of directory entry in a read sector.
inline uint8_t fat_attrib_of_dir_entry(uint8_t *sector, uint32_t entry_idx) {
  return sector[entry_idx * FAT_DIR_ENTRY_WIDTH + 11]; // Just a single byte
}

// Returns the first sector of the given cluster
inline uint32_t fat_first_sector_of_cluster(uint32_t cluster) {
  return fat_pinfo.cluster_begin_addr +
    (cluster - 2) * fat_pinfo.sectors_per_cluster;
}

/*! Returns the long name for directory entry.
  Both buffers must be 512 bytes large.
  First buffer is assumed to be loaded.
  Works forward, updating idx and buffer.
  Returns FAT_SUCCESS on succesful retrieval.
  Sets errno.
*/
int fat_get_long_name_uint16(uint8_t *buffer, FatDirEntryIdx *idx, uint16_t *buf) {
  int i = 0;
  int c = 0;

  uint8_t attrib = 0; // Attribute of entry
  uint8_t order = 0; // Order of entry

  uint32_t entries_per_sector = fat_pinfo.bytes_per_sector / 32;
  uint32_t sec_start = fat_first_sector_of_cluster(idx->cluster);
  uint32_t edx = idx->index * FAT_DIR_ENTRY_WIDTH; // Index of first byte of entry in sector

  // Check non-empty entry
  if (0xE5 == buffer[edx] || 0x00 == buffer[edx]) {
    return FAT_FAIL; // Empty entry
  }

  // Check that its the first long name entry
  attrib = fat_attrib_of_dir_entry(buffer, idx->index);
  if (attrib != FAT_LONG_NAME_ENTRY || !(buffer[edx] & FAT_LAST_LONG)) {
    return FAT_FAIL; // Not the first long entry
  }

  // Get the length of the chain
  order = buffer[edx] ^ FAT_LAST_LONG;
  uint32_t pmax = order * 13; // End of the long name
  uint32_t pmin = pmax; // Start of every chunk write

  int success = 0;
  do {
    // Check entries
    while (idx->index < entries_per_sector) {
      edx = idx->index * FAT_DIR_ENTRY_WIDTH;

      attrib = buffer[edx + 11];
      if (attrib != FAT_LONG_NAME_ENTRY) {
        return FAT_FAIL; // Broken chain
      }

      // Collect name
      c = 0;
      pmin -= 13;
      for (i = 0; i < 5; i++) {
        buf[pmin + c + i] = buffer[edx + 1 + i*2];
      }
      c += 5;
      for (i = 0; i < 6; i++) {
        buf[pmin + c + i] = buffer[edx + 14 + i*2];
      }
      c += 6;
      for (i = 0; i < 2; i++) {
        buf[pmin + c + i] = buffer[edx + 28 + i*2];
      }
      c += 2;

      idx->index++;

      // Check order
      order = buffer[edx] & FAT_LAST_LONG ?
        buffer[edx] ^ FAT_LAST_LONG : buffer[edx];
      if (order == 1) { // The last entry
        buf[umin(255, pmax)] = 0; // Cap path
        success = 1;
        break;
      }
    }

    // Stop, unless sector must be updated
    if (success && idx->index < entries_per_sector) {
      break;
    }

    // Next sector
    if (idx->sector >= fat_pinfo.sectors_per_cluster - 1) {
      return FAT_FAIL; // Reached start of cluster without finding long name
    }
    idx->sector++;
    idx->index = 0;

    if (FAT_SUCCESS != fat_read_single_block(sec_start + idx->sector, buffer)) {
      success = 0;
      break;
    }
  } while (c <= 255 && !success);

  return success ? FAT_SUCCESS : FAT_FAIL;
}

/*! Gets the 8-bit version of the long name.
  Buffer must be 512 bytes long and is assumed to be loaded.
  Works forward in entries updating idx and buffer.
 */
uint32_t fat_get_long_name_uint8(uint8_t *buffer, FatDirEntryIdx *idx, uint8_t *long_name) {
  int i;
  uint32_t edx = idx->index * FAT_DIR_ENTRY_WIDTH; // Index of first byte of entry in sector

  // Check entry entry
  if (0xE5 == buffer[edx] || 0x00 == buffer[edx]) {
    return FAT_FAIL; // Empty entry
  }

  // Check that its the first long name entry
  uint8_t attrib = fat_attrib_of_dir_entry(buffer, idx->index);
  int has_long_name = attrib == FAT_LONG_NAME_ENTRY && (buffer[edx] & FAT_LAST_LONG);

  if (has_long_name) {
    if (FAT_SUCCESS != fat_get_long_name_uint16(buffer, idx, long_name16)) {
      return FAT_FAIL;
    }
    else {
      for (i = 0; i < 256; i++) {
        long_name[i] = (uint8_t)(long_name16[i]);
      }
    }
  }
  else {
    uint8_t short_name[12]; // 11 bytes + null terminating
    fat_get_string(buffer + idx->index * FAT_DIR_ENTRY_WIDTH, short_name, 11);
    short_name[11] = 0; // Terminate

    int k = 0;

    // Name first
    for (i = 0; i < 8; i++) {
      if (short_name[i] != 0x20) {
        long_name[k++] = short_name[i];
      }
      else {
        break;
      }
    }

    // Extension
    if (short_name[8] != 0x20) {
      long_name[k++] = '.';
      for (i = 0; i < 3; i++) {
        if (short_name[8 + i] != 0x20) {
          long_name[k++] = short_name[8 + i];
        }
        else {
          break;
        }
      }
    }

    // Cap
    long_name[k] = 0;
  }

  return FAT_SUCCESS;
}

/*! Loads the first available FAT partition, if available.
    Returns 0 on success, -1 on failure.
 */
int fat_load_first_partition_info(FatPartitionInfo *pinfo) {
  uint8_t i;
  for (i = 0; i < 4; i++) {
    if (FAT_SUCCESS == fat_load_partition_info(i, pinfo)) {
      return FAT_SUCCESS;
    }
  }

  return FAT_FAIL;
}

/*! Loads the FAT partition in an entry, from the MBR.
  Return 0 on success, -1 on failure.
  Assumes block size is 512 bytes.

  errno == EIO: Bad response from disk
  errno == EBADF: Non-supported partition type
  errno == EINVAL: Entry index out of range [0 - 3];
*/
int fat_load_partition_info(uint8_t idx, FatPartitionInfo *pinfo) {
  uint8_t res;
  uint8_t buffer[512]; // Buffer holding block

  // TODO: Ensure DiskInfo block size matches FAT block size.

  // Read data
  res = fat_read_single_block(0, buffer);
  if (0 != res) {
    errno = EIO;
    return FAT_FAIL;
  }

  if (idx >= 4) {
    errno = EINVAL;
    return FAT_FAIL;
  }

  // Sanity check that this is a MBR
  // Expect byte 510 = 0x55 and 511 = 0xAA (signature / magic number)
  if (buffer[510] != 0x55 && buffer[511] != 0xAA) {
    errno = EFAULT;
    return FAT_FAIL;
  }

  // Load partition info
  uint8_t *pentry_begin = buffer + PTABLE_BEGIN + idx * PTABLE_ENTRY_SIZE;
  uint8_t ptype = fat_get_uint8(pentry_begin + PTABLE_TYPE_OFFSET);
  if (ptype != FAT_PARTITION_SIG_CHS && ptype != FAT_PARTITION_SIG_LBA) {
    errno = EBADF;
    return FAT_FAIL;
  }

  pinfo->vol_begin_addr = fat_get_uint32(pentry_begin + PTABLE_VOL_ADDR_OFFSET);

  // Read Volume ID
  res = fat_read_single_block(pinfo->vol_begin_addr, buffer);
  if (0 != res) {
    errno = EIO;
    return FAT_FAIL;
  }

  // General FAT
  pinfo->bytes_per_sector    = fat_get_uint16(buffer + 11);
  pinfo->sectors_per_cluster = fat_get_uint8(buffer + 13);
  pinfo->num_res_sectors     = fat_get_uint16(buffer + 14);
  pinfo->num_fats            = fat_get_uint8(buffer + 16);
  pinfo->num_dir_entries     = fat_get_uint16(buffer + 17);
  pinfo->num_total_sectors   = fat_get_uint16(buffer + 19);
  pinfo->media_desc_type     = fat_get_uint8(buffer + 21);
  pinfo->sectors_per_fat     = fat_get_uint16(buffer + 22);

  // FAT32 only
  pinfo->sectors_per_fat    = fat_get_uint32(buffer + 0x24);
  pinfo->root_dir_cluster   = fat_get_uint32(buffer + 0x2C);

  // Derived
  pinfo->entries_per_sector = pinfo->bytes_per_sector / FAT_DIR_ENTRY_WIDTH;
  pinfo->fat_begin_addr     = pinfo->vol_begin_addr + pinfo->num_res_sectors;
  pinfo->cluster_begin_addr = pinfo->fat_begin_addr +
    (pinfo->num_fats * pinfo->sectors_per_fat);

  // Calculate address of root dir
  pinfo->root_dir_addr      = pinfo->cluster_begin_addr +
    (pinfo->root_dir_cluster - 2) * pinfo->sectors_per_cluster;

  errno = 0;
  return FAT_SUCCESS;
}

// Finds the index in path of the next path delimiter or EOL
uint32_t fat_idx_of_next_path_delim(uint8_t *path, uint32_t start) {
  uint32_t next = start;
  for (next = start; 1; ++next) {
    if (path[next] == 0 || path[next] == '\\' || path[next] == '/' ||
        path[next] == 0x20) { // Present in FAT32 names
      break;
    }
  }
  return next;
}

// Retrieves the cluster of directory entry in a read sector.
inline uint32_t fat_cluster_of_dir_entry(uint8_t *sector, uint32_t entry_idx) {
  uint32_t high = fat_get_uint16(sector + entry_idx * FAT_DIR_ENTRY_WIDTH + 0x14);
  uint32_t low = fat_get_uint16(sector + entry_idx * FAT_DIR_ENTRY_WIDTH + 0x1a);
  return low | (high << 16);
}

// Retrieves the size of directory entry target in a read sector.
inline uint32_t fat_size_of_dir_entry(uint8_t *sector, uint32_t entry_idx) {
  return fat_get_uint32(sector + entry_idx * FAT_DIR_ENTRY_WIDTH + 0x1C);
}

// Reads entry from the FAT corresponding to cluster.
// Negative numbers are errors
int fat_get_table_value(uint32_t cluster, uint32_t *tv) {
  uint8_t buffer[fat_pinfo.bytes_per_sector]; // Should be 512

  uint32_t idx = cluster * 4; // 4 bytes for every entry
  uint32_t sec = fat_pinfo.fat_begin_addr +
    (idx / fat_pinfo.bytes_per_sector); // Sector containing entry
  uint32_t entry_idx = idx % fat_pinfo.bytes_per_sector; // Index in sector

  // Read sector of FAT
  if (0 != fat_read_single_block(sec, buffer)) {
    return FAT_FAIL;
  }

  *tv = fat_get_uint32(buffer + entry_idx) & 0x0FFFFFFF; // Ignore high 4 bits
  return FAT_SUCCESS;
}

// Compares part of a path name to the name belong to an entry (chain)
// Updates cdidx to short entry
int fat_compare_entry_name(const char *path, int pmin, int pmax,
                           uint8_t *sector, FatDirEntryIdx *cdidx)
{
  int k = 0;

  if (FAT_SUCCESS == fat_get_long_name_uint8(sector, cdidx, long_name8)) {
    int32_t entry_len = fat_idx_of_next_path_delim(long_name8, 0);
    int32_t part_len = pmax - pmin;

    // Check every character individually
    if (entry_len != part_len) {
      return FAT_FAIL;
    }
    for (k = 0; k < entry_len; k++) {
      if (toupper(long_name8[k]) != toupper(path[pmin + k])) {
        return FAT_FAIL;
      }
    }
    if (k != entry_len) { // Mismatch == early termination
      return FAT_FAIL;
    }

    return FAT_SUCCESS;
  }

  return FAT_FAIL;
}


/*! Opens a file for a dir entry in a sector
  Returns fd in case of success
  Returns -1 in case of failure
  fd is index into fat_open_files, but is offset by FAT_FD_OFFSET
  to avoid clashing with fds for stdout / stdin / stderr
*/
int fat_open_dir_entry(uint8_t *sector, FatDirEntryIdx *entry_idx) {
  int fd = -1;

  int i;
  for (i = 0; i < FAT_MAX_FILES; ++i) {
    if (0 == fat_open_files[i].free) {
      continue;
    }

    FatFile *f = &fat_open_files[i];
    f->free = 0;
    f->pos = 0;
    f->dir_idx = *entry_idx;
    f->start_cluster = fat_cluster_of_dir_entry(sector, entry_idx->index);
    f->current_cluster = f->start_cluster;
    f->size = fat_size_of_dir_entry(sector, entry_idx->index);
    fd = i;
    break;
  }

  if (fd == -1) {
    return -1;
  }

  return fd + FAT_FD_OFFSET;
}

// TODO: Handle possible loops in this return values.
// A loop in FAT entry link could cause an endless loop.
int fat_acquire_next_cluster(uint32_t *cluster, int link_to_new) {
  uint32_t tv = 0;
  if (FAT_SUCCESS != fat_get_table_value(*cluster, &tv)) {
    return FAT_FAIL;
  }
  else if (FAT_TV_BAD(tv)) {
    errno = EBADF;
    return FAT_FAIL;
  }
  else if (FAT_TV_FREE(tv)) {
    //printf("Error: Cluster %ld is marked as free!\n", *cluster);
    errno = EBADF;
    return FAT_FAIL;
  }
  else if (!FAT_TV_LAST(tv)) {
    *cluster = tv;
    return FAT_SUCCESS;
  }

  // Search for next cluster
  uint32_t fat_start = fat_pinfo.fat_begin_addr;
  uint32_t secsz = fat_pinfo.bytes_per_sector;
  uint32_t entries_per_sector = secsz / 4;
  uint8_t buf[secsz]; // Should be 512

  uint32_t ent_idx_in_fat_start = *cluster;
  uint32_t ent_idx_in_sec_start = ent_idx_in_fat_start % entries_per_sector;
  uint32_t cur_ent_in_sec = ent_idx_in_sec_start;

  uint32_t sec_off_start = ent_idx_in_fat_start / entries_per_sector;
  uint32_t cur_sec_in_fat = fat_start + sec_off_start;
  uint32_t cur_ent_in_fat = 0;

  // Copy sector containing current entry, for modification to the FAT
  uint8_t orig_sec[secsz];
  if (FAT_SUCCESS != fat_read_single_block(cur_sec_in_fat, orig_sec)) {
    //printf("Fail Read Acquire Original\n"); // TODO: Remove
    return FAT_FAIL;
  }

  uint32_t i = 0; // Sector offset
  uint32_t j = 0; // Entry offset
  for (i = 0; i < fat_pinfo.sectors_per_fat; i++) {
    cur_sec_in_fat = (sec_off_start + i) % fat_pinfo.sectors_per_fat;

    // Read sector we are currently searching
    if (FAT_SUCCESS != fat_read_single_block(fat_start + cur_sec_in_fat, buf)) {
      //printf("Fail Read Acquire 1\n"); // TODO: Remove
      return FAT_FAIL;
    }

    // Loop through entries
    for (j = 0; j < entries_per_sector; j++) { // j is the entry index in cur_sec_in_fat
      cur_ent_in_sec = (ent_idx_in_sec_start + j) % entries_per_sector;
      cur_ent_in_fat = cur_sec_in_fat * entries_per_sector + cur_ent_in_sec;
      tv = fat_get_uint32(buf + 4 * cur_ent_in_sec) & 0x0FFFFFFF; // Ignore high 4 bits

      if (FAT_TV_BAD(tv)) {
        continue;
      }
      if (FAT_TV_LAST(tv)) {
        continue;
      }
      if (!FAT_TV_FREE(tv)) {
        continue;
      }

      // The table entry is free
      uint32_t next_cluster = cur_ent_in_fat;
      if (next_cluster == 0) // TODO: Avoid this by altering the calculation
        continue;
      *cluster = next_cluster;

      // Change values in FAT
      if (link_to_new) {
        // If the entry is in the same sector, we must point to the new cluster in
        // the read buffer, or else the change will be overwritten by the second write.
        uint8_t *mark_next_buf = sec_off_start == cur_sec_in_fat ? buf : orig_sec;

        // Link old entry to new
        fat_set_uint32(next_cluster,  mark_next_buf + 4 * ent_idx_in_sec_start);
        if (FAT_SUCCESS !=
            fat_write_single_block(fat_start + sec_off_start, mark_next_buf)) {
          return FAT_FAIL;
        }
      }

      // Mark new last cluster as last
      fat_set_uint32(0x0FFFFFFF, buf + 4 * cur_ent_in_sec);
      if (FAT_SUCCESS != fat_write_single_block(fat_start + cur_sec_in_fat, buf)) {
        return FAT_FAIL;
      }

      return FAT_SUCCESS;
    }
  }

  errno = ENOSPC;
  return FAT_FAIL;
}

/*! Resolves path to a directory entry. Loads containing sector into sector.
  Returns FAT_SUCCESS or FAT_FAILURE.
  Sets errno.
*/
int fat_resolve_path(const char *path, uint8_t *sector, FatDirEntryIdx *dir_idx) {
  uint32_t current_cluster = fat_pinfo.root_dir_cluster;
  uint32_t current_start_sector = fat_pinfo.root_dir_addr;

  uint32_t entries_per_sector = fat_pinfo.bytes_per_sector / FAT_DIR_ENTRY_WIDTH;

  int pmin = 0; // Start of current path part
  int pmax = 0; // End of current path part

  int exhausted_dir = 0;
  int found_dir_in_cluster = 0;

  FatDirEntryIdx cdidx = *dir_idx;

  while (!exhausted_dir) {
    // Search forward for pmax
    pmax = fat_idx_of_next_path_delim((uint8_t *)path, pmin);

    while (!exhausted_dir) {
      cdidx.cluster = current_cluster;
      current_start_sector = fat_first_sector_of_cluster(current_cluster);

      // Check cluster for entry
      for (cdidx.sector = 0; cdidx.sector < fat_pinfo.sectors_per_cluster; cdidx.sector++) {
        fat_read_single_block(current_start_sector + cdidx.sector, sector); // TODO: Handle error

        // Check sector for entry
        for (cdidx.index = 0; cdidx.index < entries_per_sector; cdidx.index++) {

          if (FAT_SUCCESS ==
              fat_compare_entry_name(path, pmin, pmax, sector, &cdidx)) {
            found_dir_in_cluster = 1;
            break;
          }
        }

        if (1 == found_dir_in_cluster) {
          break;
        }
      }

      if (!found_dir_in_cluster) {
        uint32_t tv = 0;
        if (FAT_SUCCESS != fat_get_table_value(current_cluster, &tv)) {
          exhausted_dir = 1; // TODO: Try again? Set errno?
        }
        else if (FAT_TV_LAST(tv)) {
          exhausted_dir = 1;
        }
        else if (FAT_TV_BAD(tv)) {
          exhausted_dir = 1; // TODO: Check next cluster maybe?
        }
        else {
          current_cluster = tv;
        }
      }
      else { // found_dir_in_cluster
        uint8_t isdir = FAT_DIRECTORY &
          fat_attrib_of_dir_entry(sector, cdidx.index);

        if (0 != path[pmax]) { // Not at last path segment
          if (!isdir) {
            errno = ENOTDIR;
            return FAT_FAIL;
          }
        }
        else { // At last path segment
          *dir_idx = cdidx;
          errno = 0;
          return FAT_SUCCESS;
        }

        // Don't update current_sector
        pmin = pmax + 1;
        pmax = pmin;
        found_dir_in_cluster = 0; // Search again
        current_cluster = fat_cluster_of_dir_entry(sector, cdidx.index);
        break;
      }
    }
  }

  errno = ENOENT;
  return FAT_FAIL;
}

/* Frees all cluster in the cluster chain.
  Returns FAT_SUCCESS or FAT_FAILURE.
  Sets errno.
*/
int fat_free_cluster_chain(const uint32_t start_cluster) {
  uint32_t secsz = fat_pinfo.bytes_per_sector;
  uint8_t buf[secsz];

  uint32_t tv = start_cluster;
  uint32_t byteoff = 0;
  uint32_t secoff = 0;
  uint32_t sec = 0;
  uint32_t byteoff_in_sec = 0;

  // TODO: Only read / write buf when reaching a new one
  errno = 0;
  do {
    // Update index
    byteoff = tv * 4;
    secoff = byteoff / fat_pinfo.bytes_per_sector;
    sec = fat_pinfo.fat_begin_addr + secoff;
    byteoff_in_sec = byteoff % secsz;

    // Load in buf
    if (FAT_SUCCESS != fat_read_single_block(sec, buf)) {
      break;
    }

    // Retrieve and verify table value
    tv = fat_get_uint32(buf + byteoff_in_sec) & 0x0FFFFFFF;
    if (FAT_TV_BAD(tv)) {
      errno = EBADF;
      break;
    }
    else if (FAT_TV_FREE(tv)) {
      errno = EBADF;
      break;
    }

    // Free cluster
    fat_set_uint32(0, buf + byteoff_in_sec);

    // Write back
    if (FAT_SUCCESS != fat_write_single_block(sec, buf)) {
      break;
    }
  }
  while (!FAT_TV_LAST(tv));

  return errno == 0 ? FAT_SUCCESS : FAT_FAIL;
}

/*! Creates a new file or directory for the path, setting the file descriptor
  Arguments: Path, index of parent dir, buffer, file descriptor, file or dir?
  Directory index and buffer are assumed to set and loaded.
  Sets errno.
*/
int fat_create(const char *path, FatDirEntryIdx *pdir_idx, uint8_t *sector, int *fd, const char isfile) {

  errno = 0;

  // Isolate the file from the rest of the path
  uint8_t delim = 0;
  uint32_t pmin = 0;
  uint32_t pmax = 0;
  do {
    pmax = fat_idx_of_next_path_delim((uint8_t *)path, pmin);
    delim = path[pmax];
    if (delim == 0) { // Search until end of path
      break; // pmin is now index of first character in file name
    }
    pmin = pmax + 1;
  } while (delim != 0); // Just a fallback if above is altered

  // Check that the parent directory exists
  char dir_path[pmin]; // Can contain first part + delim + 0
  memcpy(dir_path, path, pmin);
  dir_path[pmin] = 0; // Cap path
  if (dir_path[pmin - 1]) {
    dir_path[pmin - 1] = 0; // TODO: resolve_path should make this unnecesary
  }

  // Find parent dir, if it is not root
  int path_resolved = 0;
  if (pmin == 0) {
    path_resolved = 1;
    pdir_idx->cluster = fat_pinfo.root_dir_cluster;
  }
  else if (FAT_SUCCESS == fat_resolve_path(dir_path, sector, pdir_idx)) {
    path_resolved = 1;
    pdir_idx->cluster = fat_cluster_of_dir_entry(sector, pdir_idx->index);
  }

  if (path_resolved) {
    pdir_idx->sector = pdir_idx->index = 0; // Only cluster is needed

    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    // Find file extension
    uint32_t name_len_total = pmax - pmin; // Length of file name + extension
    uint32_t name_len = name_len_total;
    uint32_t extension_len = 0;
    int32_t last_dot_idx = -1;
    for (i = 0; i < name_len_total; ++i) {
      if (path[pmax - 1 - i] == '.') {
        last_dot_idx = pmax - 1 - i;
      }
    }
    if (last_dot_idx > 0) {
      name_len = last_dot_idx - pmin;
      extension_len = name_len_total - name_len - 1; // Adjust for dot in total name
    }

    // Calculate directory entries needed to store name
    uint32_t req_entries = 1;
    if (name_len > 8 || extension_len > 3) { // Long name needed
      req_entries += name_len_total / 13;
      if (name_len_total % 13 != 0) {
        req_entries++;
      }
    }

    // Copy file name over
    char fname_short[8 + 3]; // Name and filetype
    for (i = 0; i < 11; i++) {
      fname_short[i] = 0x20; // Filler byte in FAT
    }

    uint32_t copied = 0;
    for (i = 0; i < name_len; ++i) {
      char c = path[pmin + i];
      if (copied < 8 && c != ' ' && c != '.') { // Strip all spaces and periods
        fname_short[copied] = toupper(c);
        copied++;
      }
    }

    // Copy file ending if it exists
    if (last_dot_idx > 0) {
      for (i = 0; i < 3; i++) {
        fname_short[8 + i] = toupper(path[last_dot_idx + 1 + i]);
      }
    }

    // Search until end of directory doing:
    //   1. Find place for entry that has space for long name entries before it
    //   2. Find lowest possible short name number
    uint32_t empty_entries_seen = 0; // Number of free entries seen in a row
    FatDirEntryIdx target_dir_idx; // Final index of the main directory entry

    uint32_t current_cluster = pdir_idx->cluster;
    uint32_t current_start_sector = fat_first_sector_of_cluster(current_cluster);
    uint32_t entries_per_sector = fat_pinfo.bytes_per_sector / FAT_DIR_ENTRY_WIDTH;
    uint8_t *ent = 0;

    // Store the index of the first entry in the chain of long entries
    // This makes writing them easier, since the FAT is singly-linked
    FatDirEntryIdx first_dir_idx;
    first_dir_idx.cluster = current_cluster;
    first_dir_idx.sector = 0;
    first_dir_idx.index = 0;

    // Number of collisions on the short name
    uint32_t short_name_nums[12];
    for (i = 0; i < 12; ++i) {
      short_name_nums[i] = 0;
    }

    int done_searching = 0;
    int reached_last = 0;
    int found_index = 0;
    while (!done_searching) {
      current_start_sector = fat_first_sector_of_cluster(current_cluster);

      // Check cluster for entry
      for (i = 0; i < fat_pinfo.sectors_per_cluster; ++i) {
        fat_read_single_block(current_start_sector + i, sector); // TODO: Handle error

        // Check sector for entry
        for (j = 0; j < entries_per_sector; ++j) {
          ent = sector + FAT_DIR_ENTRY_WIDTH * j;

          reached_last |= FAT_DIR_ENTRY_LAST == ent[0];
          if (reached_last || ent[0] == FAT_DIR_ENTRY_FREE) {
            // If the last entry in the folder has been reached
            // we could begin jumping whole sectors ahead, as we
            // know they must be free. This is not done however.

            // Empty entry, check for space
            empty_entries_seen++;
            if (!found_index) {
              if (empty_entries_seen == 1) {
                first_dir_idx.cluster = current_cluster;
                first_dir_idx.sector = i;
                first_dir_idx.index = j;
              }
              if (empty_entries_seen >= req_entries) {
                found_index = 1;
                target_dir_idx.cluster = current_cluster;
                target_dir_idx.sector = i;
                target_dir_idx.index = j;
              }
            }
          }
          else { // An occupied entry
            empty_entries_seen = 0;

            // Check short name collision
            if (ent[11] != FAT_LONG_NAME_ENTRY) {
              short_name_nums[0]++;
              for (k = 0; k < 11; ++k) {
                if (toupper(fname_short[k]) == toupper(ent[k])) {
                  short_name_nums[k+1]++;
                }
                else {
                  break;
                }
              }

              if (k == 11) { // Collision
                errno = EEXIST;
                done_searching = 1;
                break;
              }
            }
          }
        }
      }

      if (found_index) {
        break;
      }

      // At end of cluster, so find next, extending folder if necessary
      if (FAT_SUCCESS != fat_acquire_next_cluster(&current_cluster, 1)) {
        // Keep errno from acquire_next
        done_searching = 1;
        break;
      }

      // Long entry chains are always contiguous in memory, so when jumping
      // clusters restart our entry counting.
      empty_entries_seen = 0;

      // We need to make remove any entries marked as last, to have the next
      // cluster be reachable. This only needs to happen when reaching a new cluster,
      // as otherwise they would be overwritten by the chain itself.
      if (reached_last) {
        uint32_t tail_entries = entries_per_sector - first_dir_idx.index;
        for (j = 0; j < tail_entries; j++) {
          ent = sector + FAT_DIR_ENTRY_WIDTH * (first_dir_idx.index + j);
          ent[0] = FAT_DIR_ENTRY_FREE;
        }
        assert(first_dir_idx.sector == fat_pinfo.sectors_per_cluster - 1);
        if (FAT_SUCCESS != fat_write_single_block(current_start_sector +
                                                  first_dir_idx.sector, sector)) {
          // Keep errno from write
          break;
        }
      }
    }

    if (errno != 0) {
      // Just keep errno, skipping further error checking
    }
    else if (!found_index) {
      errno = ENOSPC;
    }
    else if (req_entries == 1 && pmax >= FAT_MAX_SHORT_PATH) {
      errno = ENAMETOOLONG;
    }
    else if (req_entries >= 2 && (pmax >= FAT_MAX_LONG_PATH ||
                                  name_len_total >= FAT_MAX_LONG_NAME)) {
      errno = ENAMETOOLONG;
    }
    else {
      // Add tail to short name if needed
      uint32_t short_chars_max = 10;
      int32_t short_name_chars = req_entries > 1 ? 6 : 8;
      short_name_chars = umin(name_len, short_name_chars);
      while (short_name_chars >= 0 &&
            short_name_nums[short_name_chars] + 1 >= short_chars_max) {
        short_name_chars--;
        short_chars_max *= 10;
      }

      // Add tail to long name entries
      if (req_entries > 1) {
        fname_short[short_name_chars] = '~';
        sprintf(fname_short + short_name_chars + 1, "%ld",
                1 + short_name_nums[short_name_chars]);

        // Copy extension back
        // TODO: Why is it removed?
        if (last_dot_idx > 0) {
          for (i = 0; i < 3; i++) {
            fname_short[8 + i] = toupper(path[last_dot_idx + 1 + i]);
          }
        }
      }

      uint32_t en = req_entries; // Current entry number. Short = 1. Next = 0.
      uint32_t ord = req_entries - 1;

      // Begin writing from the start of the chain
      uint32_t cur_cluster = first_dir_idx.cluster;
      uint32_t cur_sec_off = first_dir_idx.sector;
      uint32_t cur_sec_start = fat_first_sector_of_cluster(cur_cluster);
      uint32_t cur_ent_off = first_dir_idx.index;

      // Calculate checksum from short name
      uint8_t checksum = 0;
      for (i = 0; i < 11; i++) {
        checksum = (checksum >> 1) + (checksum << 7); // Rotate right
        checksum += fname_short[i];
      }

      // Get new cluster, marking in FAT
      uint32_t new_cluster = target_dir_idx.cluster;
      if (FAT_SUCCESS != fat_acquire_next_cluster(&new_cluster, 0)) {
        // TODO: Error handling
      }

      // Offset in path when writing long entries
      uint32_t path_off = pmax + (13 - (name_len_total % 13)) - 13;
      if (name_len_total % 13 == 0) {
        path_off -= 13;
      }

      // Read the sector containing the start of the chain
      /*
      if (target_dir_idx.sector != first_dir_idx.sector) {
        fat_read_single_block(cur_sec_start + cur_sec_off, sector); // TODO: Error handling
      }
      */
      fat_read_single_block(cur_sec_start + cur_sec_off, sector); // TODO: Error handling

      // Start writing
      while (en <= req_entries && errno == 0) {
        ent = sector + FAT_DIR_ENTRY_WIDTH * cur_ent_off;

        // TODO: Organize cases in order of: Long -> Short -> Last

        if (en == 1) { // Short entry
          memcpy(ent, fname_short, 11); // Name

          // Attribute (ARCHIVE set on creation)
          uint8_t attrib = FAT_ARCHIVE | (isfile ? 0 : FAT_DIRECTORY);
          fat_set_uint8(attrib, ent + 11);

          // We set all time values to zero, as there is no time setup
          fat_set_uint8(0, ent + 13); // Create time tenth
          fat_set_uint16(0, ent + 14); // Create time
          fat_set_uint16(0, ent + 16); // Create date
          fat_set_uint16(0, ent + 18); // Last access date
          fat_set_uint16(0, ent + 22); // Write time
          fat_set_uint16(0, ent + 24); // Write date

          fat_set_uint16(new_cluster >> 16, ent + 20); // High word of cluster
          fat_set_uint16(new_cluster, ent + 26); // Low word of cluster
          fat_set_uint32(0, ent + 28); // File size

          // Get file descriptor
          if (isfile) {
            *fd = fat_open_dir_entry(sector, &target_dir_idx); // Get file descriptor
          }
          else {
            pdir_idx->cluster = new_cluster; // Needed in mkdir
          }
        }
        else if (en >= 2) { // Long entries
          ord = en - 1;
          if (ord == req_entries - 1) {
            ord |= FAT_LAST_LONG; // Mark physical first as last in long name chain
          }
          fat_set_uint8(ord, ent + 0); // Ordinal
          fat_set_uint8(FAT_LONG_NAME_ENTRY, ent + 11); // Attribute
          fat_set_uint8(0, ent + 12); // Type (long sub-entry)
          fat_set_uint8(checksum, ent + 13); // Checksum
          fat_set_uint16(0, ent + 26); // First Cluster Low (always zero)

          // Set name
          uint16_t v = 0;
          for (i = 0; i < 5; ++i) { // Char 0-4
            v = path_off > pmax ? 0xFFFF : path_off == pmax ? 0 : path[path_off];
            fat_set_uint16(v, ent + 1 + 2*i);
            path_off++;
          }
          for (i = 0; i < 6; ++i) { // Char 5-10
            v = path_off > pmax ? 0xFFFF : path_off == pmax ? 0 : path[path_off];
            fat_set_uint16(v, ent + 14 + 2*i);
            path_off++;
          }
          for (i = 0; i < 2; ++i) { // Char 11-12
            v = path_off > pmax ? 0xFFFF : path_off == pmax ? 0 : path[path_off];
            fat_set_uint16(v, ent + 28 + 2*i);
            path_off++;
          }
          path_off -= 2*13;
        }
        else if (en == 0) { // Reached entry following the short entry
          // Mark the following entry as last in folder, if it is
          if (reached_last) {
            ent[0] = FAT_DIR_ENTRY_LAST;
          }

          fat_write_single_block(cur_sec_start + cur_sec_off, sector); // Done writing
          break; // No need to update offsets anymore
        }

        // Check if new sector is reached
        cur_ent_off++;
        if (cur_ent_off >= entries_per_sector) {
          // Write sector
          fat_write_single_block(cur_sec_start + cur_sec_off, sector);

          cur_ent_off = 0;
          cur_sec_off++;
          if (cur_sec_off >= fat_pinfo.sectors_per_cluster) {
            if (FAT_SUCCESS == fat_acquire_next_cluster(&cur_cluster, 0)) {
              cur_sec_start = fat_first_sector_of_cluster(cur_cluster);
              cur_sec_off = 0;
            }
            else {
              // TODO: Error handling
            }
          }

          // Read new sector
          fat_read_single_block(cur_sec_start + cur_sec_off, sector);
        }

        // Next entry
        en--;
      }
    }
  }

  // Keep errno from earlier failure

  return errno == 0 ? FAT_SUCCESS : FAT_FAIL;
}

// Deletes and unlinks a directory entry
int fat_delete(const char *path, uint8_t *sector) {
  errno = 0;

  // Isolate the file from the rest of the path
  uint8_t delim = 0;
  uint32_t pmin = 0;
  uint32_t pmax = 0;
  do {
    pmax = fat_idx_of_next_path_delim((uint8_t *)path, pmin);
    delim = path[pmax];
    if (delim == 0) { // Search until end of path
      break; // pmin is now index of first character in file name
    }
    pmin = pmax + 1;
  } while (delim != 0); // Just a fallback if above is altered

  // Check that the parent directory exists
  char dir_path[pmin]; // Can contain first part + delim + 0
  memcpy(dir_path, path, pmin);
  dir_path[pmin] = 0; // Cap path
  if (dir_path[pmin - 1]) {
    dir_path[pmin - 1] = 0; // TODO: resolve_path should make this unnecesary
  }

  // Find parent dir, if it is not root
  FatDirEntryIdx pdir_idx;
  int path_resolved = 0;
  if (pmin == 0) {
    path_resolved = 1;
    pdir_idx.cluster = fat_pinfo.root_dir_cluster;
  }
  else if (FAT_SUCCESS == fat_resolve_path(dir_path, sector, &pdir_idx)) {
    path_resolved = 1;
    pdir_idx.cluster = fat_cluster_of_dir_entry(sector, pdir_idx.index);
  }

  if (path_resolved) {

    // Search until file has been found either:
    //   1. End of directory
    //   2. Next non-empty entry
    // Everything in between will be cleared
    uint32_t empty_entries_seen = 0; // Number of free entries seen in a row
    FatDirEntryIdx target_dir_idx; // Last index

    FatDirEntryIdx cidx;
    cidx.cluster = pdir_idx.cluster;
    cidx.sector = 0;
    cidx.index = 0;

    uint32_t current_start_sector = fat_first_sector_of_cluster(cidx.cluster);
    uint32_t entries_per_sector = fat_pinfo.bytes_per_sector / FAT_DIR_ENTRY_WIDTH;
    uint8_t *ent = 0;

    // Index of first empty entry in streak before target entry
    FatDirEntryIdx first_empty_idx;
    first_empty_idx.cluster = cidx.cluster;
    first_empty_idx.sector = 0;
    first_empty_idx.index = 0;

    // Index of first empty entry in streak before target entry
    FatDirEntryIdx first_target_idx;

    uint32_t initial_cluster = 0;

    int found_entry = 0;
    int found_next = 0;
    int done_searching = 0;
    int reached_last = 0;
    while (!done_searching) {
      current_start_sector = fat_first_sector_of_cluster(cidx.cluster);

      // Check cluster for entry
      for (cidx.sector = 0; cidx.sector < fat_pinfo.sectors_per_cluster; cidx.sector++) {
        fat_read_single_block(current_start_sector + cidx.sector, sector);

        // Check sector for entry
        for (cidx.index = 0; cidx.index < entries_per_sector; ++cidx.index) {
          ent = sector + FAT_DIR_ENTRY_WIDTH * cidx.index;

          reached_last |= FAT_DIR_ENTRY_LAST == ent[0];
          if (ent[0] == FAT_DIR_ENTRY_FREE) {
            // Empty entry, check for space
            empty_entries_seen++;
            if (empty_entries_seen == 1) {
              first_empty_idx = cidx;
            }
          }
          else {
            if (!found_entry) {
              // Store this now since the name retrieval updates the idx
              first_target_idx = cidx;

              // Check name
              if (FAT_SUCCESS == fat_compare_entry_name(path, pmin, pmax, sector, &cidx)) {
                target_dir_idx = cidx;
                found_entry = 1;

                // Extract initial cluster
                initial_cluster = fat_cluster_of_dir_entry(sector, cidx.index);
              }
              else {
                empty_entries_seen = 0;
              }
            }
            else {
              found_next = 1;
              break;
            }
          }

          if (found_next || reached_last) {
            break;
          }
        }
        if (found_next || reached_last) {
          break;
        }
      }

      if (reached_last || (found_entry && found_next)) {
        break;
      }

      // At end of cluster, so find next
      if (FAT_SUCCESS != fat_acquire_next_cluster(&cidx.cluster, 0)) {
        // Keep errno from acquire_next
        done_searching = 1;
        break;
      }
    }

    if (errno != 0) {
      // Just keep errno, skipping further error checking
    }
    else if (!found_entry) {
      errno = ENOENT;
    }
    else if (initial_cluster == 0 || // A completely empty file can have no cluster attached
             FAT_SUCCESS == fat_free_cluster_chain(initial_cluster)) {
      if (reached_last && empty_entries_seen > 0) {
        // This entry is the last non-empty in the directory
        uint32_t sec_addr = fat_first_sector_of_cluster(first_empty_idx.cluster) +
          first_empty_idx.sector;

        fat_read_single_block(sec_addr, sector);
        fat_set_uint8(FAT_DIR_ENTRY_LAST, sector + first_empty_idx.index * FAT_DIR_ENTRY_WIDTH);
        fat_write_single_block(sec_addr, sector);
      }
      else {
        // Begin writing from the start of the chain
        uint32_t cur_cluster = first_target_idx.cluster;
        uint32_t cur_sec_off = first_target_idx.sector;
        uint32_t cur_sec_start = fat_first_sector_of_cluster(cur_cluster);
        uint32_t cur_ent_off = first_target_idx.index;

        uint8_t mark = reached_last ? FAT_DIR_ENTRY_LAST : FAT_DIR_ENTRY_FREE;

        fat_read_single_block(cur_sec_start + cur_sec_off, sector);

        // Start writing
        while (errno == 0) {
          ent = sector + FAT_DIR_ENTRY_WIDTH * cur_ent_off;

          int done = (cur_cluster == target_dir_idx.cluster &&
                      cur_sec_off == target_dir_idx.sector &&
                      cur_ent_off == target_dir_idx.index);

          // Mark entry
          fat_set_uint8(mark, ent);
          if (done) {
            fat_write_single_block(cur_sec_start + cur_sec_off, sector);
            break;
          }

          // Check if new sector is reached
          cur_ent_off++;
          if (cur_ent_off >= entries_per_sector) {
            // Write sector
            fat_write_single_block(cur_sec_start + cur_sec_off, sector);

            cur_ent_off = 0;
            cur_sec_off++;
            if (cur_sec_off >= fat_pinfo.sectors_per_cluster) {
              if (FAT_SUCCESS == fat_acquire_next_cluster(&cur_cluster, 0)) {
                cur_sec_start = fat_first_sector_of_cluster(cur_cluster);
                cur_sec_off = 0;
              }
              else {
                // TODO: Error handling
              }
            }

            // Read new sector
            fat_read_single_block(cur_sec_start + cur_sec_off, sector);
          }
        }
      }
    }
  }

  // Keep errno from earlier failure

  return errno == 0 ? FAT_SUCCESS : FAT_FAIL;
}

// --- Interface ---

/* Initializes fat32 module.
   This must be done before any calls to other fat32 functions.
   Returns 0 on success, -1 on failure.

   Sets errno == EPERM if already initialized.
   Sets errno == EIO if a bad response in received from disk.
 */
int fat_init(const FatPartitionInfo *pinfo) {
  if (fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  // Set the global partition info
  fat_pinfo = *pinfo;

  // Initialize open files
  int i;
  for (i = 0; i < FAT_MAX_FILES; ++i) {
    fat_open_files[i].free = 1;
    fat_open_files[i].pos = 0;
  }

  fat_initialized = 1;

  errno = 0;
  return FAT_SUCCESS;
}

/*! Opens a file.
    Returns the file descriptor of the opened file or -1 in case of failure.
    Ignores read / write permissions.

    Sets errno == EPERM if the module is not initialized.
    Sets errno == ENOENT if the file can not be found.
    Sets errno == EMFILE if the maximum number of open files is reached.
    Sets errno == EEXIST if file exists and O_CREAT and O_EXCL is set.
 */
int fat_open(const char *path, int oflag) {
  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  int fd = -1;
  uint8_t sector[fat_pinfo.bytes_per_sector];
  FatDirEntryIdx dir_idx;
  FatFile *f;

  long target_pos = 0;

  errno = 0;

  // Check if file exists
  if (FAT_SUCCESS == fat_resolve_path(path, sector, &dir_idx)) {
    if (oflag & O_CREAT & O_EXCL) {
      errno = EEXIST;
    }
    else {
      fd = fat_open_dir_entry(sector, &dir_idx);
      if (fd < 0) {
        errno = EMFILE;
      }
      else if (FAT_DIRECTORY & fat_attrib_of_dir_entry(sector, dir_idx.index)) {
        errno = EISDIR;
      }
      else {
        f = &fat_open_files[fd - FAT_FD_OFFSET];

        if (oflag & O_TRUNC) {
          uint32_t cluster = fat_cluster_of_dir_entry(sector, dir_idx.index);
          uint32_t next_tv = 0;
          if (FAT_SUCCESS == fat_get_table_value(cluster, &next_tv) &&
              !FAT_TV_BAD(next_tv) && !FAT_TV_FREE(next_tv) &&
              (FAT_TV_LAST(next_tv) || // If either the chain is just one cluster
               FAT_SUCCESS == fat_free_cluster_chain(next_tv))) { // Or we cleared it

              // Adjust size
              f->size = 0;
              fat_set_uint32(0, sector + 28 + dir_idx.index * FAT_DIR_ENTRY_WIDTH);

              // Write back directory entry
              uint32_t sec_addr = dir_idx.sector +
                fat_first_sector_of_cluster(dir_idx.cluster);

              fat_write_single_block(sec_addr, sector); // Keep errno on failure
          }
        }

        target_pos = oflag & O_APPEND ? f->size : 0;
      }
    }
  }
  else if (oflag & O_CREAT) {
    if (FAT_SUCCESS == fat_create(path, &dir_idx, sector, &fd, 1)) {
      // Success
    }
    // Keep errno from fat_create
  }

  // Set the cursor
  if (errno == 0) {
    if (target_pos != fat_lseek(fd, target_pos, SEEK_SET)) {
      //printf("Error seeking to %ld in %s (FD=%d)", target_pos, path, fd);
    }
  }

  if (errno != 0 && fd > 0) {
    fat_close(fd);
    fd = -1;
  }

  return fd;
}

/*! Closes a file.
    Returns 0 in case of success, -1 on failure.

    Sets errno == EPERM if the module is not initialized.
    Sets errno == EINVAL if the file descriptor is out of the valid range.
    Sets errno == EBADF if the file descriptor does not match an open file.
*/
int fat_close(int fd) {
  errno = 0;

  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  if (fd < FAT_FD_OFFSET || fd >= FAT_FD_OFFSET + FAT_MAX_FILES) {
    errno = EINVAL;
    return FAT_FAIL;
  }

  FatFile *f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (f->free) {
    errno = EBADF;
    return FAT_FAIL;
  }

  // Closing is simply freeing the fd
  f->free = 1;

  errno = 0;
  return FAT_SUCCESS;
}

/*! Write bytes to a file.
    Returns the number of bytes written to the file, -1 in case of failure.
*/
int fat_write(int fd, uint8_t *buf, uint32_t sz) {
  errno = 0;

  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  if (fd < FAT_FD_OFFSET || fd >= FAT_FD_OFFSET + FAT_MAX_FILES) {
    errno = EINVAL;
    return FAT_FAIL;
  }

  FatFile *f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (f->free) {
    errno = EBADF;
    return FAT_FAIL;
  }

  // TODO: Acquire all needed clusters before writing.
  // This is to minimize the amount of disk_read().

  // TODO: Read block size from somewhere
  uint32_t secsz = fat_pinfo.bytes_per_sector;
  uint32_t bytes_written = 0;
  uint8_t odd_buf[secsz];
  uint8_t *wrbuf; // Where the next write is from

  uint32_t secoff = (f->pos / secsz) % fat_pinfo.sectors_per_cluster;
  uint32_t secfirst = fat_first_sector_of_cluster(f->current_cluster);
  uint32_t sector = secfirst + secoff;

  uint32_t byteoff = f->pos % secsz; // Offset from start of sector
  uint32_t wrsz = 0; // Amount of bytes to write next iteration

  errno = 0; // Set in loop if something goes wrong

  while (sz > 0) {
    wrsz = umin(secsz - byteoff, sz); // Start by writing up to next sector
    sector = secfirst + secoff;

    // If not a whole sector, we must read, change, write
    if (wrsz < secsz) {
      wrbuf = odd_buf; // Write from the odd buffer

      // Read sector
      if (FAT_SUCCESS != fat_read_single_block(sector, odd_buf)) {
        break;
      }

      // Change value
      memcpy(wrbuf + byteoff, buf + bytes_written, wrsz);
    }
    else {
      wrbuf = buf + bytes_written;
    }

    // Write block
    if (FAT_SUCCESS != fat_write_single_block(sector, wrbuf)) {
      break;
    }

    // Update counters
    bytes_written += wrsz;
    sz -= wrsz;
    if (byteoff + wrsz >= secsz) { // Update sector if end is reached
      secoff++;
      if (secoff >= fat_pinfo.sectors_per_cluster) {
        if (FAT_SUCCESS != fat_acquire_next_cluster(&f->current_cluster, 1)) {
          // Out of disk space
          errno = ENOSPC;
          break;
        }

        secoff = 0;
        secfirst = fat_first_sector_of_cluster(f->current_cluster);
      }
    }

    // No byte offset next write (can only happen in first iteration)
    byteoff = 0; // Must happen here as it used in counter update
  }

  // Update size of file
  // FIXME: Check this works with 4GB files
  uint32_t new_size = f->pos + bytes_written;
  if (new_size > f->size) {
    sector = f->dir_idx.sector +
      fat_first_sector_of_cluster(f->dir_idx.cluster);

    if (FAT_SUCCESS != fat_read_single_block(sector, odd_buf)) {
      //printf("Fail Read DirEntry\n"); // TODO: Remove
    }
    if (errno != EIO) {
      fat_set_uint32(new_size,
                     odd_buf + f->dir_idx.index * FAT_DIR_ENTRY_WIDTH + 0x1C);

      if (FAT_SUCCESS != fat_write_single_block(sector, odd_buf)) {
        //printf("Fail Write DirEntry\n"); // TODO: Remove
      }
      f->size = new_size;
    }
  }

  f->pos += bytes_written;
  return bytes_written; // errno is already set
}

/*! Read bytes from a file.
    Returns the number of bytes read, or -1 in case of failure.

    Sets errno == EPERM if the module is not initialized.
    Sets errno == EINVAL if the file descriptor is out of the valid range.
    Sets errno == EBADF if the file descriptor does not match an open file.
    Sets errno == EAGAIN if a bad cluster is encountered during the read.
*/
int fat_read(int fd, uint8_t *buf, uint32_t sz) {
  errno = 0;

  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  if (fd < FAT_FD_OFFSET || fd >= FAT_FD_OFFSET + FAT_MAX_FILES) {
    errno = EINVAL;
    return FAT_FAIL;
  }

  FatFile *f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (f->free) {
    errno = EBADF;
    return FAT_FAIL;
  }

  if (f->pos + sz > f->size) {
    // Reading beyond current end of file.
    sz = f->size - f->pos;
  }

  // TODO: Acquire all needed clusters before writing.
  // This is to minimize the amount of disk_read().

  // TODO: Read block size from somewhere
  uint32_t secsz = fat_pinfo.bytes_per_sector;
  uint32_t bytes_read = 0;
  uint8_t odd_buf[secsz];
  uint8_t *rdbuf; // Where the next read is to
  uint32_t tv; // Value read from FAT

  uint32_t secoff = (f->pos / secsz) % fat_pinfo.sectors_per_cluster;
  uint32_t secfirst = fat_first_sector_of_cluster(f->current_cluster);
  uint32_t sector = secfirst + secoff;

  uint32_t byteoff = f->pos % secsz; // Offset from start of sector
  uint32_t rdsz = 0; // Amount of bytes to read next iteration

  errno = 0; // Set in loop if something goes wrong

  while (sz > 0) {
    rdsz = umin(secsz - byteoff, sz); // Start by writing up to next sector
    sector = secfirst + secoff;

    // If not a whole sector, we must read into the odd buffer
    if (rdsz < secsz) {
      rdbuf = odd_buf; // Read into the odd buffer
    }
    else {
      rdbuf = buf + bytes_read;
    }

    // Read block
    if (FAT_SUCCESS != fat_read_single_block(sector, rdbuf)) {
      //printf("Failed Read\n"); // TODO: Remove
      break;
    }

    // If not a whole sector, we must copy over the data now
    if (rdsz < secsz) {
      memcpy(buf + bytes_read, rdbuf + byteoff, rdsz);
    }

    // Update counters
    bytes_read += rdsz;
    sz -= rdsz;
    if (sz > 0 && byteoff + rdsz >= secsz) { // Update sector if end is reached
      secoff++;
      if (secoff >= fat_pinfo.sectors_per_cluster) {
        if (FAT_SUCCESS != fat_get_table_value(f->current_cluster, &tv)) {
          // Out of disk space
          errno = ENOSPC;
          break;
        }
        else if (FAT_TV_BAD(tv)) {
          //printf("Error: Reading bad cluster\n");
          errno = EBADF;
          break;
        }
        else if (FAT_TV_LAST(tv)) {
          //printf("Error: Reading last cluster\n");
          errno = EBADF; // This should not be reached since sz was adjusted
          break;
        }
        else if (FAT_TV_FREE(tv)) {
          //printf("Error: Reading free cluster\n");
          errno = EBADF;
          break;
        }
        // All of the above errors point to the FAT being corrupted
        f->current_cluster = tv;

        secoff = 0;
        secfirst = fat_first_sector_of_cluster(f->current_cluster);
      }
    }

    // No byte offset next read (can only happen in first iteration)
    byteoff = 0; // Must happen here as it used in counter update
  }

  f->pos += bytes_read;
  return bytes_read; // errno is already set
}

/*! Set the cursor in the open file.
    Returns the absolute offset of the cursor in the file, or -1 in case of failure.
    The parameter whence determines how the pos is interpreted.

    If whence == SEEK_SET then it is in absolute position, in bytes.
    If whence == SEEK_CUR then it is relative to the current position, in bytes.
    If whence == SEEK_END then it is relative to the end of the file, in bytes.

    Sets errno == EPERM if the module is not initialized.
    Sets errno == EINVAL if the file descriptor is out of the valid range.
    Sets errno == EBADF if the file descriptor does not match an open file.
    Sets errno == EAGAIN if a bad cluster is encountered during the search.
    Sets errno == EPIPE if the seek position is outside the range of the file.
*/
off_t fat_lseek(int fd, off_t pos, int whence) {
  errno = 0;

  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  if (fd < FAT_FD_OFFSET || fd >= FAT_FD_OFFSET + FAT_MAX_FILES) {
    errno = EINVAL;
    return FAT_FAIL;
  }

  FatFile *f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (f->free) {
    errno = EBADF;
    return FAT_FAIL;
  }

  long abs_pos = pos;
  if (whence == SEEK_SET) {
    abs_pos = pos;
  }
  else if (whence == SEEK_CUR) {
    abs_pos = f->pos + pos;
  }
  else if (whence == SEEK_END) {
    abs_pos = f->size + pos;
  }
  else {
    errno = EINVAL;
    return FAT_FAIL;
  }

  // We allow seeking to the character just behind the last
  if (abs_pos < 0 || abs_pos > f->size) {
    errno = EPIPE;
    return FAT_FAIL;
  }

  uint32_t bytes_per_cluster = fat_pinfo.bytes_per_sector * fat_pinfo.sectors_per_cluster;

  uint32_t clusters_to_search = 0; // The number of clusters we must search through
  uint32_t bytes_to_search = 0; // The number of bytes we must search through
  uint32_t s_cluster = 0; // The current cluster in the search

  if (abs_pos > f->pos) { // Seeking forward
    bytes_to_search = abs_pos - f->pos;
    clusters_to_search = ((f->pos % bytes_per_cluster) + bytes_to_search) / bytes_per_cluster;
    s_cluster = f->current_cluster;
  }
  else { // Seeking backwards
    bytes_to_search = abs_pos;
    clusters_to_search = bytes_to_search / bytes_per_cluster;
    s_cluster = f->start_cluster;
  }

  uint32_t tv = 0; // Table value read from FAT
  uint32_t i;
  for (i = 0; i < clusters_to_search; i++) {
    if (FAT_SUCCESS != fat_get_table_value(s_cluster, &tv)) {
      break;
    }
    else if (FAT_TV_BAD(tv)) {
      errno = EAGAIN;
      break;
    }
    else if (FAT_TV_LAST(tv)) {
      // This should never happen due to range check
      //printf("Error: EOF in lseek\n");
      return FAT_FAIL;
    }
    else {
      s_cluster = tv;
    }
  }

  f->current_cluster = s_cluster;
  f->pos = abs_pos;

  errno = 0;
  return f->pos;
}

/*! Deletes a file.
  Returns 0 if successful, or -1 in case of failure.

  Sets errno == EPERM  if the module is not initialized.
  Sets errno == ENOENT if the path is not a valid file.
  Sets errno == BUSY   if the file is open.
  Sets errno == EIO    if there occured read / write errors.
  Sets errno == BADF   if the FAT is corrupted.
*/
int fat_unlink(const char *path) {
  errno = 0;

  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  // 1. Resolve path to final directory entry
  uint8_t buf[fat_pinfo.bytes_per_sector];
  FatDirEntryIdx di;
  if (0 != fat_resolve_path(path, buf, &di)) {
    return FAT_FAIL; // Keep errno from path resolution
  }

  // 2. Check if the file is open anywhere
  FatFile *f = 0; // For brevity
  int i = 0;
  for (i = 0; i < FAT_MAX_FILES; i++) {
    f = &fat_open_files[i];
    if (!f->free && f->dir_idx.cluster == di.cluster &&
        f->dir_idx.sector == di.sector &&
        f->dir_idx.index == di.index) {

      errno = EBUSY;
      return FAT_FAIL;
    }
  }

  if (FAT_SUCCESS != fat_delete(path, buf)) {
    return -1;
  }

  return 0;
}

/*! Creates a new file or overwrite an existing one.
  Returns file descriptor if successful, or -1 in case of failure.

  Corresponds to fat_open with oflag equal to O_CREAT | O_WRONLY | O_TRUNC.
 */
int fat_creat(const char *path) {
  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  return fat_open(path, O_CREAT | O_WRONLY | O_TRUNC);
}

/*! Creates a new directory.
  Returns 0 on success, -1 on failure.
*/
int fat_mkdir(const char *path) {
  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  uint8_t sector[fat_pinfo.bytes_per_sector];
  FatDirEntryIdx dir_idx;

  errno = 0;

  // Check if directory already exists
  if (FAT_SUCCESS == fat_resolve_path(path, sector, &dir_idx)) {
    errno = EEXIST;
  }
  else {
    int fd = 0; // Should be ignored
    if (FAT_SUCCESS == fat_create(path, &dir_idx, sector, &fd, 0)) {
      assert(fd == 0);

      // Empty folder
      uint32_t sec_addr = fat_first_sector_of_cluster(dir_idx.cluster);
      if (FAT_SUCCESS == fat_read_single_block(sec_addr, sector)) {
        sector[0] = FAT_DIR_ENTRY_LAST; // Effectively empty the cluster
        fat_write_single_block(sec_addr, sector); // Sets errno
      }
      else {
        errno = EIO;
      }
    }
    else {
      // Keep errno
      return FAT_FAIL;
    }
  }

  return errno == 0 ? FAT_SUCCESS : FAT_FAIL;
}

/*! Removes an empty directory.
  Returns 0 on success, -1 on failure.
*/
int fat_rmdir(const char *path) {
  if (!fat_initialized) {
    errno = EPERM;
    return FAT_FAIL;
  }

  // We read folder contents into this buffer
  // This saves a disk read at the cost of memory
  uint8_t chksector[fat_pinfo.bytes_per_sector];
  uint8_t sector[fat_pinfo.bytes_per_sector];
  FatDirEntryIdx dir_idx;

  errno = 0;

  // Find directory already exists
  if (FAT_SUCCESS != fat_resolve_path(path, sector, &dir_idx)) {
    errno = ENOENT;
  }
  else {
    // Ensure it is a directory
    uint8_t attrib = fat_attrib_of_dir_entry(sector, dir_idx.index);
    if (!(attrib & FAT_DIRECTORY)) {
      errno = ENOTDIR;
    }
    else {
      // Ensure it is empty
      uint8_t entstat = FAT_DIR_ENTRY_FREE;
      uint32_t cluster = fat_cluster_of_dir_entry(sector, dir_idx.index);
      uint32_t sec_start = fat_first_sector_of_cluster(cluster);
      uint32_t sec = 0;
      uint32_t ent = 0;

      fat_read_single_block(sec_start, chksector);
      do {
        entstat = fat_get_uint8(chksector + ent * FAT_DIR_ENTRY_WIDTH);

        // Increase indicies
        if (++ent >= fat_pinfo.entries_per_sector) {
          ent = 0;
          if (++sec >= fat_pinfo.sectors_per_cluster) {
            sec = 0;
            if (FAT_SUCCESS != fat_acquire_next_cluster(&cluster, 0)) {
              break;
            }
            sec_start = fat_first_sector_of_cluster(cluster);
          }
          if (FAT_SUCCESS != fat_read_single_block(sec_start + sec, chksector)) {
            break;
          }
          sec = 0;
        }
      } while (errno == 0 && entstat == FAT_DIR_ENTRY_FREE);

      if (errno == 0 && entstat != FAT_DIR_ENTRY_LAST) {
        errno = ENOTEMPTY;
      }
      else if (errno == 0) {
        // Delete the folder
        fat_delete(path, sector); // Keep errno in case of failure
      }
      // Keep errno from if IO error
    }
  }

  return errno == 0 ? FAT_SUCCESS : FAT_FAIL;
}

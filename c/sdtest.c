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
 * File for testing FAT32 on SD card.
 *
 * Authors: Max Rishoej (maxrishoej@gmail.com)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <machine/spm.h>

// NOTE: Source files are imported here, not headers.
// This is to access functions otherwise hidden.
#include "libsd/sddisk.c"
#include "libsd/fat32.c"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Reused fields
uint8_t res; // Response from SD card
uint8_t buffer[512];
FatPartitionInfo pinfo;
int i;

// --- Utility functions ---

// Prints a directory entry on a single line
void print_dir_entry(uint8_t *buffer, FatDirEntryIdx *dir_idx) {
  int entry_idx = FAT_DIR_ENTRY_WIDTH * dir_idx->index;

  // Check if it's an empty entry
  if (buffer[entry_idx] == 0xE5 || buffer[entry_idx] == 0x00) {
    return;
  }

  int i;

  uint8_t long_name[512];
  uint8_t short_name[12]; // 11 bytes + null terminating
  uint8_t attrib;
  uint32_t first_cluster;
  uint32_t file_size;

  // First get long name, if it exists
  long_name[0] = 0; // Terminate in case of no long name found
  fat_get_long_name_uint8(buffer, dir_idx, long_name);

  // Indicies might have been updated
  entry_idx = FAT_DIR_ENTRY_WIDTH * dir_idx->index;

  fat_get_string(buffer + entry_idx, short_name, 11);
  short_name[11] = 0; // Terminate

  attrib = fat_attrib_of_dir_entry(buffer, dir_idx->index);
  first_cluster = fat_cluster_of_dir_entry(buffer, dir_idx->index);

  file_size = fat_get_uint32(buffer + entry_idx + 0x1C);

  printf("%02ld:", dir_idx->index);
  printf(" %s", short_name);
  printf(" (> %ld)", first_cluster);
  printf("\t[Sz = %ld]", file_size);
  printf("\t%s", long_name);
  printf("\n");
}

// --- Test functions ---
// The prefix "ptest" indicates that the functions always succeed, but just print.
// The prefix "test" indicates that it is an actual test.

// Write data to card and read it back again
// NOTE: Requires reformat of card
int ptest_sd_read_write() {

  // Read block
  res = sd_read_single_block(0, buffer, 512);
  printf("RD Res: %d\n", res);
  sd_print_block(buffer, 512);

  // Alter and write back
  memcpy(buffer, buffer, 512);
  buffer[16] = 0;
  buffer[32] = 0;
  buffer[48] = 0;
  res = sd_write_single_block(0, buffer, 512);
  printf("WR Res: %d\n", res);

  // Read again
  res = sd_read_single_block(0, buffer, 512);
  printf("RD Res: %d\n", res);
  sd_print_block(buffer, 512);

  return 0;
}

// Load partition info and print it
int ptest_pinfo() {
  fat_load_first_partition_info(&pinfo);

  printf("\nPartition Info:\n");
  printf("VolBeginAddr: %ld\n", pinfo.vol_begin_addr);
  printf("BytesPerSector: %ld\n", pinfo.bytes_per_sector);
  printf("SectorsPerCluster: %ld\n", pinfo.sectors_per_cluster);
  printf("NumResSectors: %ld\n", pinfo.num_res_sectors);
  printf("NumFATs: %ld\n", pinfo.num_fats);
  printf("NumDirEntries: %ld\n", pinfo.num_dir_entries);
  printf("NumTotalSectors: %ld\n", pinfo.num_total_sectors);
  printf("SectorsPerFAT: %ld\n", pinfo.sectors_per_fat);
  printf("\n");

  return 0;
}

// Test disk reads
// NOTE: Requires reformat of card
int ptest_root_dir_raw() {
  uint8_t buffer[512];
  uint32_t addr = pinfo.root_dir_addr + 1;

  disk_read(addr, buffer, 0);
  sd_print_block(buffer, 512);

  disk_read(addr, buffer, 1);
  sd_print_block(buffer, 512);

  disk_read(addr, buffer, 2);
  sd_print_block(buffer, 512);

  return 0;
}

// Test FAT lseek and read
int ptest_fat_lseek_read(char *path) {
  int fd = -1;
  uint8_t test_buf3[10 + 1];

  test_buf3[10] = 0;
  fd = fat_open(path, O_RDWR);

  fat_lseek(fd, 5, SEEK_CUR);
  fat_read(fd, test_buf3, 10);
  printf("%s\n", test_buf3);

  fat_lseek(fd, -10, SEEK_CUR);
  fat_read(fd, test_buf3, 10);
  printf("%s\n", test_buf3);

  return 0;
}

// Print directory contents
int ptest_print_dir(uint32_t start_cluster) {
  FatDirEntryIdx dir_idx;
  dir_idx.cluster = start_cluster;
  dir_idx.sector = 0;
  dir_idx.index = 0;
  uint32_t start_sector = 0;

  uint32_t entries_per_sector = pinfo.bytes_per_sector / FAT_DIR_ENTRY_WIDTH;
  int done = 0;

  while (!done) {
    // Read sector
    start_sector = fat_first_sector_of_cluster(dir_idx.cluster);
    disk_read(start_sector + dir_idx.sector, buffer, 1);

    for (dir_idx.index = 0; dir_idx.index < entries_per_sector; dir_idx.index++) {
      print_dir_entry(buffer, &dir_idx);

      // Check first byte to see if last entry
      if (0x00 == buffer[dir_idx.index * FAT_DIR_ENTRY_WIDTH]) {
        done = 1;
        break;
      }
    }

    if (done) {
      break;
    }

    dir_idx.sector++;
    if (dir_idx.sector == pinfo.sectors_per_cluster) {
      dir_idx.sector = 0;

      // We assume folders are well-formed, meaning that the last entry
      // is marked as such and thus we can just get the table value.
      uint32_t tv = 0;
      if (0 != fat_get_table_value(dir_idx.cluster, &tv)) {
        printf("[PrintDir] Error retrieving table value for %ld\n",
               dir_idx.cluster);
        return 0;
      }
      dir_idx.cluster = tv;
    }
  }

  printf("\n");
  return 0;
}

// Print root directory
int ptest_print_root_dir() {
  // Read root folder
  printf("Root Directory:\n");
  ptest_print_dir(pinfo.root_dir_cluster);

  return 0;
}

// Prints a sector of a text file once and then again, one char at a time
int ptest_fat_read_one_char() {
  char *path = "REASON";
  uint8_t test_buf1[512 + 1];
  uint8_t test_buf2[1 + 1];
  uint32_t bytes_read = 0;

  printf("\n1. TIME:\n=========\n");
  for (i = 0; i < 512 + 1; i++) test_buf1[i] = 0;
  int fd = fat_open(path, O_RDWR);
  printf("FD = %d\n", fd);

  for (i = 0; i < 3; i++) {
    bytes_read = fat_read(fd, test_buf1, 512);
    printf("%s", test_buf1);
  }

  printf("\n2. TIME:\n=========\n");
  test_buf2[1] = 0;
  fd = fat_open(path, O_RDWR);
  printf("FD = %d\n", fd);

  // Read entire file, one char at a time
  for (i = 0; i < 362462; i++) {
    bytes_read = fat_read(fd, test_buf2, 1);
    printf("%s", test_buf2);
  }

  return 0;
}

// Test disk_read and disk_write
int ptest_disk_read_write() {
  int i;
  uint8_t buf_big[3*512 + 1];
  uint8_t buf_mid[512 + 1];

  buf_big[3*512] = 0;
  buf_mid[512] = 0;

  uint32_t offset_sector = fat_first_sector_of_cluster(189); // REASON file

  // Big chunk to compare against
  printf("Big chunk:\n===========================\n");
  disk_read(offset_sector, buf_big, 3);
  printf("%s", buf_big);
  printf("\n");

  // Chunked big chunk
  printf("Chunked:\n===========================\n");
  for (i = 0; i < 3*512 / 512; i++) {
    disk_read(offset_sector + i, buf_mid, 1);
    printf("|%d|%s", i, buf_mid);
  }
  printf("\n");

  // Make changes across boundary
  uint32_t write_offset = offset_sector + 1;
  uint8_t *token_begin = (uint8_t *)"| WRITE BEGIN |"; // 15 chars
  uint8_t *token_end = (uint8_t *)"| WRITE END |"; // 13 chars
  uint8_t buf_bak[512];

  disk_read(write_offset, buf_mid, 1);
  memcpy(buf_bak, buf_mid, 512); // Save edited region for restoration
  memcpy(buf_mid, token_begin, 15);
  memcpy(buf_mid + 512 - 13, token_end, 13);
  disk_write(write_offset, buf_mid, 1);

  // Read out big chunk again for comparison
  printf("Big chunk modified:\n===========================\n");
  disk_read(offset_sector, buf_big, 3);
  printf("%s", buf_big);
  printf("\n");

  // Restore edited region
  disk_write(write_offset, buf_bak, 1);

  return 0;
}

int ptest_write_file() {
  uint8_t buf_mid[512 + 1];
  buf_mid[512] = 0;

  char *path = "REASON";
  uint32_t fd = fat_open(path, O_RDWR);

  fat_lseek(fd, 0, SEEK_SET);
  fat_read(fd, buf_mid, 512);
  printf("Original:\n===========================\n");
  printf("%s", buf_mid);

  printf("\n");
  memcpy(buffer, buf_mid, 512);
  memcpy(buffer + 0, path, 6);
  memcpy(buffer + 128, path, 6);
  memcpy(buffer + 256 - 6, path, 6);
  fat_lseek(fd, 0, SEEK_SET);
  fat_write(fd, buffer, 512);

  fat_lseek(fd, 0, SEEK_SET);
  fat_read(fd, buf_mid, 512);
  printf("Original:\n===========================\n");
  printf("%s", buf_mid);

  printf("\n");

  return 0;
}

// Print a sector of the FAT
int ptest_print_fat() {
  uint8_t buf[fat_pinfo.bytes_per_sector];

  uint32_t sec_off = 1;
  uint32_t sec = pinfo.fat_begin_addr + sec_off;

  uint32_t entries_per_sector = fat_pinfo.bytes_per_sector / 4;

  uint32_t c = 0; // Cluster
  uint32_t v = 0; // Value from table
  uint32_t i = 0;

  printf("FAT Sector %ld:\n\n", sec_off);
  for (i = 0; i < entries_per_sector; i++) {
    c = i + sec_off * fat_pinfo.bytes_per_sector / 4;
    v = fat_get_uint32(buf + 4 * i) & 0x0FFFFFFF;
    printf("%3ld: %ld -> %ld\n", i, c, v);
  }

  printf("\n");

  return 0;
}

int ptest_fat_write_file() {
  int fd1, fd2;
  char path1[] = "REASON";
  char path2[] = "FIRSTFILE";

  uint32_t sz = 512;
  uint8_t buf[sz + 1];
  buf[sz] = 0;

  // Open files
  fd1 = fat_open(path1, O_RDWR);
  fd2 = fat_open(path2, O_RDWR);
  printf("Opened %s (FD = %d)\n", path1, fd1);
  printf("Opened %s (FD = %d)\n", path2, fd2);

  // Read something
  fat_read(fd1, buf, sz);
  printf("%s\n", buf);

  // Write it
  FatFile *f = &fat_open_files[fd2 - FAT_FD_OFFSET];
  printf("Writing to %s [Sz=%ld Pos=%ld Clus=%ld]\n",
         path2, f->size, f->pos, f->current_cluster);
  printf("Dir Entry [Cluster = %ld Sector=%ld Index=%ld]\n",
         f->dir_idx.cluster, f->dir_idx.sector, f->dir_idx.index);

  /*
  // Search to end
  fat_lseek(fd2, 0, SEEK_END);
  if (0 != errno) {
    printf("Failed seeking! Errno=%d\n", errno);
    return 1;
  }
  printf("Searched to end in %s [Sz=%ld Pos=%ld CurCluster=%ld]\n",
         path2, f->size, f->pos, f->current_cluster);
  */

  /*
  // Search to start
  fat_lseek(fd2, 0, SEEK_SET);
  if (0 != errno) {
    printf("Failed seeking! Errno=%d\n", errno);
    return 1;
  }
  printf("Searched to beginning in %s [Sz=%ld Pos=%ld CurCluster=%ld]\n",
         path2, f->size, f->pos, f->current_cluster);
  */

  // Write
  uint32_t clusters_to_write = 2 * 1024; // 1MB

  buf[0] = '[';
  buf[2] = ']';
  uint32_t i = 0;
  for (i = 0; i < 64 + 2; i++) {
    buf[1] = '0' + (i % 10);
    if (sz != fat_write(fd2, buf, sz)) {
      printf("Failed writing! Errno=%d\n", errno);
      return 1;
    }
  }
  printf("\nDone writing to %s [Sz=%ld Pos=%ld Clus=%ld]\n",
         path2, f->size, f->pos, f->current_cluster);

  printf("\n");

  return 0;
}

int ptest_fat_print_file(char *path) {
  int fd;

  // Open file
  fd = fat_open(path, O_RDWR);
  if (fd < 0) {
    int err = errno;
    printf("Error opening %s (FD = %d). Errno=%d\n", path, fd, err);
    return 1;
  }

  // Read something
  uint32_t rsz = 512;
  uint32_t sz = 512;
  do {
    sz = fat_read(fd, buffer, sz);
    printf("%s", buffer);
  } while (sz == rsz);
  printf("\n");

  fat_close(fd);

  return 0;
}

// Writes ascending numbers to a file and read them back and verifies them
int test_fat_write_and_verify(int fd, uint32_t sz, int wrsz) {
  int i;
  uint8_t buf[wrsz + 1];

  // Write numbers
  uint32_t written = 0;
  while (written < sz) {
    sprintf((char *)buf, "%06d\n", i++); // Ensure this is wrsz characters
    if (wrsz != fat_write(fd, buf, wrsz)) {
      printf("[TEST] Failed writing: Errno=%d\n", errno);
      return 1;
    }
    written += wrsz;
  }

  // Seek to beginning
  if (0 != fat_lseek(fd, 0, SEEK_SET)) {
    printf("[TEST] Error seeking to start in (FD=%d): Errno=%d\n", fd, errno);
    return 1;
  }

  // Read numbers back
  uint32_t read = 0;
  i = 0;
  uint32_t v;
  char *ptr; // Needed for strtol
  while (read < written) {
    if (wrsz != fat_read(fd, buf, wrsz) && read < written) {
      printf("[TEST] Error reading (FD=%d): Errno=%d Written=%ld Read=%ld\n",
             fd, errno, written, read);
      return 1;
    }
    read += wrsz;

    // Verify numbers
    v = strtol((char *)buf, &ptr, 10);
    if (v != i++) {
      printf("[TEST] Error in (FD=%d): Line=%d but Value=%ld\n", fd, i, v);
      return 1;
    }
  }

  return 0;
}

// Writes a long chain of numbers to a file, and verifies them afterwards
int test_fat_write_read_numbers_limit(char *path, uint32_t limit, int oflag) {
  int fd;
  uint32_t wr_sz = 7; // 6 chars + \n
  //uint32_t limit = wr_sz * 10;
  uint8_t buf[wr_sz + 1];

  // Open file
  fd = fat_open(path, oflag | O_RDWR);
  if (fd < 0) {
    printf("[TEST] Error opening %s\n", path);
    return 1;
  }

  // Write numbers
  uint32_t written = 0;
  uint32_t i = 0;
  while (written < limit) {
    sprintf((char *)buf, "%06ld\n", i++); // Ensure this is wr_sz characters
    if (wr_sz != fat_write(fd, buf, wr_sz)) {
      printf("[TEST] Failed writing: Errno=%d\n", errno);
      return 1;
    }
    written += wr_sz;
  }

  // Seek to beginning
  if (0 != fat_lseek(fd, 0, SEEK_SET)) {
    printf("[TEST] Error seeking to start in %s (FD=%d): Errno=%d\n", path, fd, errno);
    return 1;
  }

  // Read numbers back
  uint32_t read = 0;
  i = 0;
  uint32_t v;
  char *ptr; // Needed for strtol
  while (read < written) {
    if (wr_sz != fat_read(fd, buf, wr_sz) && read < written) {
      printf("[TEST] Error reading %s (FD=%d): Errno=%d Written=%ld Read=%ld\n",
             path, fd, errno, written, read);
      return 1;
    }
    read += wr_sz;

    // Verify numbers
    v = strtol((char *)buf, &ptr, 10);
    if (v != i++) {
      printf("[TEST] Error in %s (FD=%d): Line=%ld but Value=%ld\n", path, fd, i, v);
      return 1;
    }
  }

  if (0 != fat_close(fd)) {
    printf("[TEST] Error in closing %s (FD=%d)\n", path, fd);
    return 1;
  }

  printf("[TEST] Successfully wrote and verified %ld lines (%ld bytes) in %s (FD=%d).\n",
         i, read, path, fd);
  return 0;
}
int test_fat_write_read_numbers(char *path) {
  uint32_t limit = 3 * pinfo.sectors_per_cluster * pinfo.bytes_per_sector;
  return test_fat_write_read_numbers_limit(path, limit, O_RDWR);
}

// Prints the cluster chain beginning a cluster
int ptest_fat_values(uint32_t cluster) {
  uint32_t tv = 0;

  uint32_t i = 0;
  do {
    if (0 != fat_get_table_value(cluster, &tv)) {
      printf("Error getting table value: Errno=%d Cluster=%ld\n", errno, cluster);
      break;
    }
    else if (FAT_TV_BAD(tv)) {
      printf("BAD Value %ld from %ld\n", tv, cluster);
      return 1;
    }
    else if (FAT_TV_FREE(tv)) {
      printf("FREE Value %ld from %ld\n", tv, cluster);
      return 1;
    }
    else {
      printf("Cluster %ld -> %ld\n", cluster, tv);
    }

    cluster = tv;
  } while (!FAT_TV_LAST(tv));

  return 0;
}

// Print FAT table values between two clusters
int ptest_fat_table(uint32_t start_cluster, uint32_t end_cluster) {
  uint32_t cur_sec = 0;
  uint32_t cur_ent = 0;
  uint32_t cur_tv = 0;
  uint32_t fat_start = pinfo.fat_begin_addr;

  uint32_t entries_per_sector = pinfo.bytes_per_sector / 4;
  uint8_t buf[pinfo.bytes_per_sector];

  uint32_t sec_start = 4 * start_cluster / pinfo.bytes_per_sector;
  uint32_t sec_end = 4 * end_cluster / pinfo.bytes_per_sector;
  uint32_t secs = sec_end - sec_start;

  uint32_t i, j;
  for (i = 0; i < secs; i++) { // Loop through sectors
    cur_sec = sec_start + i;
    disk_read(fat_start + cur_sec, buf, 1);
    for (j = 0; j < entries_per_sector; j++) { // Loop through entries
      cur_ent = cur_sec * entries_per_sector + j;
      cur_tv = fat_get_uint32(buf + 4 * j) & 0x0FFFFFFF;
      printf("%ld -> %ld\n", cur_ent, cur_tv);
    }
  }

  return 0;
}

// Writes and verifies number in a file, prints the cluster chain,
// deletes the file and does it to the test file just before.
// Read output to verify they occupy same clusters.
int ptest_fat_unlink(char *path) {
  FatFile *f = 0;
  int fd = 0;

  // 1. Write multiple clusters to test file n
  if (0 != test_fat_write_read_numbers(path)) {
    printf("[TEST] Aborted due to error in writing numbers to \"%s\"\n", path);
    return 1;
  }

  // 2. Print clusters of file n
  fd = fat_open(path, O_RDWR);
  if (fd < 0) {
    printf("[TEST] Aborted due to error in opening file \"%s\"\n", path);
    return 1;
  }
  f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (0 != ptest_fat_values(f->start_cluster)) {
    printf("[TEST] Aborted due to error in printing cluster chain of \"%s\" (FD=%d)\n",
           path, fd);
    return 1;
  }
  if (0 != fat_close(fd)) {
    printf("[TEST] Aborted due to error in closing file \"%s\" (FD=%d)\n", path, fd);
    return 1;
  }

  // 3. Unlink file n
   if (0 != fat_unlink(path)) {
    printf("[TEST] Aborted due to error in unlinking file \"%s\"\n", path);
    return 1;
  }
  printf("[TEST] Successfully unlinked file \"%s\"\n", path);

  // 4. Write multiple clusters to test file n-1
  if (0 != test_fat_write_read_numbers(path)) {
    printf("[TEST] Aborted due to error in writing numbers to \"%s\"\n", path);
    return 1;
  }

  // 5. Print clusters of file n-1
  fd = fat_open(path, O_RDWR);
  if (fd < 0) {
    printf("[TEST] Aborted due to error in opening file \"%s\"\n", path);
    return 1;
  }
  f = &fat_open_files[fd - FAT_FD_OFFSET];
  if (0 != ptest_fat_values(f->start_cluster)) {
    printf("[TEST] Aborted due to error in printing cluster chain of \"%s\" (FD=%d)\n",
           path, fd);
    return 1;
  }
  if (0 != fat_close(fd)) {
    printf("[TEST] Aborted due to error in closing file \"%s\" (FD=%d)\n", path, fd);
    return 1;
  }

  // NOTE: File n-1 should occupy the same clusters as file n did
  return 0;
}

// Performs above test on multiple files
int ptest_unlink_many(char *format_path, int minidx, int maxidx) {
  char path[256];

  int i = 0;
  for (i = maxidx; i >= minidx; --i) {
    sprintf(path, format_path, i);
    if (0 != ptest_fat_unlink(format_path)) {
      printf("Errno=%d\n", errno);
      return 1;
    }
  }
  return 0;
}

// Creates a file
int ptest_create(char *path) {
  int fd;

  fd = fat_open(path, O_CREAT);
  if (fd < 0) {
    printf("[TEST] Aborted due to error in creating file \"%s\"\n", path);
    return 1;
  }
  fat_close(fd);

  test_fat_write_read_numbers(path);

  return 0;
}

// Create enough files to extend the folder by a cluster.
// Write and verify a tiny amount in each folder.
// Takes a sprintf format path as argument, that must have exactly one %d.
int ptest_create_many(char *format_path) {
  int fd;
  int i;
  char path[256];

  int entries = 1 + (fat_pinfo.sectors_per_cluster * fat_pinfo.bytes_per_sector / 32);
  int verify_sz = 16;

  // Generate random-ish name
  const uint8_t c_min = 'A';
  const uint8_t c_max = 'Z';
  uint8_t rand = 0x36;
  int k;

  for (i = 0; i < 256; i++)
    path[i] = 0; // Empty array

  for (i = 0; i < entries; i++) {
    sprintf(path, format_path, i);
    for (k = 0; '.' != path[k]; k++) {
      rand ^= path[k] ^ i;
      path[k] = c_min + (rand % (c_max - c_min));
    }

    fd = fat_open(path, O_CREAT | O_EXCL);
    if (fd < 0 || errno != 0) {
      /*
      printf("[TEST] Aborted due to error in creating file \"%s\": %d\n",
             path, errno);
      return 1;
      */
      printf("[TEST] Skipped due to error in creating file \"%s\": %d\n",
             path, errno);
      continue;
    }
    fat_close(fd);

    if (0 != test_fat_write_read_numbers_limit(path, verify_sz, O_RDWR)) {
      printf("[TEST] Aborted due to error in verifying file \"%s\": %d\n",
             path, errno);
      return 1;
    }
  }

  return 0;
}

// Create a file in a folder
int ptest_create_in_folder(char *dir_path, char *file_name) {
  int fd;
  int verify_sz = 16;
  char path[256];

  // Create folder
  if (FAT_SUCCESS == fat_mkdir(dir_path)) {
    printf("[TEST] Successfully created folder \"%s\"\n", dir_path);
  }
  else {
    /*
    printf("[TEST] Aborted due to error in creating folder \"%s\": %d\n",
            path, errno);
    return 1;
    */
    printf("[TEST] Folder \"%s\" already existed\n", dir_path);
  }

  // Create file
  sprintf(path, "%s/%s", dir_path, file_name);
  fd = fat_open(path, O_CREAT | O_EXCL);
  if (fd < 0 || errno != 0) {
    printf("[TEST] Aborted due to error in creating file \"%s\": %d\n",
          path, errno);
    return 1;
  }
  fat_close(fd);

  // Verify
  if (0 != test_fat_write_read_numbers_limit(path, verify_sz, O_RDWR)) {
    printf("[TEST] Aborted due to error in verifying file \"%s\": %d\n",
          path, errno);
    return 1;
  }

  return 0;
}

// Create a tree of folders with files in the leaf folders
// Write and verify a tiny amount in each leaf file.
// Takes a sprintf format path as argument, that must have exactly /two/ %d.
int ptest_create_tree(char *format_path, int height, int width) {
  int fd;
  int j, k;
  char path[256];
  int verify_sz = 16;

  for (i = 0; i < 256; i++) {
    path[i] = 0;
  }

  uint32_t name_begin = 0;
  uint32_t path_len = 0;

  for (j = 0; j < width; j++) {
    // Create path
    sprintf(path, format_path, height, j);

    for (k = 0; k < 256; k++) {
      if (path[k] == '/') {
        name_begin = k + 1;
      }
      if (path[k] == 0) {
        path_len = k;
        break;
      }
    }
    if (k == 256 | path_len >= 256) {
      printf("[TEST] Aborted due to path being too long \"%s\"\n", path);
      return 1;
    }

    // Create files / folders
    if (height > 0) { // Creating folders
      if (FAT_SUCCESS == fat_mkdir(path)) {
        printf("[TEST] Successfully created folder \"%s\"\n", path);

        // Recurse
        path[path_len] = '/';
        path_len++;
        memcpy(path + path_len, format_path + name_begin, path_len - name_begin + 1);
        ptest_create_tree(path, height - 1, width);
      }
      else {
        printf("[TEST] Aborted due to error in creating folder \"%s\": %d\n",
              path, errno);
        return 1;
      }
    }
    else { // Creating leaves
      fd = fat_open(path, O_CREAT | O_EXCL);
      if (fd < 0 || errno != 0) {
        printf("[TEST] Skipped due to error in creating file \"%s\": %d\n",
              path, errno);
        continue;
      }
      fat_close(fd);

      if (0 != test_fat_write_read_numbers_limit(path, verify_sz, O_RDWR)) {
        printf("[TEST] Aborted due to error in verifying file \"%s\": %d\n",
              path, errno);
        return 1;
      }
    }
  }

  return 0;
}

// A composite test that covers many features:
// 1. Create new directory
// 2. Create files in directory, writing and verifying data in each
// 3. Delete all the files
// 4. Delete now empty folder
int test_fat_create_verify_delete(char *dirpath, char *fpath, int n, int sz) {
  int i;
  int fd;
  char path[256];

  // Create directory
  if (FAT_SUCCESS != fat_mkdir(dirpath)) {
    printf("[TEST] Error in creating folder \"%s\": %d\n", dirpath, errno);
  }

  // Create files
  for (i = 0; i < n; i++) {
    sprintf(path, fpath, i);
    fd = fat_open(path, O_CREAT | O_EXCL);
    if (fd < 0) {
      printf("[TEST] Error in creating file \"%s\": %d\n", path, errno);
    }
    if (0 != fat_close(fd)) {
      printf("[TEST] Error in closing file \"%s\": %d\n", path, errno);
      break;
    }

    // Verify
    if (0 != test_fat_write_read_numbers_limit(path, sz, 0)) {
      break;
    }
  }

  // Delete all the files
  for (i = 0; i < n; i++) {
    sprintf(path, fpath, i);

    if (0 != fat_unlink(path)) {
      printf("[TEST] Error in deleting file \"%s\": %d\n", path, errno);
      break;
    }
  }

  // Delete the folder
  if (0 != fat_rmdir(dirpath)) {
    printf("[TEST] Error in deleting folder \"%s\": %d\n", dirpath, errno);
    return 1;
  }

  return 0; // TODO: Return error on failure
}

int main() {
  int errsv = 0;

  printf("\n");

  // Initialize card
  if (0 != disk_init()) {
    errsv = errno;
    printf("Error initializing SD card! Errno=%d\n", errsv);
    return errsv;
  }
  printf("Card initialized\n");

  // Load partition info
  if (0 != fat_load_first_partition_info(&pinfo)) {
    errsv = errno;
    printf("Error loading FAT partition! Errno = %d\n", errsv);
    return errsv;
  };
  printf("PartitionInfo loaded\n");

  // Initialize FAT
  if (0 != fat_init(&pinfo)) {
    errsv = errno;
    printf("Error initializing FAT! Errno = %d\n", errsv);
    return errsv;
  }
  printf("FAT initialized\n");

  // Print tests
  ptest_pinfo();
  ptest_print_root_dir();
  test_fat_create_verify_delete("TempDir", "TempDir/LongFileName%d", 2, 5);

  return 0;
}

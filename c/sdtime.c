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
 * File for timing FAT32 on SD card.
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

// Prints timing of disk reads
int ptest_time_disk_read(uint32_t sz) {
  uint8_t buf[512 + 1];

  printf("[TIME] Reading %ld bytes: ", sz);
  clock_t begin = clock();

  uint32_t sec = fat_first_sector_of_cluster(pinfo.root_dir_cluster);
  uint32_t secs = sz / pinfo.bytes_per_sector;

  do {
    disk_read(sec++, buf, 1);
    secs--;
  } while (secs > 0);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%fs\n", time_spent);

  return 0;
}

// Prints timing of disk writes
int ptest_time_disk_write(uint32_t sz) {
  uint8_t buf[512 + 1];

  printf("[TIME] Writing %ld bytes: ", sz);
  clock_t begin = clock();

  uint32_t sec = fat_first_sector_of_cluster(1000); // Arbitrary number
  uint32_t secs = sz / pinfo.bytes_per_sector;

  do {
    disk_write(sec++, buf, 1);
    secs--;
  } while (secs > 0);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%fs\n", time_spent);

  return 0;
}

// Prints timing of file writes
// Only times writing, not creation / opening / closing
int ptest_time_fat_write(char *path, uint32_t wrsz, uint32_t sz, uint32_t off) {
  uint8_t buf[512 + 1];
  int fd;

  // Create file if it does not exist
  fd = fat_open(path, O_RDWR | O_CREAT);
  if (fd < 0) {
    printf("[TIME] Aborted due to error in opening file \"%s\": %d\n", path, errno);
    return 1;
  }

  // Offset
  if (off != fat_write(fd, buf, off)) {
    printf("[TIME] Aborted due to error when writing %ld offset bytes in file \"%s\": %d\n", off, path, errno);
    return 1;
  }

  printf("[TIME] Writing %ld bytes to \"%s\" in %ld chunks: ", sz, path, wrsz);
  clock_t begin = clock();
  uint32_t td = 0;

  do {
    td += fat_write(fd, buf, wrsz);
  } while (td < sz);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%fs\n", time_spent);

  // Close file
  fat_close(fd);

  return 0;
}

int ptest_time_fat_read(char *path, uint32_t rdsz, uint32_t sz, uint32_t off) {
  uint8_t buf[512 + 1];
  int fd;

  // Create file if it does not exist
  fd = fat_open(path, O_RDWR);
  if (fd < 0) {
    printf("[TIME] Aborted due to error in opening file \"%s\": %d\n", path, errno);
    return 1;
  }

  // Offset the cursor
  if (off != fat_lseek(fd, off, SEEK_SET)) {
    printf("[TIME] Aborted due to error when seeking to %ld in file \"%s\": %d\n", off, path, errno);
    return 1;
  }

  printf("[TIME] Reading %ld bytes from \"%s\" in %ld chunks: ", sz, path, rdsz);
  clock_t begin = clock();
  uint32_t td = 0;

  do {
    td += fat_read(fd, buf, rdsz);
  } while (td < sz);

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%fs\n", time_spent);

  // Close file
  fat_close(fd);

  return td < sz;
}

// Times creating a lot of small files in a folder
int ptest_time_fat_create_many(char *dirpath, char *filepath, int n) {
  int i;
  int fd;

  // Prepare directory path
  char fpath[256];
  sprintf(fpath, dirpath, n);

  // Create directory if it does not exist
  if (0 != fat_mkdir(fpath)) {
    printf("[TIME] Aborted due to error in creating folder \"%s\": %d\n", fpath, errno);
  }
  printf("[TIME] Creating %d files in directory \"%s\": ", n, fpath);

  // Setup full format path
  sprintf(fpath, "%s/%s", fpath, filepath);
  char path[256];

  // Begin clock
  clock_t begin = clock();

  while (n > 0) {
    sprintf(path, fpath, n);
    fd = fat_open(path, O_CREAT | O_EXCL);
    if (fd < 0) {
      break;
    }
    if (0 != fat_close(fd)) {
      break;
    }

    n--;
  }

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("%fs\n", time_spent);

  return n ? 1 : 0;
}

// Deletes a single file that already exists
int ptest_time_deletion_single(char *path) {
  // Time and delete the file
  clock_t begin = clock();

  if (0 != fat_unlink(path)) {
    printf("[TEST] Error in deleting file \"%s\": %d\n", path, errno);
    return 1;
  }

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("[TIME] Deleted \"%s\": %f\n", path, time_spent);

  return 0;
}

// Times the deletion of a single large file that is created before timing
int ptest_time_deletion_single_create(char *dirpath, char *fpath, int sz) {
  int i;
  int fd;
  uint8_t buf[512];
  char path[256];

  // Create directory
  if (FAT_SUCCESS != fat_mkdir(dirpath)) {
    printf("[TEST] Error in creating folder \"%s\": %d\n", dirpath, errno);
  }

  // Create file
  sprintf(path, fpath, i);
  fd = fat_open(path, O_CREAT | O_EXCL);
  if (fd < 0) {
    printf("[TEST] Error in creating file \"%s\": %d\n", path, errno);
  }

  // Write to file
  uint32_t td = 0;
  do {
    td += fat_write(fd, buf, 512);
  } while (td < sz);

  if (0 != fat_close(fd)) {
    printf("[TEST] Error in closing file \"%s\": %d\n", path, errno);
    return 1;
  }

  printf("[TEST] File \"%s\" (size = %d) created\n", path, sz);

  // Time and delete the file
  ptest_time_deletion_single(path);

  // Delete the folder
  if (0 != fat_rmdir(dirpath)) {
    printf("[TEST] Error in deleting folder \"%s\": %d\n", dirpath, errno);
    return 1;
  }

  return 0; // TODO: Return error on failure
}

// Times deletion of many existing files
int ptest_time_deletion_many(char *fpath, int n) {
  int i;
  char path[256];

  // Time and delete the file
  int num = n;
  clock_t begin = clock();

  while (n > 0) {
    sprintf(path, fpath, n);
    if (0 != fat_unlink(path)) {
      printf("[TEST] Error in deleting file \"%s\": %d\n", path, errno);
      return 1;
    }
    n--;
  }

  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("[TIME] Deleted %d files: %f\n", num, time_spent);

  return n == 0;
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

  // --- Tests ---
  /*
  for (i = 1; i < 128; i *= 2) {
    if (0 != ptest_time_fat_read("szone/sz128", 512, i * 1000 * 1000, 3)) {
      printf("[TIME] Aborted due to incomplete test with %d megabytes\n", i);
      break;
    }
  }
  */

  /*
  for (i = 1; i <= 2048; i *= 2) {
    if (0 != ptest_time_fat_create_many("L%d", "F0123456789ABCDEF%d", i)) {
      printf("[TIME] Aborted due to incomplete test with %d files\n", i);
      break;
    }
  }
  */

  /*
  for (i = 1; i <= 128; i *= 2) {
    if (0 != ptest_time_deletion_single_create("SzOne/Time0.txt", 1000 * 1000 * i)) {
      printf("[TIME] Aborted due to incomplete test with %d bytes\n", i);
    }
  }
  */

  /*
  ptest_time_deletion_single("SzOne/Sz1");
  ptest_time_deletion_single("SzOne/Sz2");
  ptest_time_deletion_single("SzOne/Sz4");
  ptest_time_deletion_single("SzOne/Sz8");
  ptest_time_deletion_single("SzOne/Sz16");
  ptest_time_deletion_single("SzOne/Sz32");
  ptest_time_deletion_single("SzOne/Sz64");
  ptest_time_deletion_single("SzOne/Sz128");
  */

  /*
  ptest_time_deletion_many("SzMany/D1/F%d", 1);
  ptest_time_deletion_many("SzMany/D2/F%d", 2);
  ptest_time_deletion_many("SzMany/D4/F%d", 4);
  ptest_time_deletion_many("SzMany/D8/F%d", 8);
  ptest_time_deletion_many("SzMany/D16/F%d", 16);
  ptest_time_deletion_many("SzMany/D32/F%d", 32);
  ptest_time_deletion_many("SzMany/D64/F%d", 64);
  ptest_time_deletion_many("SzMany/D128/F%d", 128);
  ptest_time_deletion_many("SzMany/D256/F%d", 256);
  ptest_time_deletion_many("SzMany/D512/F%d", 512);
  ptest_time_deletion_many("SzMany/D1024/F%d", 1024);
  */

  return 0;
}

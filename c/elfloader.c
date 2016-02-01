/*
   Copyright 2016 Technical University of Denmark, DTU Compute. 
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
 * Download ELF files via TFTP and execute them using virtual memory.
 * 
 * Authors: Wolfgang Puffitsch (wpuffitsch@gmail.com)
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <machine/patmos.h>
#include <machine/exceptions.h>

#include "libelf/libelf.h"
#include "ethlib/udp.h"

/**********************************************************************/
/* Aligned memory allocation                                          */
/**********************************************************************/

#define SEG_ALIGN 16
#define ALIGN(PTR,A) ((typeof(PTR))(((unsigned int)PTR + A-1) & ~(A-1)))

void *aligned_alloc(size_t size, size_t align) {
  void *ptr = memalign(align, size);
  if (ptr == NULL) {
    fprintf(stderr, "error: out of memory\n");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

/**********************************************************************/
/* Low-level functionality                                            */
/**********************************************************************/

#define MMU_SEG ((volatile _IODEV int *)0xf0070000)

#define HEAP_SIZE  (512*1024)
#define STACK_SIZE (128*1024)

void nop(void) __attribute__((noinline));
int invoke(unsigned int entry) __attribute__((noinline));

// go to user mode
static inline void user_mode(void) {
  EXC_STATUS &= ~0x2;
  asm volatile("nop; nop;");
}

void super_mode(void) {
  trap(8);
}

void super_mode_trap(void) __attribute__((naked));
void super_mode_trap(void) {
  exc_prologue();

  unsigned int sxb;
  asm volatile("mfs %0 = $sxb;" : "=r" (sxb));
  // only enable super mode when being called from the right function
  if (sxb == (unsigned int)&invoke) {
    EXC_STATUS |= 0x8;
  }

  exc_epilogue();
}

// setup up a segment
void setup_segment(int hw_seg, unsigned int base,
                   unsigned int perm, unsigned int length) {
  length = ALIGN(length, SEG_ALIGN);

  MMU_SEG[2*hw_seg+0] = base; /* base */
  MMU_SEG[2*hw_seg+1] = ((perm & 07) << 29) | (length & 0x1fffffff); /* perm | length */
}

// a no-op function to clobber call return address registers
void nop(void) {
  asm volatile ("");
}

// variables to remember the stack pointers
static unsigned int top;
static unsigned int r31;
static unsigned int ssz;

int invoke(unsigned int entry) {
  // clobber return address registers
  nop();

  unsigned int tmp1, tmp2;
  asm volatile (// save stack pointers
                "mfs %0 = $st;"
                "li %1 = %2;"
                "swm [%1] = %0;"
                "mfs %1 = $ss;"
                "sub %0 = %1, %0;"
                "li %1 = %3;"
                "swm [%1] = %0;"
                "sspill %0;"
                "li %0 = %4;"
                "swm [%0] = $r31;"
                : "=r" (tmp1), "=r" (tmp2)
                : "i" (&top), "i" (&ssz), "i" (&r31));

  exc_register(8, &super_mode_trap);
  user_mode();

  asm volatile (// call function
                "callnd %5;"
                // keep return value
                "mov %1 = $r1;"
                // restore stack pointers
                "li %0 = %2;"
                "lwm %0 = [%0];"
                "nop;"
                "mts $ss = %0;"
                "mts $st = %0;"
                "li %0 = %3;"
                "lwm %0 = [%0];"
                "nop;"
                "sens %0;"
                "li %0 = %4;"
                "lwm $r31 = [%0];"
                "nop;"

                : "=r" (tmp1), "=r" (tmp2)
                : "i" (&top), "i" (&ssz), "i" (&r31), "r" (entry)
                : "$r2", "$r3", "$r4", "$r5",
                  "$r6", "$r7", "$r8", "$r9",
                  "$r10", "$r11", "$r12", "$r13",
                  "$r14", "$r15", "$r16", "$r17",
                  "$r18", "$r19", "$r20", "$r21",
                  "$r22", "$r23", "$r24", "$r25",
                  "$r26", "$r27", "$r28", "$r29",
                  "$r30", "$r31");

  super_mode();

  // clear caches
  inval_dcache();
  inval_mcache();

  return tmp2;
}

/**********************************************************************/
/* Download via TFTP                                                  */
/**********************************************************************/

#define RX_ADDR  0x000
#define TX_ADDR  0x800
#define ARP_ADDR 0xc00

static const int tftp_port = 69;
static const int target_port = 6969;
static       int host_port = 0;

unsigned char host_ip [4] = { 192, 168, 24, 1 };

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5

void tftp_send_rrq(char *filename) {
  const char *mode = "octet";

  unsigned char tftp_rrq [2 + strlen(filename)+1 + strlen(mode)+1];
  size_t idx = 0;

  // TFTP read request
  tftp_rrq[idx++] = '\0';
  tftp_rrq[idx++] = TFTP_RRQ;

  // file name
  for (size_t i = 0; i <= strlen(filename); i++, idx++) {
    tftp_rrq[idx] = filename[i];
  }

  // transmission mode: octal
  for (size_t i = 0; i <= strlen(mode); i++, idx++) {
    tftp_rrq[idx] = mode[i];    
  }

  // transmit TFTP request via UDP
  udp_send(TX_ADDR, ARP_ADDR, host_ip, target_port, tftp_port, tftp_rrq, idx, 10000);
}

void tftp_send_ack(unsigned short int block) {

  unsigned char tftp_ack [4];
  size_t idx = 0;

  // TFTP acknowledgement
  tftp_ack[idx++] = '\0';
  tftp_ack[idx++] = TFTP_ACK;
  tftp_ack[idx++] = block >> 8;
  tftp_ack[idx++] = block & 0xff;

  // transmit TFTP acknowledgement via UDP
  udp_send(TX_ADDR, ARP_ADDR, host_ip, target_port, host_port, tftp_ack, idx, 10000);
}

size_t tftp_receive(char *filename, char **buffer) {
  size_t size = 0;

  tftp_send_rrq(filename);

  for (;;) {
    eth_mac_receive(RX_ADDR, 0);
    unsigned char packet_type = mac_packet_type(RX_ADDR);

    // receive UDP packet
    if (packet_type == 2) {
      unsigned char src_ip[4];
      ipv4_get_source_ip(RX_ADDR, src_ip);

      // from correct IP address, at the correct port
      if (ipv4_compare_ip(host_ip, src_ip) &&
          udp_get_destination_port(RX_ADDR) == target_port) {
        // remember host port
        host_port = udp_get_source_port(RX_ADDR);

        // inspect TFTP packet
        unsigned int  tftp_length = udp_get_data_length(RX_ADDR);
        unsigned char tftp_packet [tftp_length];
        udp_get_data(RX_ADDR, tftp_packet, tftp_length);

        unsigned short int op = (tftp_packet[0] << 8) | tftp_packet[1];

        if (op == TFTP_ERROR) {
          // print out error message and abort download
          unsigned short int code = (tftp_packet[2] << 8) | tftp_packet[3];
          fprintf(stderr, "\nerror: TFTP error %d: %s\n", code, &tftp_packet[4]);
          return -1;

        } else if (op == TFTP_DATA) {
          // received valid data
          unsigned short int block = (tftp_packet[2] << 8) | tftp_packet[3];
          unsigned short int block_length = tftp_length-4;
          size_t start = (block-1)*512;
          size_t end = start + block_length;

          if (end > size) {
            // increase buffer size
            size_t new_size = size;
            while (new_size < end) {
              new_size = ((new_size+1)*3) >> 1; // increase by ~50%
            }

            // copy data into new buffer
            void *new_buffer = aligned_alloc(new_size, SEG_ALIGN);
            memcpy(new_buffer, *buffer, size);
            free(*buffer);

            *buffer = new_buffer;
            size = new_size;
          }

          // fill in received data
          memcpy(&(*buffer)[start], &tftp_packet[4], block_length);

          // acknowledge reception
          tftp_send_ack(block);
          if (block_length < 512) {
            return size;
          }
        }
      }
    }
    // respond to ARP requests
    if (packet_type == 3) {
      arp_process_received(RX_ADDR, TX_ADDR);
    }
  }
}

/**********************************************************************/
/* ELF file loading                                                   */
/**********************************************************************/

static void *segment_alloc [8];

unsigned int elf_load(char *buffer, size_t size) {
  // check libelf version
  elf_version(EV_CURRENT);

  // open elf binary
  Elf *elf = elf_memory(buffer, size);
  if (elf == NULL) {
    fprintf(stderr, "error: %s\n", elf_errmsg(0));
    elf_end(elf);
    return 0;
  }

  // check file kind
  Elf_Kind ek = elf_kind(elf);
  if (ek != ELF_K_ELF) {
    fprintf(stderr, "error: ELF file must be of kind ELF.\n");
    elf_end(elf);
    return 0;
  }
  // get elf header
  Elf32_Ehdr *hdr = elf32_getehdr(elf);
  if (hdr == NULL) {
    fprintf(stderr, "error: %s\n", elf_errmsg(0));
    elf_end(elf);
    return 0;
  }
  if (hdr->e_machine != 0xBEEB) {
    fprintf(stderr, "error: ELF file is not a Patmos ELF file.\n");
    elf_end(elf);
    return 0;
  }

  // get program headers
  size_t n;
  int ntmp = elf_getphdrnum (elf, &n);
  assert(ntmp == 0);

  // get program headers
  Elf32_Phdr *phdr = elf32_getphdr(elf);
  assert(phdr);

  for (size_t i = 0; i < n; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      // some assertions
      assert(phdr[i].p_filesz <= phdr[i].p_memsz);

      if ((phdr[i].p_vaddr & 0x1fffffff) != 0) {
        fprintf(stderr, "error: invalid virtual segment address: %08lx\n", phdr[i].p_vaddr);
        elf_end(elf);
        return 0;
      }

      unsigned int hw_seg = phdr[i].p_vaddr >> 29;

      void *segment;
      ssize_t segment_size = phdr[i].p_memsz;
      if ((phdr[i].p_flags & PF_W) || (segment_size > phdr[i].p_filesz)) {
        // allocate memory for writable segments and segments that are
        // larger than in the file

        // assume that a writable segment includes the heap
        if (phdr[i].p_flags & PF_W) {
          segment_size += HEAP_SIZE;
        }

        // set segment contents
        segment = aligned_alloc(segment_size, SEG_ALIGN);
        segment_alloc[hw_seg] = segment;

        memcpy(segment, &buffer[phdr[i].p_offset], phdr[i].p_filesz);
        memset(&segment[phdr[i].p_filesz], 0, segment_size-phdr[i].p_filesz);
      } else {
        // use ELF buffer for read-only segments
        segment = &buffer[phdr[i].p_offset];
      }

      if (((unsigned int)segment & (SEG_ALIGN-1)) != 0) {
        fprintf(stderr, "error: unaligned segment base: %08x\n", (unsigned int)segment);
        elf_end(elf);
        return 0;
      }

      unsigned int perm = 0;
      if (phdr[i].p_flags & PF_R) { perm |= 04; }
      if (phdr[i].p_flags & PF_W) { perm |= 02; }
      if (phdr[i].p_flags & PF_X) { perm |= 01; }

      setup_segment(hw_seg, (unsigned int)segment, perm, segment_size);
    }
  }

  elf_end(elf);
  return hdr->e_entry;
}

/**********************************************************************/
/* Print a T-CREST banner                                             */
/**********************************************************************/

#define FANCY_BANNER

void banner(void) {
  const unsigned char logo [12][10] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0f, 0xf8, 0x03, 0xe0, 0xff, 0x07, 0xfc, 0x7e, 0x7f, 0xc0 },
    { 0x0f, 0xf8, 0x0f, 0xf8, 0xff, 0xc7, 0xfd, 0xff, 0x7f, 0xc0 },
    { 0x01, 0xc0, 0x1e, 0x3c, 0xe1, 0xe7, 0x01, 0xc2, 0x0e, 0x00 },
    { 0x01, 0xc0, 0x1c, 0x1c, 0xe0, 0xe7, 0x01, 0xc0, 0x0e, 0x00 },
    { 0x01, 0xcf, 0x9c, 0x00, 0xe1, 0xe7, 0xf8, 0xfc, 0x0e, 0x00 },
    { 0x01, 0xcf, 0x9c, 0x00, 0xe7, 0x87, 0xf8, 0x3f, 0x0e, 0x00 },
    { 0x01, 0xc0, 0x1c, 0x1c, 0xe7, 0x87, 0x00, 0x03, 0x8e, 0x00 },
    { 0x01, 0xc0, 0x1e, 0x3c, 0xe3, 0xc7, 0x00, 0x83, 0x8e, 0x00 },
    { 0x01, 0xc0, 0x0f, 0xf8, 0xe3, 0xc7, 0xfd, 0xff, 0x8e, 0x00 },
    { 0x01, 0xc0, 0x03, 0xe0, 0xe1, 0xe7, 0xfc, 0xfe, 0x0e, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
  };

  printf("\x1b[0m\x1b[2J\x1b[1;1H\x1b[34m\x1b[47m\x1b[1m\x1b[?25l");

#ifdef FANCY_BANNER
  for (int k = 0; k < sizeof(logo[0]); k++) {
    for (int i = 0; i < 8; i++) {
      for (int l = 0; l < sizeof(logo)/sizeof(logo[0]); l++) {
        printf("\x1b[%d;%dH", l+1, 8*k+i+1);
        putchar((logo[l][k] >> (7-i)) & 1 ? '#' : ' ');
        fflush(stdout);
      }
    }
  }
  printf("\x1b[1E\x1b[22m");
#else
  for (int l = 0; l < sizeof(logo)/sizeof(logo[0]); l++) {
    for (int k = 0; k < sizeof(logo[0]); k++) {
      for (int i = 0; i < 8; i++) {
        putchar((logo[l][k] >> (7-i)) & 1 ? '#' : ' ');
      }
    }
    printf("\x1b[1E");
  }
#endif /* FANCY_BANNER */

  puts("        TIME-PREDICTABLE MULTI-CORE ARCHITECTURE FOR EMBEDDED SYSTEMS           ");

  puts("\x1b[0m\x1b[?25h");
}

/**********************************************************************/
/* The main function                                                  */
/**********************************************************************/

int main(void) {

  eth_mac_initialize();
  arp_table_init();

  banner();

  for (;;) {
    // set up a buffer to download the ELF file
    ssize_t elf_size = -1;
    char *elf_buf = aligned_alloc(1, SEG_ALIGN);

    do {
      // get file name
      fprintf(stdout, "> ");
      char filename [1024];
      fgets(filename, sizeof(filename), stdin);
      if (strchr(filename, '\n') != NULL) {
        *strchr(filename, '\n') = '\0';
      }
      if (strcmp(filename, "") == 0) {
        // do nothing
      } else if (strcmp(filename, "help") == 0) {
        fprintf(stdout, "Usage:\n");
        fprintf(stdout, "  help   print this help\n");
        fprintf(stdout, "  banner print banner\n");
        fprintf(stdout, "  quit   exit application\n");
        fprintf(stdout, "  <file> download and execute <file>\n");
      } else if (strcmp(filename, "banner") == 0) {
        banner();
      } else if (strcmp(filename, "quit") == 0) {
        exit(EXIT_SUCCESS);
      } else {
        fprintf(stdout, "loading file %s...", filename);
        fflush(stdout);

        // read ELF file via TFTP
        elf_size = tftp_receive(filename, &elf_buf);

        fprintf(stdout, "done\n");
      }
    } while (elf_size < 0);

    // load the ELF file
    unsigned int entry = elf_load(elf_buf, elf_size);

    if (entry != 0) {
      // make sure the system segment is set up
      setup_segment(0, 0, 07, 0);

      // set up a stack segment
      void *stack = aligned_alloc(STACK_SIZE, SEG_ALIGN);
      setup_segment(4, (unsigned int)stack, 06, STACK_SIZE);

      // call the entry function of the loaded file
      int retval = invoke(entry);
      
      // release stack segment again
      free(stack);

      fprintf(stdout, "exit code: %d\n", retval);
    }

    // release any allocated segments
    for (int i = 0; i < sizeof(segment_alloc)/sizeof(segment_alloc[0]); i++) {
      if (segment_alloc[i] != NULL) {
        free(segment_alloc[i]);
        segment_alloc[i] = NULL;
      }
    }

    // release buffer for the the ELF file
    free(elf_buf);
  }

  return 0;
}

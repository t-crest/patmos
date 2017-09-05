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
 * \file librticap.c Definitions for librticap.
 * 
 * \author Luca Pezzarossa <lpez@dtu.dk>
 *
 * \brief Low-level RT-ICAP controller communication library for the T-CREST platform.
 */

#include "rticap.h"

//volatile _UNCACHED unsigned int *random_array = NULL;

static unsigned int current_bank = 0xFFFFFFFF; 
static volatile _UNCACHED unsigned int *random_array = NULL;

///////////////////////////////////////////////////////////////////////////////
// Funtions to manage the recon SPM
///////////////////////////////////////////////////////////////////////////////

/// Kernel function: write single word on the BRAM buffer NB address must be xxx..xx00
void rticap_reconspm_write32(unsigned int * addr, unsigned int data){
   if (current_bank != ((unsigned int) addr >> BRAM_ADDR_WIDTH))
   {
      current_bank = ((unsigned int) addr >> BRAM_ADDR_WIDTH);
      *((volatile unsigned int _IODEV *)(RTICAP_BRAM_BASE | 0x00008000)) = current_bank;
      //printf("WR - Changing bank: addr %08X - bank %d\n", (unsigned int) addr, current_bank);
   }
   *((volatile unsigned int _IODEV *)(RTICAP_BRAM_BASE | (((unsigned int) addr) & (BANK_SIZE-1)))) = data;
   return;
}

/// Kernel function: read single word on the BRAM buffer 
unsigned int rticap_reconspm_read32(unsigned int * addr){
   if (current_bank != ((unsigned int) addr >> BRAM_ADDR_WIDTH))
   {
      current_bank = ((unsigned int) addr >> BRAM_ADDR_WIDTH);
      *((volatile unsigned int _IODEV *)(RTICAP_BRAM_BASE | 0x00008000)) = current_bank;
      //printf("RD - Changing bank: addr %08X - bank %d\n", (unsigned int) addr, current_bank);
   }
   return *((volatile unsigned int _IODEV *)(RTICAP_BRAM_BASE | (((unsigned int) addr) & (BANK_SIZE-1))));
}

//This function performs the SPM buffer memory test
//It returns:
// - (0) memory ok
// - (-1) fail
// - (-2) failed to dynamically allocate memory - test not performed
int rticap_reconspm_test(unsigned seed){
   free((void *)random_array);
   random_array = NULL;
   unsigned int i;
   srand(seed);
   //Allocate dynamic memeory
   random_array = (volatile _UNCACHED unsigned int *) malloc(BANK_NUMBER * BANK_WORD_SIZE * sizeof(*random_array));
   if (random_array == NULL)
   {
      //printf("Dynamic memory allocation failed.\n");
      return -2;
   }

   //printf("Filling the array with random values.\n");
   //Filling the array with random values
   for (int i = 0; i < BANK_NUMBER * BANK_WORD_SIZE; i++)
   {
      random_array[i] = ((unsigned int)rand());
   }
   //printf("Ready to test.\n\n");

   //Copy the bram into the
   for (i = 0; i < BANK_NUMBER * BANK_WORD_SIZE; i++)
   {
        //*(RTICAP_BRAM_BASE+i) = random_array[i];
         rticap_reconspm_write32((unsigned int *) (i<<2), random_array[i]);
   }

   //Check against the main memory
   for (i = 0; i < BANK_NUMBER * BANK_WORD_SIZE; i++)
   {
       if ((rticap_reconspm_read32((unsigned int *) (i<<2))) != random_array[i])
       {
            //printf("I failed at address %08X, sorry.\n\n", (i<<2));
            return -1;//break;
       }
   }

   //printf("FYI: the first random word was %08X and the last was %08X.\n\n", random_array[0], random_array[BANK_NUMBER * BANK_WORD_SIZE -1]);

   free((void *)random_array);
   random_array = NULL;

   return 0;
}

//////////////////////////////
////  GET functions
//////////////////////////////

/// This function reads a rt-icap controller register
unsigned short int rticap_get_status(){
   return (unsigned short int)(rticap_read_reg(RTICAP_STATUS_REG) & 0x0000FFFF);
}

/// This function reads a rt-icap controller register
unsigned short int rticap_get_icap_status(){
   return (unsigned short int)(rticap_read_reg(RTICAP_STATUS_REG) & 0xFFFF0000);
}

/// This function reads a rt-icap controller register
unsigned int rticap_get_signature(){
   return (rticap_read_reg(RTICAP_SIGNATURE_REG));
}

//////////////////////////////
////  SET functions
//////////////////////////////


/// This function reads a rt-icap controller register
unsigned int rticap_read_reg(volatile unsigned int _IODEV * reg_addr){
   return *reg_addr;
}

/// This function writes a rt-icap controller register
void rticap_write_reg(volatile unsigned int _IODEV * reg_addr, unsigned int data){
   *reg_addr = data;
   return;
}




/*
//This function returns the size of the BRAM buffer
//It returns:
// - (>=0) size of the BRAM buffer
unsigned int rticap_BRAM_size(){
   return RTICAP_BRAM_SIZE;
}
*/



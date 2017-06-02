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
 * \file librticap.h Definitions for librticap.
 * 
 * \author Luca Pezzarossa <lpez@dtu.dk>
 *
 * \brief Low-level RT-ICAP controller communication library for the T-CREST platform.
 */

#ifndef _RTICAP_H_
#define _RTICAP_H_

#include <machine/patmos.h>
#include <machine/spm.h>

//#include <machine/boot.h>
//#include <machine/exceptions.h>


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
   unsigned int id;
   void * address;
   unsigned int length; //in bytes
} Bitstream;


///////////////////////////////////////////////////////////////////////////////
// Definitions for the multi-bank reconfigration SMP
///////////////////////////////////////////////////////////////////////////////
// OCP_ADDR_WIDTH  => 16,       -- must be 16 (the 2 LSB are not used) the MSB is always the bank_select enable bit
#define BRAM_ADDR_WIDTH 15 //      -- this detemines the size of each bank (must be < or = than OCP_ADDR_WIDTH-1)
#define BANK_ADDR_WIDTH 3 //       -- this detemines the number of banks (3 is 8 banks)

#define BANK_SIZE 32768 //in bytes
#define BANK_WORD_SIZE 8192 //in words
#define BANK_NUMBER 8  

/// This function reads a rt-icap controller register
unsigned int rticap_read_reg(volatile unsigned int _IODEV * reg_addr);

/// This function writes a rt-icap controller register
void rticap_write_reg(volatile unsigned int _IODEV * reg_addr, unsigned int data);

//This function measures the size of the BRAM buffer
//It returns:
// - (>=0) size of the BRAM buffer
// - (-1) not able to measure
// - (-2) memory failure happened
unsigned int rticap_BRAM_size();

//This function performs the SPM buffer memory test
//It returns:
// - (0) memory ok
// - (-1) fail
// - (-2) failed to dynamically allocate memory - test not performed
int rticap_reconspm_test();

/// Write single 32 bit word on the reconfigration SPM
void rticap_reconspm_write32(unsigned int * addr, unsigned int data);

/// Read a single 32 bit word from the reconfigration SPM 
unsigned int rticap_reconspm_read32(unsigned int * addr);

///////////////////////////////////////////////////////////////////////////////
// Definitions for address mapping
///////////////////////////////////////////////////////////////////////////////

/// The base address of the RT-ICAP registers
#define RTICAP_REG_BASE    ((volatile unsigned int _IODEV *)0xF00C0000)//DEFINE THIS BEFORE FIRST RUN

/// The base address of the BRAM (for the processor)
#define RTICAP_BRAM_BASE 0xF00B0000//DEFINE THIS BEFORE FIRST RUN
#define RTICAP_BRAM_BASE32 ((volatile unsigned int _IODEV *) RTICAP_BRAM_BASE)//DEFINE THIS BEFORE FIRST RUN

/// The size the BRAM expressed in bytes (N.B. the bram can be accessed only word by word xxx..xx00b)
#define RTICAP_BRAM_SIZE 65536

/// The address of the registers
#define RTICAP_STATUS_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+0))
#define RTICAP_CONTROL_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+1))
#define RTICAP_BS_LENGTH_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+2))
#define RTICAP_BS_ADDR_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+3))
#define RTICAP_STREAM_IN_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+4))

#define RTICAP_REG_5  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+5))
#define RTICAP_REG_6  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+6))
#define RTICAP_SIGNATURE_REG  ((volatile unsigned int _IODEV *)(RTICAP_REG_BASE+7))

/// IcapStatus register values
///constant ICAP_CFG_ERR_BIT     : natural := 4;
///constant ICAP_DATA_SYNC_BIT   : natural := 3;
///constant ICAP_READ_I_P_BIT    : natural := 2;
///constant ICAP_ABORT_I_P_BIT   : natural := 1;
///constant ICAP_BUSY_BIT		  : natural := 0;

/// Controller status register values
#define READY_STATUS 0             // := "000";
#define READY_AND_DONE_STATUS 1    // := "001";
#define READY_AND_FAIL_STATUS 2    // := "010";
#define WAIT_BUSY_ICAP_STATUS 3    // := "011";
#define WRITE_IN_PROGRESS_STATUS 4 // := "100";
#define WAIT_END_STATUS 5          // := "101";
#define ABORT_IN_PROGRESS_STATUS 6 // := "110";
#define ND_STATUS 7                // := "111";

/// Controller command register values
#define SW_RESET_COMMAND 0          // := "00";
#define ABORT_COMMAND 1             // := "01";
#define START_CPU_STREAM_COMMAND 2  // := "10";
#define START_RAM_STREAM_COMMAND 3 // := "11";

#endif /* _RTICAP_H_ */
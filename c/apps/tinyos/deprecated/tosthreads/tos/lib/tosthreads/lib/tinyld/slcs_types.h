/*
 * Copyright (c) 2008 Johns Hopkins University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
 */

#ifndef _SLCS_TYPES_H
#define _SLCS_TYPES_H

#include "tosthread.h"
#include "tosthread_leds.h"
#include "tosthread_amradio.h"
#include "tosthread_blockstorage.h"
#include "tosthread_logstorage.h"
#include "tosthread_configstorage.h"
#include "tosthread_threadsync.h"
#include "tosthread_amserial.h"
#include "tosthread_queue.h"

#include "tosthread_sensirionSht11.h"
#include "tosthread_hamamatsuS10871.h"
#include "tosthread_hamamatsuS1087.h"

struct value_addr_pair {
  uint16_t value;
  void *addr;
};

struct addr {
  void *addr;
};

struct prog_desc {
  uint16_t main_addr;   // Loadable program's main function (or tosthread_main() in our case)
  uint16_t alloc_count;
  uint16_t alloc_size;
  uint16_t g_reloc_count;
  uint16_t l_reloc_count;
  uint16_t datasec_count;
  uint16_t code_count;
  
  uint16_t patch_table_count;   // alloc_count + g_reloc_count + l_reloc_count;
  uint16_t code_offset;   // sizeof(main_addr) +
                          // sizeof(alloc_count) +
                          // sizeof(alloc_size) +
                          // sizeof(g_reloc_count) +
                          // sizeof(l_reloc_count) +
                          // sizeof(datasec_count) +
                          // sizeof(code_count) +
                          // (g_sym_count + patch_table_count) * 4
  
  uint16_t loading_stage;
};

#endif

/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

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

//
// Basic implementation of the instruction cache wrapper for Patmos.
//
#include "instr-cache.h"

#include "memory.h"
#include "exception.h"
#include "simulation-core.h"

#include <cmath>
#include <ostream>

#include <boost/format.hpp>

using namespace patmos;

bool no_instr_cache_t::fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[NUM_SLOTS])
{
  // TODO In case of using a data cache, we should optionally assert on two
  // misses, in case the hardware does not support this, so that we can
  // debug alignment with pasim.

  for (; Fetched < NUM_SLOTS; Fetched++) {

    uword_t addr = address + Fetched * sizeof(word_t);

    bool status = Memory->read(s, addr,
                               reinterpret_cast<byte_t*>(&Fetch_cache[Fetched]),
                               sizeof(word_t), true);
    if (!status) {
      Is_miss[Fetched] = true;
      return false;
    }
  }

  // all words have been fetched into the cache, copy to iw and finish.
  bool first_miss = Is_miss[0];
  int misses = 0;

  for (unsigned int i = 0; i < NUM_SLOTS; i++) {
    iw[i] = Fetch_cache[i];
    if (Is_miss[i]) {
      misses++;
      Is_miss[i] = false;
    }
  }

  // update stats counter
  if (misses == 0) {
    Num_hits++;
  } else if (misses == NUM_SLOTS) {
    Num_all_miss++;
  } else if (misses == 1 && first_miss) {
    Num_first_miss++;
  } else {
    Num_succ_miss++;
  }

  Fetched = 0;

  return true;
}

bool no_instr_cache_t::load_method(simulator_t &s, uword_t address, word_t offset)
{
  return true;
}

bool no_instr_cache_t::is_available(simulator_t &s, uword_t address)
{
  return true;
}


void no_instr_cache_t::print_stats(const simulator_t &s, std::ostream &os,
                                   const stats_options_t& options)
{
  uint64_t total = Num_hits + Num_all_miss + Num_first_miss + Num_succ_miss;

  os << boost::format("   Fetch requests   : %1$10d\n"
                      "   All hits         : %2$10d  %3$10.2f%%\n"
                      "   All misses       : %4$10d  %5$10.2f%%\n"
                      "   First miss       : %6$10d  %7$10.2f%%\n"
                      "   Non-first miss   : %8$10d  %9$10.2f%%\n")
   % total
   % Num_hits       % (100.0 * (float)Num_hits / (float)total)
   % Num_all_miss   % (100.0 * (float)Num_all_miss / (float)total)
   % Num_first_miss % (100.0 * (float)Num_first_miss / (float)total)
   % Num_succ_miss  % (100.0 * (float)Num_succ_miss  / (float)total);
}

void no_instr_cache_t::reset_stats() {
  Num_all_miss = 0;
  Num_first_miss = 0;
  Num_succ_miss = 0;
  Num_hits = 0;
}


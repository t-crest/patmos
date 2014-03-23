//
//  This file is part of the Patmos Simulator.
//  The Patmos Simulator is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  The Patmos Simulator is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with the Patmos Simulator. If not, see <http://www.gnu.org/licenses/>.
//
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
                               sizeof(word_t));
    if (!status) {
      Is_miss[Fetched] = true;
      return false;
    }
  }
  
  // all words have been fetched into the cache, copy to iw and finish.
  bool first_miss = Is_miss[0];
  int misses = 0;
  
  for (int i = 0; i < NUM_SLOTS; i++) {
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

bool no_instr_cache_t::load_method(simulator_t &s, word_t address, word_t offset)
{
  return true;
}

bool no_instr_cache_t::is_available(simulator_t &s, word_t address)
{
  return true;
}


void no_instr_cache_t::print_stats(const simulator_t &s, std::ostream &os, 
                                   bool short_stats)
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


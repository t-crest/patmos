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
// Instruction scratchpad interface.
//
// The I-SPM is implemented as a wrapper for a backing instruction cache. 
//

#ifndef PATMOS_INSTR_SPM_H
#define PATMOS_INSTR_SPM_H

#include "instr-cache.h"

#include "simulation-core.h"

namespace patmos
{ 
  class instr_spm_t : public instr_cache_t
  {
  private:
    
    /// A map of function addresses to access counters per function.
    typedef std::map<word_t, uint64_t> method_stats_t;
    
    /// The backing memory.
    memory_t *Memory;

    /// The default instruction cache to use for all other accesses.
    instr_cache_t *Cache;

    /// The size of the I-SPM.
    word_t Size;
    
    /// Number of load requests for the SPM.
    uint64_t Num_loads;
    
    method_stats_t Method_stats;
    
  public:
    /// Construct a new instruction SPM instance.
    /// The memory passed to this SPM is not owned by this cache and must be
    /// managed externally. The cache is owned and managed by the SPM.
    /// @param memory The memory that is accessed through the SPM.
    /// @param cache the cache to use for non-SPM accesses.
    /// @param size the size of the SPM. It will be mapped to [0..size).
    instr_spm_t(memory_t &memory, instr_cache_t *icache, word_t size) 
    : Memory(&memory), Cache(icache), Size(size),
      Num_loads(0)
    {
    }
    
    virtual ~instr_spm_t() {
      if (Cache) delete Cache;
    }

    virtual void initialize(simulator_t &s, uword_t address) { 
      Cache->initialize(s, address); 
    }

    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[NUM_SLOTS]);

    virtual bool load_method(simulator_t &s, word_t address, word_t offset);

    virtual bool is_available(simulator_t &s, word_t address);
    
    virtual void tick(simulator_t &s) { Cache->tick(s); }

    virtual void print(const simulator_t &s, std::ostream &os) { 
      Cache->print(s, os);
    }

    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const stats_options_t& options);
    
    virtual void reset_stats();
    
    virtual void flush_cache();
  };

}

#endif // PATMOS_INSTR_CACHE_H

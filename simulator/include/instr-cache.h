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
// Basic definitions of interfaces to simulate data caches.
//

#ifndef PATMOS_INSTR_CACHE_H
#define PATMOS_INSTR_CACHE_H

#include "memory.h"
#include "data-cache.h"

#include "simulation-core.h"

namespace patmos
{ 
  /// Basic interface for instruction-caches implementations.
  class instr_cache_t
  {
  private:
  public:
    virtual ~instr_cache_t() {}

    /// Initialize the cache before executing the first instruction.
    /// @param address Address to fetch initial instructions.
    virtual void initialize(simulator_t &s, uword_t address) = 0;

    /// A simulated instruction fetch from the instruction cache.
    /// @param base The current method base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[2]) = 0;

    /// Ensure that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// Has no effect on other caches.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(simulator_t &s, word_t address, word_t offset) = 0;

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(simulator_t &s, word_t address) = 0;

    /// Notify the cache that a cycle passed.
    virtual void tick(simulator_t &s) = 0;

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) = 0;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options) = 0;
    
    /// Reset statistics.
    virtual void reset_stats() = 0;
    
    /// Flush the cache.
    virtual void flush_cache() = 0;
  };
  

  class no_instr_cache_t : public instr_cache_t
  {
  private:
    /// The global memory.
    memory_t *Memory;

    /// Number of words fetched so far for the current fetch request.
    word_t Fetched;
    
    /// Words fetched so far for the current fetch request.
    word_t Fetch_cache[NUM_SLOTS];
    
    /// Was the current slot access a miss?
    bool Is_miss[NUM_SLOTS];
    
    /// Number of fetch requests with only misses
    uint64_t Num_all_miss;
    
    /// Number of fetch requests with a single miss in the first slot
    uint64_t Num_first_miss;
    
    /// Number of fetch requests with a single miss not in the first slot
    uint64_t Num_succ_miss;
    
    /// Number of fetch requests without misses
    uint64_t Num_hits;
    
  public:
    /// Construct a new instruction cache instance.
    /// The memory passed to this cache is not owned by this cache and must be
    /// managed externally.
    /// @param memory The memory that is accessed through the cache.
    no_instr_cache_t(memory_t &memory) 
    : Memory(&memory), Fetched(0),
      Num_all_miss(0), Num_first_miss(0), Num_succ_miss(0), Num_hits(0)
    {
      for (int i = 0; i < NUM_SLOTS; i++) {
        Is_miss[i] = false;
      }
    }
    
    virtual void initialize(simulator_t &s, uword_t address) {}

    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[NUM_SLOTS]);

    virtual bool load_method(simulator_t &s, word_t address, word_t offset);

    virtual bool is_available(simulator_t &s, word_t address);
    
    virtual void tick(simulator_t &s) {}

    virtual void print(std::ostream &os) {}

    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const stats_options_t& options);
    
    virtual void reset_stats();
    
    virtual void flush_cache() {}
  };
  
  
  /// An instuction cache using a backing data cache.
  /// @param owning_cache set to true if this cache should own the given memory.
  template<bool IS_OWNING_CACHE>
  class instr_cache_wrapper_t : public no_instr_cache_t
  {
  private:
    /// The global memory or backing cache.
    data_cache_t *Backing_cache;
    
  public:
    /// Construct a new instruction cache instance.
    /// @param memory The memory that is accessed through the cache.
    instr_cache_wrapper_t(data_cache_t *data_cache) 
    : no_instr_cache_t(*data_cache), Backing_cache(data_cache)
    {
    }
    virtual ~instr_cache_wrapper_t() {
      if (IS_OWNING_CACHE) {
        delete Backing_cache;
      }
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick(simulator_t &s)
    {
      if (IS_OWNING_CACHE) {
        Backing_cache->tick(s);
      }
      
      no_instr_cache_t::tick(s);
    }

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os)
    {
      if (IS_OWNING_CACHE) {
        Backing_cache->print(os);
        os << "\n";
      }
      
      no_instr_cache_t::print(os);;
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options)
    {
      if (IS_OWNING_CACHE) {
        Backing_cache->print_stats(s, os, options);
        os << "\n";
      }

      no_instr_cache_t::print_stats(s, os, options);
    }
    
    virtual void reset_stats() {
      if (IS_OWNING_CACHE) {
        Backing_cache->reset_stats();
      }
      no_instr_cache_t::reset_stats();
    }
    
    virtual void flush_cache() {
      Backing_cache->flush_cache();
    }
  };

}

#endif // PATMOS_INSTR_CACHE_H

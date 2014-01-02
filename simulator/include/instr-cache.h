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
// Basic definitions of interfaces to simulate data caches.
//

#ifndef PATMOS_INSTR_CACHE_H
#define PATMOS_INSTR_CACHE_H

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
    virtual void initialize(uword_t address) = 0;

    /// A simulated instruction fetch from the instruction cache.
    /// @param base The current method base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2]) = 0;

    /// Ensure that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// Has no effect on other caches.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address, word_t offset) = 0;

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address) = 0;

    /// Notify the cache that a cycle passed.
    virtual void tick() = 0;

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) = 0;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os) = 0;
    
    /// Reset statistics.
    virtual void reset_stats() = 0;
    
    /// Flush the cache.
    virtual void flush_cache() = 0;
  };
  

  class no_instr_cache_t : public instr_cache_t
  {
  protected:
    /// The global memory.
    memory_t *Memory;

    /// Number of words fetched so far for the current fetch request.
    word_t Fetched;
    
    /// Words fetched so far for the current fetch request.
    word_t Fetch_cache[NUM_SLOTS];
    
  public:
    /// Construct a new instruction cache instance.
    /// The memory passed to this cache is not owned by this cache and must be
    /// managed externally.
    /// @param memory The memory that is accessed through the cache.
    no_instr_cache_t(memory_t &memory) : Memory(&memory), Fetched(0)
    {
    }
    
    virtual void initialize(uword_t address) {}

    virtual bool fetch(uword_t base, uword_t address, word_t iw[2])
    {
      uword_t addr = address + Fetched * sizeof(word_t);
      
      bool status;
      status = Memory->read(addr, 
                            reinterpret_cast<byte_t*>(&Fetch_cache[Fetched]),
                            sizeof(word_t));
      if (!status) return false;
      
      Fetched++;
      
      if (Fetched < NUM_SLOTS) return false;
      
      // all words have been fetched into the cache, copy to iw and finish.
      for (int i = 0; i < NUM_SLOTS; i++) {
        iw[i] = Fetch_cache[i];
      }
      
      Fetched = 0;
      
      return true;
    }

    virtual bool load_method(word_t address, word_t offset)
    {
      return true;
    }

    virtual bool is_available(word_t address)
    {
      return true;
    }
    
    virtual void tick() {}

    virtual void print(std::ostream &os) {}

    virtual void print_stats(const simulator_t &s, std::ostream &os) {}
    
    virtual void reset_stats() {}
    
    virtual void flush_cache() {}
  };
  
  
  /// An instuction cache using a backing data cache.
  /// @param owning_cache set to true if this cache should own the given memory.
  template<bool IS_OWNING_MEMORY>
  class instr_cache_wrapper_t : public instr_cache_t
  {
  protected:
    /// The backing cache.
    data_cache_t *Backing_cache;
    
  public:
    /// Construct a new instruction cache instance.
    /// @param memory The memory that is accessed through the cache.
    instr_cache_wrapper_t(data_cache_t *data_cache) : Backing_cache(data_cache)
    {
    }
    virtual ~instr_cache_wrapper_t() {
      if (IS_OWNING_MEMORY) {
        delete Backing_cache;
      }
    }

    /// Initialize the cache before executing the first instruction.
    /// @param address Address to fetch initial instructions.
    virtual void initialize(uword_t address)
    {
    }

    /// A simulated instruction fetch from the method cache.
    /// @param base The current method's base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2])
    {
      // We could be more intelligent here and check if address is 64bit aligned
      // In this case we only have one miss. If it is not aligned, we might
      // assume that the first word is already in the cache, provided that RET
      // ensures that returning to a PC fetches the block at that PC.
      // This requires all jump-targets to be 64bit aligned though.

      // TODO we should at least assert on two misses, in case the hardware
      // does not support this, so that we can debug alignment with pasim.
      
      for (int i = 0; i < NUM_SLOTS; i++) {
        bool status;
        uword_t addr = address + i * sizeof(word_t);
        status = Backing_cache->read(addr, reinterpret_cast<byte_t*>(&iw[i]),
                              sizeof(word_t));
        if (!status) return false;
      }
      return true;
    }

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address, word_t offset)
    {
      return true;
    }

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address)
    {
      return true;
    }
    
    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      if (IS_OWNING_MEMORY) {
        Backing_cache->tick();
      }
    }

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os)
    {
      if (IS_OWNING_MEMORY) {
        Backing_cache->print(os);
      }
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os)
    {
      if (IS_OWNING_MEMORY) {
        Backing_cache->print_stats(s, os);
      }
    }
    
    virtual void reset_stats() {
      if (IS_OWNING_MEMORY) {
        Backing_cache->reset_stats();
      }
    }
    
    virtual void flush_cache() {
      Backing_cache->flush_cache();
    }
  };

}

#endif // PATMOS_INSTR_CACHE_H


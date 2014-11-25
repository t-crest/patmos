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

#ifndef PATMOS_DATA_CACHE_H
#define PATMOS_DATA_CACHE_H

#include "memory.h"

namespace patmos
{
  /// A data cache.
  class data_cache_t : public memory_t
  {
  public:
    virtual ~data_cache_t() {}
    
    virtual void flush_cache() = 0;
  };

  /// An ideal data cache.
  class ideal_data_cache_t : public data_cache_t
  {
  protected:
    /// The 'cached' memory.
    memory_t &Memory;

  public:
    /// Construct a new data cache instance.
    /// @param memory The memory that is accessed through the cache.
    ideal_data_cache_t(memory_t &memory) : Memory(memory)
    {
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      Memory.read_peek(s, address, value, size);
      return true;
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      Memory.write_peek(s, address, value, size);
      return true;
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      Memory.read_peek(s, address, value, size);
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      Memory.write_peek(s, address, value, size);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      return true;
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      // this is directly invoked by the simulation main loop -- thus no need
      // to propagate this down to the memory here.
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      // nothing to be done here
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             bool short_stats)
    {
      // nothing to be done here
    }
    
    virtual void reset_stats() {}
    
    virtual void flush_cache() {}
  };

  /// A data cache always accessing a memory.
  class no_data_cache_t : public ideal_data_cache_t
  {
  public:
    /// Construct a new data cache instance.
    /// @param memory The memory that is accessed through the cache.
    no_data_cache_t(memory_t &memory) : ideal_data_cache_t(memory)
    {
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      return Memory.read(s, address, value, size);
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      return Memory.write(s, address, value, size);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      return Memory.is_ready();
    }
  };


  /// An associative, block-based data cache using LRU or FIF replacement policy and
  /// a write-through strategy with no write allocation.
  template<bool LRU_REPLACEMENT>
  class set_assoc_data_cache_t : public ideal_data_cache_t
  {
  private:
    /// Representation of the tag of a cache line.
    struct cache_tag_t
    {
      /// Flag indicating whether the block is valid.
      bool Is_valid;

      /// The full address of the tag.
      unsigned int Block_address;
    };

    /// Array of cache tags
    typedef cache_tag_t* cache_tags_t;

    /// The number of blocks in the cache.
    unsigned int Num_blocks;

    /// Number of bytes per block.
    unsigned int Num_block_bytes;

    /// Associativity of the cache
    unsigned int Associativity;

    /// The number of indexes.
    /// i.e., Num_blocks / Associativity.
    unsigned int Num_indexes;

    /// Flag indicating whether the cache is waiting for a pending request.
    bool Is_busy;

    /// Tag information of all the data cache's content.
    cache_tags_t *Content;

    /// Number of stall cycles caused by method cache misses.
    unsigned int Num_stall_cycles;
    
    /// Number of cache read hits
    unsigned int Num_read_hits;

    /// Number of cache read misses
    unsigned int Num_read_misses;

    /// Number of bytes read from the cache under a hit
    unsigned int Num_read_hit_bytes;

    /// Number of bytes read from the cache under a miss
    unsigned int Num_read_miss_bytes;

    /// Number of cache write hits
    unsigned int Num_write_hits;

    /// Number of cache write misses
    unsigned int Num_write_misses;

    /// Number of bytes written to the cache under a hit
    unsigned int Num_write_hit_bytes;

    /// Number of bytes written to the cache under a miss
    unsigned int Num_write_miss_bytes;

    /// Align an address to the block size.
    /// @param address The memory address to read from.
    /// @param size The number of bytes to read.
    unsigned int get_block_address(uword_t address, uword_t size);

  public:
    /// Construct a new data cache instance.
    /// @param memory The memory that is accessed through the cache.
    /// @param num_blocks The size of the cache in blocks.
    set_assoc_data_cache_t(memory_t &memory, unsigned int associativity, 
                           unsigned int num_blocks, 
                           unsigned int num_block_bytes);

    virtual ~set_assoc_data_cache_t();

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready();

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             bool short_stats);

    virtual void reset_stats();

    virtual void flush_cache();
  };
}

#endif // PATMOS_DATA_CACHE_H


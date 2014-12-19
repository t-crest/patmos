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
// Interface to method-cache implementations.
//

#ifndef PATMOS_METHOD_CACHE_H
#define PATMOS_METHOD_CACHE_H

#include "basic-types.h"
#include "instr-cache.h"
#include "simulation-core.h"
#include "symbol.h"

#include <map>
#include <limits>


namespace patmos
{
  /// An ideal method cache, i.e., all methods are always in the cache --
  /// magically.
  class ideal_method_cache_t : public instr_cache_t
  {
  private:
    /// The backing memory to fetch instructions from.
    memory_t &Memory;

    /// Keeping track of the most recenty loaded method
    uword_t current_base;

  public:
    /// Construct an ideal method cache that always hits.
    /// @param memory The memory to fetch instructions from.
    ideal_method_cache_t(memory_t &memory) : Memory(memory)
    {
    }

    /// Initialize the cache before executing the first instruction.
    /// @param address Address to fetch initial instructions.
    virtual void initialize(simulator_t &s, uword_t address);

    /// A simulated instruction fetch from the method cache.
    /// @param base The current method's base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[2]);

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(simulator_t &s, word_t address, word_t offset);

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(simulator_t &s, word_t address);

    virtual uword_t get_active_method_base();

    /// Notify the cache that a cycle passed.
    virtual void tick();

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os);

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options);
    
    virtual void reset_stats() {}
    
    virtual void flush_cache() {}
  };

  /// Cache statistics for a particular method and return offset. Map offsets
  /// to number of cache hits/misses.
  typedef std::map<word_t, std::pair<unsigned int, unsigned int> >
                                                                 offset_stats_t;

  // Cache statistics to keep track how often a given method was evicted by
  // other methods due to the limited cache capacity (first) or limited number
  // of tags (second).
  typedef std::map<word_t, std::pair<unsigned int, unsigned int> >
                                                               eviction_stats_t;

  /// Cache statistics of a particular method.
  class method_stats_info_t
  {
  public:
    /// Number of bytes transferred for this method.
    unsigned int Num_method_bytes;
    
    /// Number of blocks required for this method.
    unsigned int Num_blocks_allocated;
    
    /// Number of cache hits for the method.
    offset_stats_t Accesses;

    /// Minimum utilization of the cache entry for this method in words.
    float Min_utilization;
    
    /// Maximum utilization of the cache entry for this method in words.
    float Max_utilization;

    /// Keep track how often this method was evicted by some other method.
    eviction_stats_t Evictions;
    
    /// Initialize the method statistics.
    method_stats_info_t() : Num_method_bytes(0), Num_blocks_allocated(0),
      Min_utilization(std::numeric_limits<float>::max()), 
      Max_utilization(0)
    {
    }
  };

  /// A direct-mapped method cache using LRU replacement on methods.
  /// The cache is organized in blocks (Num_blocks) each of a fixed size
  /// (Num_block_bytes) in bytes.
  class lru_method_cache_t : public instr_cache_t
  {
  protected:
    /// Phases of fetching a method from memory.
    enum phase_e
    {
      /// The method cache is idle and available to handle requests.
      IDLE,
      /// The method cache is on the way of fetching the size of the method
      /// from memory.
      SIZE,
      /// The instructions of the method are being transferred from memory.
      TRANSFER
    };

    /// Bookkeeping information on methods in the cache.
    class method_info_t
    {
    public:
      /// The address of the method.
      uword_t Address;

      /// The number of blocks occupied by the method.
      uword_t Num_blocks;

      /// The size of the method in bytes.
      uword_t Num_bytes;

      std::vector<bool> Utilization;
      
      /// Construct a method lru info object. All data is initialized to zero.
      /// @param instructions Pointer to the method's instructions.
      method_info_t() 
      : Address(0), Num_blocks(0), Num_bytes(0)
      {
      }

      /// Update the internal data of the method lru info entry.
      /// @param address The new address of the entry.
      /// @param num_blocks The number of blocks occupied in the method cache.
      /// @param num_bytes The number of valid instruction bytes of the method.
      void update(uword_t address, uword_t num_blocks,
                  uword_t num_bytes);
      
      void reset_utilization();
      
      unsigned int get_utilized_bytes();
    };

    /// Map addresses to cache statistics of individual methods.
    typedef std::map<word_t, method_stats_info_t> method_stats_t;

    /// The backing memory to fetch instructions from.
    memory_t &Memory;

    /// Number of blocks in the method cache.
    unsigned int Num_blocks;

    /// Number of bytes in a block.
    unsigned int Num_block_bytes;

    /// Maximum number of active functions allowed in the cache.
    unsigned int Num_max_methods;
    
    /// Currently active phase to fetch a method from memory.
    phase_e Phase;

    /// Number of blocks of the currently pending transfer, if any.
    uword_t Num_allocate_blocks;

    /// Number of bytes of the currently pending transfer, if any.
    uword_t Num_method_size;

    /// The methods in the cache sorted by age.
    method_info_t *Methods;

    /// Cache buffer.
    byte_t *Cache;

    /// The number of methods currently in the cache.
    unsigned int Num_active_methods;

    /// The sum of sizes of all method entries currently active in the cache.
    unsigned int Num_active_blocks;

    /// Number of blocks transferred from the main memory.
    unsigned int Num_blocks_allocated;

    /// Largest number of blocks transferred from the main memory for a single
    /// method.
    unsigned int Num_max_blocks_allocated;

    /// Number of bytes transferred from the main memory.
    unsigned int Num_bytes_transferred;

    /// Largest number of bytes transferred from the main memory for a single
    /// method.
    unsigned int Num_max_bytes_transferred;

    /// Number of bytes fetched from the cache.
    unsigned int Num_bytes_fetched;
    
    /// Maximum number of methods allocated in the cache.
    unsigned int Num_max_active_methods;
    
    /// Number of cache hits.
    unsigned int Num_hits;

    /// Number of cache misses.
    unsigned int Num_misses;

    /// Number of cache misses on returns.
    unsigned int Num_misses_ret;

    /// Number of cache evictions due to limited cache capacity.
    unsigned int Num_evictions_capacity;

    /// Number of cache evictions due to the limited number of tags.
    unsigned int Num_evictions_tag;

    /// Number of stall cycles caused by method cache misses.
    unsigned int Num_stall_cycles;

    /// Number of bytes used in evicted methods.
    unsigned int Num_bytes_utilized;
    
    /// Cache statistics of individual method.
    method_stats_t Method_stats;

    /// A simulated instruction fetch from the method cache.
    /// @param current_method The currently active method.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    bool do_fetch(simulator_t &s, method_info_t &current_method, uword_t address, word_t iw[2]);

    /// Check whether the method at the given address is in the method cache.
    /// @param address The method address.
    /// @return True in case the method is in the cache, false otherwise.
    virtual bool lookup(simulator_t &s, uword_t address);

    void update_utilization_stats(method_info_t &method, uword_t utilized_bytes);

    enum eviction_type_e { EVICT_CAPACITY, EVICT_TAG, EVICT_FLUSH };
    
    /// Evict a given method, updating the cache state, and various statics.
    /// Also updates the utilization stats.
    /// @param method The method to be evicted.
    /// @param new_method the address of the new method causing the eviction
    /// @param capacity_miss true if the method is evicted due to a capacity miss
    void update_evict_stats(method_info_t &method, uword_t new_method, 
                                 eviction_type_e type);

    bool read_function_size(simulator_t &s, word_t function_base, uword_t *result_size);

    bool peek_function_size(simulator_t &s, word_t function_base, uword_t *result_size);

    uword_t get_num_blocks_for_bytes(uword_t num_bytes);

    uword_t get_transfer_start(uword_t address);
    
    uword_t get_transfer_size();
    
  public:
    /// Construct an LRU-based method cache.
    /// @param memory The memory to fetch instructions from on a cache miss.
    /// @param num_blocks The size of the cache in blocks.
    /// @param num_block_bytes The size of a single block in bytes
    /// @param max_active_methods The max number of active methods
    lru_method_cache_t(memory_t &memory, unsigned int num_blocks, 
                       unsigned int num_block_bytes, 
                       unsigned int max_active_methods = 0);

    /// Initialize the cache before executing the first instruction.
    /// @param address Address to fetch initial instructions.
    virtual void initialize(simulator_t &s, uword_t address);

    /// A simulated instruction fetch from the method cache.
    /// @param base The current method's base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[2]);

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(simulator_t &s, word_t address, word_t offset);

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(simulator_t &s, word_t address);

    virtual uword_t get_active_method_base();

    /// Notify the cache that a cycle passed -- i.e., if there is an ongoing
    /// transfer of a method to the cache, advance this transfer by one cycle.
    virtual void tick();

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os);

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options);

    virtual void reset_stats();
    
    virtual void flush_cache();
    
    /// free dynamically allocated cache memory.
    virtual ~lru_method_cache_t();
  };

  /// A direct-mapped method cache using FIFO replacement on methods.
  /// \see lru_method_cache_t
  class fifo_method_cache_t : public lru_method_cache_t
  {
  private:
    typedef lru_method_cache_t base_t;

    size_t active_method;

    /// Check whether the method at the given address is in the method cache.
    /// @param address The method address.
    /// @return True in case the method is in the cache, false otherwise.
    virtual bool lookup(simulator_t &s, uword_t address);

  public:

    /// Construct an FIFO-based method cache.
    /// @param memory The memory to fetch instructions from on a cache miss.
    /// @param num_blocks The size of the cache in blocks.
    /// @param num_block_bytes The size of a single block in bytes
    /// @param max_active_methods The max number of active methods
    fifo_method_cache_t(memory_t &memory, unsigned int num_blocks, 
                        unsigned int num_block_bytes,
                        unsigned int max_active_methods = 0) :
        lru_method_cache_t(memory, num_blocks, num_block_bytes, 
                           max_active_methods)
    {
	active_method = base_t::Num_blocks - 1;
    }

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @param offset Offset within the method where execution should continue.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(simulator_t &s, word_t address, word_t offset);

    virtual uword_t get_active_method_base();

    /// A simulated instruction fetch from the method cache.
    /// @param base The current method's base address.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[2]);

    virtual void flush_cache();
    
  };
}

#endif // PATMOS_METHOD_CACHE_H

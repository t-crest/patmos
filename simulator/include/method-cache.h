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
    virtual void initialize(uword_t address);

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2]);

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address);

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address);

    virtual uword_t get_active_method_base();

    /// Notify the cache that a cycle passed.
    virtual void tick();

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os);

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_stats(const simulator_t &s, std::ostream &os);
    
    virtual void reset_stats() {}
    
    virtual void flush_cache() {}
  };

  /// Cache statistics of a particular method.
  class method_stats_info_t
  {
  public:
    /// Number of bytes transferred for this method.
    unsigned int Num_method_bytes;
    
    /// Number of blocks required for this method.
    unsigned int Num_blocks_allocated;
    
    /// Number of cache hits for the method.
    unsigned int Num_hits;

    /// Number of cache misses for the method.
    unsigned int Num_misses;

    /// Minimum utilization of the cache entry for this method in words.
    float Min_utilization;
    
    /// Maximum utilization of the cache entry for this method in words.
    float Max_utilization;
    
    /// Initialize the method statistics.
    method_stats_info_t() : Num_method_bytes(0), Num_blocks_allocated(0),
      Num_hits(0), Num_misses(0),
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
      /// Pointer to the instructions of the method.
      byte_t *Instructions;

      /// The address of the method.
      uword_t Address;

      /// The number of blocks occupied by the method.
      uword_t Num_blocks;

      /// The size of the method in bytes.
      uword_t Num_bytes;

      std::vector<bool> Utilization;
      
      /// Construct a method lru info object. All data is initialized to zero.
      /// @param instructions Pointer to the method's instructions.
      method_info_t(byte_t *instructions = NULL): Instructions(instructions),
          Address(0), Num_blocks(0), Num_bytes(0)
      {
      }

      /// Update the internal data of the method lru info entry.
      /// @param instructions Pointer to the method's instructions.
      /// @param address The new address of the entry.
      /// @param num_blocks The number of blocks occupied in the method cache.
      /// @param num_bytes The number of valid instruction bytes of the method.
      void update(byte_t *instructions, uword_t address, uword_t num_blocks,
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

    /// Temporary transfer buffer
    byte_t *Transfer_buffer;

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

    /// Maximum number of methods allocated in the cache.
    unsigned int Num_max_active_methods;
    
    /// Number of cache hits.
    unsigned int Num_hits;

    /// Number of cache misses.
    unsigned int Num_misses;

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
    bool do_fetch(method_info_t &current_method, uword_t address, word_t iw[2]);

    /// Check whether the method at the given address is in the method cache.
    /// @param address The method address.
    /// @return True in case the method is in the cache, false otherwise.
    virtual bool lookup(uword_t address);

    void update_utilization_stats(method_info_t &method, uword_t utilized_bytes);
    
    void evict(method_info_t &method);

    bool read_function_size(word_t function_base, uword_t *result_size);
    
    bool peek_function_size(word_t function_base, uword_t *result_size);

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
    virtual void initialize(uword_t address);

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2]);

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address);

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address);

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
    virtual void print_stats(const simulator_t &s, std::ostream &os);

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
    virtual bool lookup(uword_t address);

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
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address);

    virtual uword_t get_active_method_base();

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2]);

    virtual void flush_cache();
    
  };
}

#endif // PATMOS_METHOD_CACHE_H

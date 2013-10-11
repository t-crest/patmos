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
#include "endian-conversion.h"
#include "instr-cache.h"
#include "simulation-core.h"
#include "symbol.h"

#include <cassert>
#include <cmath>
#include <map>
#include <ostream>
#include <limits>

#include <boost/format.hpp>
#include "exception.h"




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
    virtual void initialize(uword_t address)
    {
      current_base = address;
    }

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2])
    {
      Memory.read_peek(address, reinterpret_cast<byte_t*>(&iw[0]),
                       sizeof(word_t)*2);
      return true;
    }

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address)
    {
      current_base = address;
      return true;
    }

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address)
    {
      return true;
    }

    virtual uword_t get_active_method_base()
    {
      return current_base;
    }

    /// Notify the cache that a cycle passed.
    virtual void tick()
    {
      // do nothing here
    }

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os)
    {
      // nothing to do here either, since the cache has no internal state.
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_stats(const simulator_t &s, std::ostream &os)
    {
      // nothing to do here either, since the cache has no internal state.
    }
    
    virtual void reset_stats() {}
  };

  /// Cache statistics of a particular method.
  class method_stats_info_t
  {
  public:
    /// Number of bytes transferred for this method.
    unsigned int Num_bytes_transferred;
    
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
    method_stats_info_t() : Num_bytes_transferred(0), Num_blocks_allocated(0),
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
                  uword_t num_bytes)
      {
        Instructions = instructions;
        Address = address;
        Num_blocks = num_blocks;
        Num_bytes = num_bytes;
        reset_utilization();
      }
      
      void reset_utilization() {
        Utilization.clear();
        Utilization.resize(Num_bytes / sizeof(uword_t));        
      }
      
      unsigned int get_utilized_bytes() {
        uword_t utilized_bytes = 0;
        for (int i = 0; i < Utilization.size(); i++) {
          if (Utilization[i]) {
            utilized_bytes += sizeof(uword_t);
          }
        }
        return utilized_bytes;
      }
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
    uword_t Num_transfer_blocks;

    /// Number of bytes of the currently pending transfer, if any.
    uword_t Num_transfer_bytes;

    /// The methods in the cache sorted by age.
    method_info_t *Methods;

    /// The methods' instructions sorted by ID.
    byte_t *Instructions;

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
    bool do_fetch(method_info_t &current_method, uword_t address, word_t iw[2])
    {
      if(Phase != IDLE ||
         address < current_method.Address ||
         current_method.Address + current_method.Num_bytes <= address)
      {
        simulation_exception_t::illegal_pc(current_method.Address);
      }

      // get instruction word from the method's instructions
      byte_t *iwp = reinterpret_cast<byte_t*>(&iw[0]);
      for(unsigned int i = 0; i != sizeof(word_t)*NUM_SLOTS; i++, iwp++)
      {
        *iwp = current_method.Instructions[address + i -
                                           current_method.Address];
      }
      
      for (unsigned int i = 0; i < NUM_SLOTS; i++) {
        unsigned int word = (address-current_method.Address)/sizeof(word_t) + i;
        current_method.Utilization[word] = true;
      }
      return true;
    }

    /// Check whether the method at the given address is in the method cache.
    /// @param address The method address.
    /// @return True in case the method is in the cache, false otherwise.
    virtual bool lookup(uword_t address)
    {
      // check if the address is in the cache
      for(int i = Num_blocks - 1; i >= (int)(Num_blocks - Num_active_methods);
          i--)
      {
        if (Methods[i].Address == address)
        {
          // update the ordering of the methods to match LRU.

          // store the currently accessed entry
          method_info_t tmp = Methods[i];

          // shift all methods between the location of the currently accessed
          // entry and the previously most recently used entry.
          for(unsigned int j = i; j < Num_blocks - 1; j++)
          {
            Methods[j] = Methods[j + 1];
          }

          // reinsert the current entry at the head of the table
          Methods[Num_blocks - 1] = tmp;

          return true;
        }
      }

      // No entry matches the given address.
      return false;
    }

    void update_utilization_stats(method_info_t &method, uword_t utilized_bytes) 
    {
      
      float utilization = (float)utilized_bytes / (float)method.Num_bytes;
      
      Method_stats[method.Address].Max_utilization = std::max(utilization, 
                                  Method_stats[method.Address].Max_utilization);
      Method_stats[method.Address].Min_utilization = std::min(utilization, 
                                  Method_stats[method.Address].Min_utilization);      
    }
    
    void evict(method_info_t &method)
    {
      unsigned int utilized_bytes = method.get_utilized_bytes();
      
      Num_bytes_utilized += utilized_bytes;
      
      update_utilization_stats(method, utilized_bytes);
    }

    bool read_function_size(word_t function_base, uword_t *result_size)
    {
      uword_t num_bytes_big_endian;
      if (Memory.read(function_base - sizeof(uword_t),
            reinterpret_cast<byte_t*>(&num_bytes_big_endian),
            sizeof(uword_t)))
      {
        // convert method size to native endianess and compute size in
        // blocks
        *result_size = from_big_endian<big_uword_t>(num_bytes_big_endian);
        return true;
      }
      return false;
    }

    bool peek_function_size(word_t function_base, uword_t *result_size)
    {
      uword_t num_bytes_big_endian;
      Memory.read_peek(function_base - sizeof(uword_t),
          reinterpret_cast<byte_t*>(&num_bytes_big_endian),
          sizeof(uword_t));
      // convert method size to native endianess and compute size in
      // blocks
      *result_size = from_big_endian<big_uword_t>(num_bytes_big_endian);
      return true;
    }


    uword_t get_num_blocks_for_bytes(uword_t num_bytes)
    {
      return ((num_bytes - 1) / Num_block_bytes) + 1;
    }

    uword_t get_transfer_size()
    {
      // Memory controller aligns to burst size
      return Num_transfer_bytes;
    }
    
  public:
    /// Construct an LRU-based method cache.
    /// @param memory The memory to fetch instructions from on a cache miss.
    /// @param num_blocks The size of the cache in blocks.
    /// @param num_block_bytes The size of a single block in bytes
    /// @param max_active_methods The max number of active methods
    lru_method_cache_t(memory_t &memory, unsigned int num_blocks, 
                       unsigned int num_block_bytes, 
                       unsigned int max_active_methods = 0) :
        Memory(memory), Num_blocks(num_blocks), 
        Num_block_bytes(num_block_bytes), Phase(IDLE),
        Num_transfer_blocks(0), Num_transfer_bytes(0), Num_active_methods(0),
        Num_active_blocks(0), Num_blocks_allocated(0),
        Num_max_blocks_allocated(0), Num_bytes_transferred(0),
        Num_max_bytes_transferred(0), Num_max_active_methods(0),
        Num_hits(0), Num_misses(0), Num_stall_cycles(0), Num_bytes_utilized(0)
    {
      Num_max_methods = max_active_methods ? max_active_methods : num_blocks;
      Methods = new method_info_t[Num_blocks];
      for(unsigned int i = 0; i < Num_blocks; i++)
        Methods[i] = method_info_t(new byte_t[Num_block_bytes * Num_blocks]);
    }

    /// Initialize the cache before executing the first instruction.
    /// @param address Address to fetch initial instructions.
    virtual void initialize(uword_t address)
    {
      assert(Num_active_blocks == 0 && Num_active_methods == 0);

      // get 'most-recent' method of the cache
      method_info_t &current_method = Methods[Num_blocks - 1];

      // we assume it is an ordinary function entry with size specification
      // (the word before) and copy it in the cache.
      uword_t num_bytes, num_blocks;
      peek_function_size(address, &num_bytes);
      num_blocks = get_num_blocks_for_bytes(num_bytes);

      Memory.read_peek(address, current_method.Instructions,
          num_blocks * Num_block_bytes);
      current_method.update(current_method.Instructions, address,
          num_blocks, num_bytes);
      Num_active_blocks = num_blocks;

      Num_active_methods = 1;
      Num_max_active_methods = std::max(Num_max_active_methods, 1U);
    }

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2])
    {
      // fetch from 'most-recent' method of the cache
      return do_fetch(Methods[Num_blocks - 1], address, iw);
    }

    /// Assert that the method is in the method cache.
    /// If it is not available yet, initiate a transfer,
    /// evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool load_method(word_t address)
    {
      // check status of the method cache
      switch(Phase)
      {
        // a new request has to be started.
        case IDLE:
        {
          assert(Num_transfer_blocks == 0 && Num_transfer_bytes == 0);

          if (lookup(address))
          {
            // method is in the cache ... done!
            Num_hits++;
            Method_stats[address].Num_hits++;
            return true;
          }
          else
          {
            // proceed to next phase ... fetch the size from memory.
            // NOTE: the next phase starts immediately.
            Phase = SIZE;
            Num_misses++;
            Method_stats[address].Num_misses++;
          }
        }

        // the size of the method has to be fetched from memory.
        case SIZE:
        {
          assert(Num_transfer_blocks == 0 && Num_transfer_bytes == 0);

          // get the size of the method that should be loaded
          if (read_function_size(address, &Num_transfer_bytes))
          {
            Num_transfer_blocks = get_num_blocks_for_bytes(Num_transfer_bytes);

            // Note that this does not include alignment, this is done by the 
            // memory controller.
            Method_stats[address].Num_bytes_transferred = get_transfer_size();
            Method_stats[address].Num_blocks_allocated = Num_transfer_blocks;
            
            // check method size against cache size.
            if (Num_transfer_blocks == 0 || Num_transfer_blocks > Num_blocks)
            {
              simulation_exception_t::code_exceeded(address);
            }

            // throw other entries out of the cache if needed
            while (Num_active_blocks + Num_transfer_blocks > Num_blocks ||
                   Num_active_methods >= Num_max_methods)
            {
              assert(Num_active_methods > 0);
              Num_active_blocks -=
                            Methods[Num_blocks - Num_active_methods].Num_blocks;
              evict(Methods[Num_blocks - Num_active_methods]);
              Num_active_methods--;
            }

            // update counters
            Num_active_methods++;
            Num_max_active_methods = std::max(Num_max_active_methods,
                                              Num_active_methods);
            Num_active_blocks += Num_transfer_blocks;
            Num_blocks_allocated += Num_transfer_blocks;
            Num_max_blocks_allocated = std::max(Num_max_blocks_allocated,
                                                  Num_transfer_blocks);
            Num_bytes_transferred += Num_transfer_bytes;
            Num_max_bytes_transferred = std::max(Num_max_bytes_transferred,
                                                  Num_transfer_bytes);

            // shift the remaining blocks
            byte_t *saved_instructions =
                          Methods[Num_blocks - Num_active_methods].Instructions;
            for(unsigned int j = Num_blocks - Num_active_methods;
                j < Num_blocks - 1; j++)
            {
              Methods[j] = Methods[j + 1];
            }

            // insert the new entry at the head of the table
            Methods[Num_blocks - 1].update(saved_instructions, address,
                                           Num_transfer_blocks,
                                           Num_transfer_bytes);

            // proceed to next phase ... the size of the method has been fetched
            // from memory, now transfer the method's instructions.
            // NOTE: the next phase starts immediately.
            Phase = TRANSFER;
          }
          else
          {
            // keep waiting until the size has been loaded.
            return false;
          }
        }

        // begin transfer from main memory to the method cache.
        case TRANSFER:
        {
          assert(Num_transfer_blocks != 0 && Num_transfer_bytes != 0);

          if (Memory.read(address, Methods[Num_blocks - 1].Instructions,
                          get_transfer_size()))
          {
            // the transfer is done, go back to IDLE phase
            Num_transfer_blocks = Num_transfer_bytes = 0;
            Phase = IDLE;
            return true;
          }
          else
          {
            // keep waiting until the transfer is completed.
            return false;
          }
        }
      }

      assert(false);
      abort();
    }

    /// Check whether a method is in the method cache.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address)
    {
      // check if the address is in the cache
      for(int i = Num_blocks - 1; i >= (int)(Num_blocks - Num_active_methods);
          i--)
      {
        if (Methods[i].Address == address)
        {
          return true;
        }
      }

      return false;
    }

    virtual uword_t get_active_method_base()
    {
      return Methods[Num_blocks - 1].Address;
    }

    /// Notify the cache that a cycle passed -- i.e., if there is an ongoing
    /// transfer of a method to the cache, advance this transfer by one cycle.
    virtual void tick()
    {
      // update statistics
      if (Phase != IDLE)
        Num_stall_cycles++;
    }

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os)
    {
      os << boost::format(" #M: %1$02d #B: %2$02d\n")
         % Num_active_methods % Num_active_blocks;

      for(int i = Num_blocks - 1; i >= (int)(Num_blocks - Num_active_methods);
          i--)
      {
        os << boost::format("   M%1$02d: 0x%2$08x (%3$8d Blk %4$8d b)\n")
           % (Num_blocks - i) % Methods[i].Address % Methods[i].Num_blocks
           % Methods[i].Num_bytes;
      }

      os << '\n';
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_stats(const simulator_t &s, std::ostream &os)
    {
      uword_t bytes_utilized = Num_bytes_utilized;
      for(unsigned int j = Num_blocks - Num_active_methods; j < Num_blocks; j++)
      {
        bytes_utilized += Methods[j].get_utilized_bytes();
      }
      // Utilization = Bytes used / bytes allocated in cache
      float utilization = (float)bytes_utilized / 
                          (float)(Num_blocks_allocated * Num_block_bytes);
      // Fragmentation = Bytes loaded to cache / Bytes allocated in cache
      float fragmentation = 1.0 - (float)Num_bytes_transferred / 
                          (float)(Num_blocks_allocated * Num_block_bytes);
      
      // instruction statistics
      os << boost::format("                            total        max.\n"
                          "   Blocks Allocated    : %1$10d  %2$10d\n"
                          "   Bytes Transferred   : %3$10d  %4$10d\n"
                          "   Bytes Used          : %5$10d\n"
                          "   Utilization         : %6$10.2f%%\n"
                          "   Fragmentation       : %7$10.2f%%\n"
                          "   Max Methods in Cache: %8$10d\n"
                          "   Cache Hits          : %9$10d\n"
                          "   Cache Misses        : %10$10d\n"
                          "   Miss Stall Cycles   : %11$10d  %12$10.2f%%\n\n")
        % Num_blocks_allocated % Num_max_blocks_allocated
        % Num_bytes_transferred % Num_max_bytes_transferred
        % bytes_utilized % (utilization * 100.0) % (fragmentation * 100.0)
        % Num_max_active_methods % Num_hits % Num_misses % Num_stall_cycles 
        % (100.0 * Num_stall_cycles / (float)s.Cycle);

      // Update utilization stats for all methods not yet evicted.
      for(int i = Num_blocks - Num_active_methods; i < Num_blocks; i++)
      {
        // TODO we do not *actually* want to evict this method, use a diffent 
        //      method.
        evict(Methods[i]);
      }

      // print stats per method
      os << "       Method:      #hits     #misses       bytes      blocks    min-util    max-util\n";
      for(method_stats_t::iterator i(Method_stats.begin()),
          ie(Method_stats.end()); i != ie; i++)
      {
        os << boost::format("   0x%1$08x: %2$10d  %3$10d  %4$10d  %5$10d %6$10.2f%% %7$10.2f%%    %8%\n")
           % i->first % i->second.Num_hits % i->second.Num_misses
           % i->second.Num_bytes_transferred % i->second.Num_blocks_allocated
           % (i->second.Min_utilization * 100.0)
           % (i->second.Max_utilization * 100.0)
           % s.Symbols.find(i->first);
      }
    }

    virtual void reset_stats() 
    {
      Num_blocks_allocated = 0;
      Num_max_blocks_allocated = 0;
      Num_bytes_transferred = 0;
      Num_max_blocks_allocated = 0;
      Num_bytes_utilized = 0;
      Num_max_active_methods = 0;
      Num_hits = 0; 
      Num_misses = 0;
      Num_stall_cycles = 0;
      Method_stats.clear();
      for(unsigned int j = Num_blocks - Num_active_methods; j < Num_blocks; j++)
      {
        Methods[j].reset_utilization();
      }

    }
    
    /// free dynamically allocated cache memory.
    virtual ~lru_method_cache_t()
    {
      for(unsigned int i = 0; i < Num_blocks; i++)
        delete[] Methods[i].Instructions;

      delete [] Methods;
    }
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
    virtual bool lookup(uword_t address)
    {
      return base_t::is_available(address);
    }
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
    virtual bool load_method(word_t address)
    {
      // check if the address is in the cache
      bool avail = base_t::load_method(address);

      if (avail) {
	// update the active method pointer
	for(int i = base_t::Num_blocks - 1;
	    i >= (int)(base_t::Num_blocks - base_t::Num_active_methods); i--)
	{
	  if (base_t::Methods[i].Address == address)
	  {
	    active_method = i;
	  }
	}
      }

      return avail;
    }

    virtual uword_t get_active_method_base()
    {
      return base_t::Methods[active_method].Address;
    }

    /// A simulated instruction fetch from the method cache.
    /// @param address The memory address to fetch from.
    /// @param iw A pointer to store the fetched instruction word.
    /// @return True when the instruction word is available from the read port.
    virtual bool fetch(uword_t base, uword_t address, word_t iw[2])
    {
      // fetch from the currently active method
      return base_t::do_fetch(base_t::Methods[active_method], address, iw);
    }
  };
}

#endif // PATMOS_METHOD_CACHE_H

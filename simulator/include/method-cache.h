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

#include <cassert>
#include <ostream>

#include <boost/format.hpp>

namespace patmos
{
  /// Basic interface for method-caches implementations.
  class method_cache_t
  {
  private:
  public:
    /// Check whether a method is in the method cache, if it is not available 
    /// yet initiate a transfer, evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address) = 0;

    /// Notify the cache that a cycle passed.
    virtual void tick() = 0;

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) = 0;
  };

  /// An ideal method cache, i.e., all methods are always in the cache -- 
  /// magically.
  class ideal_method_cache_t : public method_cache_t
  {
  public:
    /// Check whether a method is in the method cache, if it is not available
    /// yet initiate a transfer, evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address)
    {
      return true;
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
  };
  
  /// A direct-mapped method cache using LRU replacement on methods.
  /// The cache is organized in blocks (NUM_BLOCKS) each of a fixed size
  /// (BLOCK_SIZE) in bytes.
  template<int BLOCK_SIZE, int NUM_BLOCKS, int TRANSFER_TIME>
  class lru_method_cache_t : public method_cache_t
  {
  private:
    /// Bookkeeping information on methods in the cache.
    struct method_info_t
    {
      /// The address of the method.
      word_t Address;

      /// The size of the method.
      uword_t Size;
    };
    
    /// The methods in the cache sorted by age.
    method_info_t Methods[NUM_BLOCKS];

    /// The number of methods currently in the cache.
    unsigned int Active_methods;
    
    /// The sum of sizes of all method entries currently active in the cache.
    unsigned int Active_blocks;

    /// Cycle counter indicating how many cycles it will still take to finish
    /// an ongoing transfer of a method to the cache.
    unsigned int Transfer_cycles;
  public:
    lru_method_cache_t() :
        Active_methods(1), Active_blocks(1), Transfer_cycles(0)
    {
      method_info_t initial = {0, 1};
      Methods[NUM_BLOCKS - 1] = initial;
    }

    /// Check whether a method is in the method cache, if it is not available
    /// yet initiate a transfer, evicting other methods if needed.
    /// @param address The base address of the method.
    /// @return True when the method is available in the cache, false otherwise.
    virtual bool is_available(word_t address)
    {
      // is a transfer currently pending?
      if (Transfer_cycles != 0)
      {
        return false;
      }
      
      // check if the address is in the cache
      for(unsigned int i = NUM_BLOCKS - 1; i >= NUM_BLOCKS - Active_methods;
          i--)
      {
        if (Methods[i].Address == address)
        {
          // update the ordering of the methods to match LRU.

          // store the currently accessed entry
          method_info_t tmp = Methods[i];

          // shift all methods between the location of the currently accessed 
          // entry and the previously most recently used entry.
          for(unsigned int j = i; j < NUM_BLOCKS - 1; j++)
          {
            Methods[j] = Methods[j + 1];
          }

          // reinsert the current entry at the head of the table
          Methods[NUM_BLOCKS - 1] = tmp;

          return true;
        }
      }

      // get the size of method to be loaded
      // TODO: actually get size from memory
      unsigned int size = 2;
      
      // the method was not found in the cache -- initiate a transfer.
      Transfer_cycles = size * TRANSFER_TIME;

      // update counters
      Active_methods++;
      Active_blocks += size;

      // throw other entries out of the cache if needed
      while (Active_blocks + size > NUM_BLOCKS)
      {
        assert(Active_methods > 0);
        Active_blocks -= Methods[NUM_BLOCKS - Active_methods].Size;
        Active_methods--;
      }

      // shift the remaining blocks
      for(unsigned int j = NUM_BLOCKS - Active_methods; j < NUM_BLOCKS - 1; j++)
      {
        Methods[j] = Methods[j + 1];
      }

      // insert the new entry at the head of the table
      method_info_t tmp = {address, size};
      Methods[NUM_BLOCKS - 1] = tmp;
      
      return false;
    }
    
    /// Notify the cache that a cycle passed -- i.e., if there is an ongoing 
    /// transfer of a method to the cache, advance this transfer by one cycle.
    virtual void tick()
    {
      // if a transfer is ongoing, advance it.
      if (Transfer_cycles != 0)
      {
        Transfer_cycles--;
      }
    }

    /// Print debug information to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os)
    {
      os << boost::format(" #M: %1$02x #B: %2$02d\n")
          % Active_methods % Active_blocks;

      for(unsigned int i = NUM_BLOCKS - 1; i >= NUM_BLOCKS - Active_methods;
          i--)
      {
        os << boost::format("   M%1$02d: %2$08x (%3$08x)\n")
           % i % Methods[i].Address % Methods[i].Size;
      }
    }
  };
}

#endif // PATMOS_METHOD_CACHE_H


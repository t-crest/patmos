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
// Basic definitions of interfaces to simulate the stack cache of Patmos.
//

#ifndef PATMOS_STACK_CACHE_H
#define PATMOS_STACK_CACHE_H

#include "memory.h"

#include <cmath>
#include <ostream>

#include <boost/format.hpp>

namespace patmos
{
  /// Base class for all stack cache implementations.
  class stack_cache_t : public memory_t
  {
  public:
    /// Reserve a given number of bytes to main memory, potentially spilling 
    /// stack data to main memory.
    /// @param size The number of bytes to be reserved to main memory.
    /// @return True when the stack space is actually reserved on the cache, 
    /// false otherwise.
    virtual bool reserve(uword_t size) = 0;

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size) = 0;

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @return True when the requested data is actually in the cache, false 
    /// otherwise.
    virtual bool ensure(uword_t size) = 0;
  };

  /// An ideal stack cache with "infinite" space.
  class ideal_stack_cache_t : public stack_cache_t
  {
  public:
    /// The content of the cache.
    std::vector<byte_t> Content;

  public:
    /// Reserve a given number of bytes to main memory, potentially spilling
    /// stack data to main memory.
    /// @param size The number of bytes to be reserved to main memory.
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size)
    {
      Content.resize(Content.size() + size);
      return true;
    }

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size)
    {
      Content.resize(Content.size() - size);
      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size)
    {
      return true;
    }
    
    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the cache.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      assert(Content.size() - address > 0 &&
            Content.size() - address + size < Content.size());
      
      // read the value
      for(unsigned int i = 0; i < size; i++)
      {
        *value++ = Content[Content.size() - address + i];
      }
      
      return true;
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the cache.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the cache, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      assert(Content.size() - address > 0 &&
             Content.size() - address + size < Content.size());

      // store the value
      for(unsigned int i = 0; i < size; i++)
      {
        Content[Content.size() - address + i] = *value++;
      }

      return true;
    }

    /// Notify the stack cache that a cycle has passed.
    virtual void tick()
    {
      // do nothing here
    }

    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      // do nothing here
    }
  };

  /// Possible transfers to/from the stack cache.
  enum Stack_cache_transfer_e
  {
    /// No transfer ongoing.
    IDLE,
    /// Data is transferred from the stack cache to the main memory.
    SPILL,
    /// Data is transferred from the main memory to the stack cache.
    FILL
  };

  /// A stack cache organized in blocks.
  /// The cache is organized in blocks (NUM_BLOCKS) each a fixed size in bytes
  /// (BLOCK_SIZE). Spills and fills are performed automatically during the 
  /// reserve and ensure operations operating on main memory (MAIN_BASE) up to
  /// a limit (NUM_BYTES).
  /// The DEBUG_INIT template argument can be used to test the stack cache, it
  /// causes all new stack entries to be initialized to the current stack depth 
  /// in bytes. It this allows to see if the stack content was updated 
  /// correctly during spill/fill.
  template<unsigned int BLOCK_SIZE, unsigned int NUM_BLOCKS,
           unsigned int NUM_MAIN_BLOCKS, int TRANSFER_TIME,
           bool DEBUG_INIT = false>
  class block_stack_cache_t : public stack_cache_t
  {
  private:
    /// The content of the stack cache.
    byte_t Content[BLOCK_SIZE * NUM_BLOCKS];

    /// The current stack point in main memory.
    byte_t *Main_base;
    
    /// The number of blocks currently spilled to main memory.
    unsigned int Spilled_blocks;

    /// The current top of the stack within the stack-cache in blocks.
    unsigned int Top_block;
    
    /// The number of blocks currently reserved.
    unsigned int Reserved_blocks;

    /// Store currently ongoing transfer.
    Stack_cache_transfer_e Transfer;
    
    /// Number of cycles left for the current transfer to be finished.
    unsigned int Transfer_Cycles;
  public:
    block_stack_cache_t(byte_t *main_base) :
        Main_base(main_base), Spilled_blocks(0), Top_block(0),
        Reserved_blocks(0), Transfer(IDLE), Transfer_Cycles(0)
    {
    }     
    
    /// Reserve a given number of bytes to main memory, potentially spilling
    /// stack data to main memory.
    /// @param size The number of bytes to be reserved to main memory.
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size)
    {
      // simulate ongoing transfers
      if (Transfer_Cycles != 0)
      {
        assert(Transfer == SPILL);
        return false;
      }
      
      assert(Transfer == IDLE);

      // get the number of blocks
      unsigned int size_blocks = std::ceil((float)size/(float)BLOCK_SIZE);
      assert(size_blocks <= NUM_BLOCKS);

      // spill some blocks
      if (Reserved_blocks + size_blocks > NUM_BLOCKS)
      {
        // ensure that we do not exceed the stack size limit
        assert(Spilled_blocks + size_blocks <= NUM_MAIN_BLOCKS);

        // get the number of blocks to transfer
        unsigned int transfer_blocks = Reserved_blocks + size_blocks -
                                         NUM_BLOCKS;

        // copy the data to main memory        
        for(unsigned int i = 0; i < transfer_blocks * BLOCK_SIZE; i++)
        {
          byte_t value = Content[(((Top_block + NUM_BLOCKS - Reserved_blocks) *
                                  BLOCK_SIZE) + i) % (BLOCK_SIZE * NUM_BLOCKS)];
          *(--Main_base) = value;
        }

        // track the new blocks in main memory
        Spilled_blocks += transfer_blocks;
        
        // free the blocks in the stack cache
        Reserved_blocks -= transfer_blocks;
        
        // setup a spill transfer
        Transfer = SPILL;
        Transfer_Cycles = TRANSFER_TIME * transfer_blocks;

        return false;
      }

      // allocate space on the stack cache
      Reserved_blocks += size_blocks;

      // push top-of-stack
      Top_block = (Top_block + size_blocks) % NUM_BLOCKS;

      // for testing purposes initialize the content of the cache.
      if (DEBUG_INIT)
      {
        unsigned int init_value = (Reserved_blocks + Spilled_blocks) *
                                  BLOCK_SIZE;
        for(unsigned int i = 0; i < size_blocks * BLOCK_SIZE; i++)
        {
          Content[((Top_block + NUM_BLOCKS) * BLOCK_SIZE - i - 1)
                    % (BLOCK_SIZE * NUM_BLOCKS)] = init_value--;
        }
      }
      
      return true;
    }

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size)
    {
      // we do not expect any transfers at this point
      assert(Transfer == IDLE && Transfer_Cycles == 0);
      
      // get the number of blocks
      unsigned int size_blocks = std::ceil((float)size/(float)BLOCK_SIZE);
      assert(size_blocks <= NUM_BLOCKS);

      if (size_blocks > Reserved_blocks)
      {
        // also free some blocks spilled to memory
        unsigned int main_blocks = (size_blocks - Reserved_blocks);
        assert(main_blocks <= Spilled_blocks);
        
        Spilled_blocks -= main_blocks;
        Main_base += (main_blocks * BLOCK_SIZE);

        // empty the stack cache
        Reserved_blocks = 0;
      }
      else
      {
        // free the blocks in the stack cache
        Reserved_blocks -= size_blocks;

        // pop top-of-stack
        Top_block = (Top_block + NUM_BLOCKS - size_blocks) % NUM_BLOCKS;
      }
      
      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size)
    {
      // simulate ongoing transfers
      if (Transfer_Cycles != 0)
      {
        assert(Transfer == FILL);
        return false;
      }

      assert(Transfer == IDLE);

      // get the number of blocks
      unsigned int size_blocks = std::ceil((float)size/(float)BLOCK_SIZE);
      assert(size_blocks <= NUM_BLOCKS);

      // need to transfer blocks from main memory?
      if (Reserved_blocks < size_blocks)
      {
        // get the number of blocks that have to be transferred
        unsigned int transfer_blocks = size_blocks - Reserved_blocks;

        assert(transfer_blocks <= Spilled_blocks);
        
        // copy the data from main memory
        for(unsigned int i = 0; i < transfer_blocks * BLOCK_SIZE; i++)
        {
          Content[(((Top_block + NUM_BLOCKS - Reserved_blocks) * BLOCK_SIZE) -
                    i - 1) % (BLOCK_SIZE * NUM_BLOCKS)] = *(Main_base++);
        }

        // remove the blocks from main memory
        Spilled_blocks -= transfer_blocks;

        // track the newly loaded blocks in the stack cache
        Reserved_blocks = size_blocks;
        
        // setup a fill transfer
        Transfer = FILL;
        Transfer_Cycles = TRANSFER_TIME * transfer_blocks;

        return false;
      }
      
      return true;
    }


    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the cache.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      unsigned int size_blocks = std::ceil((float)address/(float)BLOCK_SIZE);
      assert(size_blocks <= Reserved_blocks && address >= size);

      // read the value
      for(unsigned int i = 0; i < size; i++)
      {
        *value++ = Content[(((Top_block + NUM_BLOCKS) * BLOCK_SIZE) -
                              address + i) % (BLOCK_SIZE * NUM_BLOCKS)];
      }

      return true;
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the cache.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the cache, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      unsigned int size_blocks = std::ceil((float)address/(float)BLOCK_SIZE);
      assert(size_blocks <= Reserved_blocks && address >= size);

      // store the value
      for(unsigned int i = 0; i < size; i++)
      {
        Content[(((Top_block + NUM_BLOCKS) * BLOCK_SIZE) - address + i) %
                  (BLOCK_SIZE * NUM_BLOCKS)] = *value++;
      }

      return true;
    }

    /// Notify the stack cache that a cycle has passed.
    virtual void tick()
    {
      if (Transfer_Cycles != 0)
      {
        assert(Transfer != IDLE);
        Transfer_Cycles--;
        Transfer = (Transfer_Cycles == 0) ? IDLE : Transfer;
      }
    }

    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      os << boost::format("  T: %1$3d #B: %2$3d (%3%) #S: %4$3d (%5%) %6%(%7%)"
                          "\n")
         % Top_block % Reserved_blocks % NUM_BLOCKS % Spilled_blocks
         % NUM_MAIN_BLOCKS % Transfer % Transfer_Cycles;

      if (DEBUG_INIT)
      {
        os << "  Content:\n";

        // print stack content in main memory
        byte_t *content_main = Main_base + Spilled_blocks * BLOCK_SIZE;
        for(unsigned int i = 0; i < Spilled_blocks * BLOCK_SIZE; i++)
        {
          os << boost::format("    %1$3d\n") % (int)*--content_main;
        }

        os << "  ---------------------\n";

        // print stack cache content
        for(unsigned int i = 0; i < Reserved_blocks * BLOCK_SIZE; i++)
        {
          os << boost::format("    %1$2x\n")
             % (int)Content[(((Top_block + NUM_BLOCKS - Reserved_blocks) *
                              BLOCK_SIZE) + i) % (BLOCK_SIZE * NUM_BLOCKS)];
        }
      }
    }
  };  

  /// Operator to print the transfer state of a stack cache to a stream
  /// @param os The output stream to print to.
  /// @param transfer The transfer state of the stack cache.
  inline std::ostream &operator <<(std::ostream &os,
                                   Stack_cache_transfer_e transfer)
  {
    switch (transfer)
    {
      case IDLE:
        os << "idle";
        break;
      case SPILL:
        os << "spill";
        break;
      case FILL:
        os << "fill";
        break;
    }

    return os;
  }
}

#endif // PATMOS_STACK_CACHE_H


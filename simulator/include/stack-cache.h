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
#include "exception.h"
#include "simulation-core.h"

#include <cmath>
#include <ostream>

#include <boost/format.hpp>

namespace patmos
{
  /// Base class for all stack cache implementations.
  class stack_cache_t : public memory_t
  {
  private:
    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      // this is not supported by stack caches.
      assert(false);
      abort();
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      // this is not supported by stack caches.
      assert(false);
      abort();
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      // this is not supported by stack caches.
      assert(false);
      abort();
    }
  public:
    /// Reserve a given number of bytes, potentially spilling stack data to some
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_top) = 0;

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_top) = 0;

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_top) = 0;
  };

  /// An ideal stack cache with "infinite" space.
  class ideal_stack_cache_t : public stack_cache_t
  {
  protected:
    /// The content of the cache.
    std::vector<byte_t> Content;

  public:
    /// Reserve a given number of bytes, potentially spilling stack data to some 
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_top)
    {
      Content.resize(Content.size() + size);
      return true;
    }

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_top)
    {
      // check if stack size is exceeded
      if (Content.size() < size)
      {
        simulation_exception_t::stack_exceeded();
      }

      Content.resize(Content.size() - size);
      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_top)
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
      // if access exceeds the stack size
      if (Content.size() < address + size)
      {
        simulation_exception_t::stack_exceeded();
      }

      // read the value
      for(unsigned int i = 0; i < size; i++)
      {
        *value++ = Content[Content.size() - address - size + i];
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
      // if access exceeds the stack size
      if (Content.size() < address + size)
      {
        simulation_exception_t::stack_exceeded();
      }

      // store the value
      for(unsigned int i = 0; i < size; i++)
      {
        Content[Content.size() - address - size + i] = *value++;
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
      unsigned int idx = Content.size();

      for(std::vector<byte_t>::const_iterator i(Content.begin()),
          ie(Content.end()); i != ie; i++, idx--)
      {
        os << boost::format(" %1$08x:  %2$02x\n") % idx % (uword_t)(ubyte_t)*i;
      }

      os << "\n";
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // nothing to be done here
    }
  };

  /// A stack cache organized in blocks.
  /// The cache is organized in blocks (Num_blocks) each a fixed size in bytes
  /// (NUM_BLOCK_BYTES). Spills and fills are performed automatically during the
  /// reserve and ensure instructions, which operate on a bounded number of
  /// blocks in memory (Num_blocks_total).
  template<unsigned int NUM_BLOCK_BYTES = NUM_STACK_CACHE_BLOCK_BYTES>
  class block_stack_cache_t : public ideal_stack_cache_t
  {
  private:
    /// Possible transfers to/from the stack cache.
    enum phase_e
    {
      /// No transfer ongoing.
      IDLE,
      /// Data is transferred from the stack cache to the memory.
      SPILL,
      /// Data is transferred from the memory to the stack cache.
      FILL
    };

    /// Size of the stack cache in blocks.
    unsigned int Num_blocks;

    /// Total size of stack data allowed, including spilled data in main memory.
    unsigned int Num_blocks_total;

    /// Store currently ongoing transfer.
    phase_e Phase;

    /// The memory to spill/fill.
    memory_t &Memory;

    /// Temporary buffer used during spill/fill.
    byte_t *Buffer;

    /// Number of blocks to transfer to/from memory during a pending spill/fill.
    unsigned int Num_transfer_blocks;

    /// The number of blocks currently on the stack.
    unsigned int Num_reserved_blocks;

    /// The number of blocks currently spilled to memory.
    unsigned int Num_spilled_blocks;
  public:
    /// Construct a black-based stack cache.
    /// @param memory The memory to spill/fill.
    /// @param num_blocks Size of the stack cache in blocks.
    /// @param num_blocks_total Total size of stack data allowed, incl. spilled
    /// data.
    block_stack_cache_t(memory_t &memory, unsigned int num_blocks,
                        unsigned int num_blocks_total) :
        ideal_stack_cache_t(), Num_blocks(num_blocks),
        Num_blocks_total(num_blocks_total), Phase(IDLE), Memory(memory),
        Num_transfer_blocks(0), Num_reserved_blocks(0), Num_spilled_blocks(0)
    {
      Buffer = new byte_t[num_blocks * NUM_BLOCK_BYTES];
    }

    /// Reserve a given number of bytes, potentially spilling stack data to some 
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_top)
    {
      // convert byte-level size to block size.
      unsigned int size_blocks = std::ceil((float)size/(float)NUM_BLOCK_BYTES);

      switch (Phase)
      {
        case IDLE:
        {
          assert(Num_transfer_blocks == 0);

          // unsure that the stack cache size is not exceeded
          if (size_blocks > Num_blocks)
          {
            simulation_exception_t::stack_exceeded();
          }

          // reserve stack space
          Num_reserved_blocks += size_blocks;
          bool result = ideal_stack_cache_t::reserve(
                                      size_blocks * NUM_BLOCK_BYTES, stack_top);
          assert(result);

          // need to spill some blocks?
          if(Num_reserved_blocks <= Num_blocks)
          {
            // done.
            return true;
          }
          else
          {
            // yes? spill some blocks ...
            Num_transfer_blocks = Num_reserved_blocks - Num_blocks;

            // ensure that we do not exceed the total stack size limit
            if(Num_transfer_blocks + Num_spilled_blocks > Num_blocks_total)
            {
              simulation_exception_t::stack_exceeded();
            }

            // copy data to a buffer to allow contiguous transfer to the memory.
            unsigned int idx = Content.size() -
                               Num_reserved_blocks * NUM_BLOCK_BYTES;
            for(unsigned int i = 0; i < Num_transfer_blocks * NUM_BLOCK_BYTES;
                i++, idx++)
            {
              Buffer[i] = Content[idx];
            }

            // proceed to spill phase ...
            // NOTE: the spill commences immediately
            Phase = SPILL;
          }
        }
        case SPILL:
        {
          assert(Num_transfer_blocks != 0);

          // spill the content of the stack buffer to the memory.
          if (Memory.write(stack_top, &Buffer[0],
                           Num_transfer_blocks * NUM_BLOCK_BYTES))
          {
            // update the internal stack cache state.
            Num_reserved_blocks -= Num_transfer_blocks;
            Num_spilled_blocks += Num_transfer_blocks;

            // update the stack top pointer of the processor 
            stack_top -= Num_transfer_blocks * NUM_BLOCK_BYTES;

            // the transfer is done, go back to IDLE phase
            Num_transfer_blocks = 0;
            Phase = IDLE;
            return true;
          }
          else
          {
            // keep waiting until the transfer is completed.
            return false;
          }

          // should never be reached
          break;
        }
        case FILL:
          // should never be reached
          break;
      };

      // we should not get here.
      assert(false);
      abort();
    }

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_top)
    {
      // we do not expect any transfers at this point
      assert(Phase == IDLE && Num_transfer_blocks == 0);

      // convert byte-level size to block size.
      unsigned int size_blocks = std::ceil((float)size/(float)NUM_BLOCK_BYTES);

      // ensure that the stack cache size is not exceeded
      if(size_blocks > Num_blocks)
      {
        simulation_exception_t::stack_exceeded();
      }
      // ensure that the total stack cache size is not exceeded
      else if(size_blocks > Num_spilled_blocks + Num_reserved_blocks)
      {
        simulation_exception_t::stack_exceeded();
      }

      // free space on the stack
      bool result = ideal_stack_cache_t::free(size_blocks * NUM_BLOCK_BYTES,
                                              stack_top);
      assert(result);

      // also free space in memory?
      if (size_blocks <= Num_reserved_blocks)
      {
        // no? -- update internal state of the stack cache
        Num_reserved_blocks -= size_blocks;
      }
      else
      {
        // yes? -- also free some blocks in main memory
        unsigned int freed_spilled_blocks = size_blocks - Num_reserved_blocks;

        // update internal state of the stack cache
        Num_spilled_blocks -= freed_spilled_blocks;
        Num_reserved_blocks = 0;

        // update the stack top pointer of the processor
        stack_top += freed_spilled_blocks * NUM_BLOCK_BYTES;
      }

      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_top)
    {
      // convert byte-level size to block size.
      unsigned int size_blocks = std::ceil((float)size/(float)NUM_BLOCK_BYTES);

      switch(Phase)
      {
        case IDLE:
        {
          assert(Num_transfer_blocks == 0);

          // unsure that the stack cache size is not exceeded
          if (size_blocks > Num_blocks)
          {
            simulation_exception_t::stack_exceeded();
          }
          // ensure that the total stack cache size is not exceeded
          else if(size_blocks > Num_reserved_blocks + Num_spilled_blocks)
          {
            simulation_exception_t::stack_exceeded();
          }

          // need to transfer blocks from memory?
          if (Num_reserved_blocks >= size_blocks)
          {
            // no? -- done
            return true;
          }
          else
          {
            // yes? -- fill from memory
            Num_transfer_blocks = size_blocks - Num_reserved_blocks;

            assert(Num_transfer_blocks <= Num_spilled_blocks);

            // proceed to next phase -- fill from memory
            // NOTE: the fill commences immediately
            Phase = FILL;
          }
        }
        case FILL:
        {
          assert(Num_transfer_blocks != 0);

          // copy the data from memory into a temporary buffer
          if (Memory.read(stack_top - Num_transfer_blocks * NUM_BLOCK_BYTES,
                          Buffer, Num_transfer_blocks * NUM_BLOCK_BYTES))
          {
            // no need to copy from the temporary buffer into the stack cache,
            // since the data has never been erased there during the spill.

            // update the internal state of the stack cache
            Num_spilled_blocks -= Num_transfer_blocks;
            Num_reserved_blocks += Num_transfer_blocks;

            // terminate transfer -- goto IDLE state
            Phase = IDLE;
            Num_transfer_blocks = 0;
          }
          else
          {
            // wait until the transfer from the memory is completed.
            return false;
          }

          // should never be reached
          break;
        }
        case SPILL:
          // should never be reached
          break;
      }

      // we should not get here.
      assert(false);
      abort();
    }


    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      os << boost::format("  %1$5s: Reserved: %2$4d (%3%) "
                                    "Spilled: %4$4d (%5%)\n")
         % Phase % Num_reserved_blocks % Num_blocks % Num_spilled_blocks
         % Num_blocks_total;

      // print stack cache content
      ideal_stack_cache_t::print(os);
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // TODO: implement
    }

    /// free buffer memory.
    virtual ~block_stack_cache_t()
    {
      delete[] Buffer;
    }
  };


  /// Operator to print the state of a stack cache to a stream
  /// @param os The output stream to print to.
  /// @param phase The state of the stack cache.
  template<typename T>
  inline std::ostream &operator <<(std::ostream &os,
                                   typename T::phase_e phase)
  {
    switch (phase)
    {
      case T::IDLE:
        os << "idle";
        break;
      case T::SPILL:
        os << "spill";
        break;
      case T::FILL:
        os << "fill";
        break;
    }

    return os;
  }
}

#endif // PATMOS_STACK_CACHE_H


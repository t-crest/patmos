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
    virtual ~stack_cache_t() {}
    
    /// Reserve a given number of bytes, potentially spilling stack data to some
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Spill the given number of bytes from the stack.
    /// @param size The number of bytes that have to be spilled.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool spill(uword_t size, uword_t &stack_spill, uword_t &stack_top) = 0;

    /// Get the current size of the stack cache in bytes.
    virtual uword_t size() const = 0;
  };

  /// An ideal stack cache with "infinite" space.
  class ideal_stack_cache_t : public stack_cache_t
  {
  protected:
    /// The content of the cache.
    std::vector<byte_t> Content;

    /// The memory to spill/fill.
    memory_t &Memory;

  public:
    ideal_stack_cache_t(memory_t &memory) : Memory(memory) {}
    
    /// Reserve a given number of bytes, potentially spilling stack data to some 
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      Content.resize(Content.size() + size);
      stack_top -= size;
      return true;
    }

    /// Free a given number of bytes on the stack.
    /// @param size The number of bytes to be freed.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      // check if stack size is exceeded
      if (Content.size() < size)
      {
        simulation_exception_t::stack_exceeded();
      }

      Content.resize(Content.size() - size);
      stack_top += size;
      stack_spill = std::max(stack_spill, stack_top);
      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      // check if stack size is exceeded
      if (Content.size() < size)
      {
        Content.insert(Content.begin(), size - Content.size(), 0);
      }
      // load from memory, to support setting the spill pointer
      while (stack_spill < stack_top + size) {
        byte_t c;
        Memory.read_peek(stack_spill, &c, 1);
        Content[Content.size() - (++stack_spill - stack_top)] = c;
      }
      return true;
    }

    /// Spill the given number of bytes from the stack.
    /// @param size The number of bytes that have to be spilled.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool spill(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      // check if stack size is exceeded
      if (stack_top > stack_spill - size)
      {
        simulation_exception_t::stack_exceeded();
      }
      // write back to memory
      for (int i = 0; i < size; i++) {
        byte_t c = Content[Content.size() - (stack_spill - stack_top)];
        Memory.write_peek(--stack_spill, &c, 1);
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
      // if access exceeds the stack size
      if (Content.size() < address + size)
      {
        simulation_exception_t::stack_exceeded();
      }

      // read the value
      for(unsigned int i = 0; i < size; i++)
      {
        *value++ = Content[Content.size() - address - i - 1];
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
        Content[Content.size() - address - i - 1] = *value++;
      }

      return true;
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      // we do not simulate timing here anyway..
      read(address, value, size);
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
      assert(((idx % 4) == 0) && "Stack cache size (in bytes) needs to be multiple of 4");

      for(std::vector<byte_t>::const_iterator i(Content.begin()),
          ie(Content.end()); i != ie; i+=4, idx-=4)
      {
        os << boost::format(" %08x:  %02x%02x%02x%02x\n") % idx
                                                          % (uword_t)(ubyte_t)*(i+0)
                                                          % (uword_t)(ubyte_t)*(i+1)
                                                          % (uword_t)(ubyte_t)*(i+2)
                                                          % (uword_t)(ubyte_t)*(i+3);
      }

      os << "\n";
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // nothing to be done here
    }

    /// Get the current size of the stack cache in words.
    virtual uword_t size() const
    {
      return Content.size();
    }

  };

  /// A stack cache organized in blocks.
  /// The cache is organized in blocks (Num_blocks) each a fixed size in bytes
  /// (NUM_BLOCK_BYTES). Spills and fills are performed automatically during the
  /// reserve and ensure instructions.
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

    /// Store currently ongoing transfer.
    phase_e Phase;

    /// The memory to spill/fill.
    memory_t &Memory;

    /// Temporary buffer used during spill/fill.
    byte_t *Buffer;

    /// Number of blocks to transfer to/from memory during a pending spill/fill.
    unsigned int Num_transfer_blocks;

    // *************************************************************************
    // statistics

    /// Total number of blocks reserved.
    unsigned int Num_blocks_reserved_total;

    /// Maximal stack depth in blocks.
    unsigned int Max_blocks_allocated;

    /// Maximal number of blocks reserved at once.
    unsigned int Max_blocks_reserved;

    /// Total number of blocks transferred to main (spill) memory.
    unsigned int Num_blocks_spilled;

    /// Maximal number of blocks transferred to main at once (spill) memory.
    unsigned int Max_blocks_spilled;

    /// Total number of blocks transferred from main (fill) memory.
    unsigned int Num_blocks_filled;

    /// Maximal number of blocks transferred from main at once (fill) memory.
    unsigned int Max_blocks_filled;

    /// Number of executed free instructions resulting in an entirely empty
    /// stack cache.
    unsigned int Num_free_empty;

    /// Number of read accesses to the stack cache.
    unsigned int Num_read_accesses;

    /// Number of bytes read from the stack cache.
    unsigned int Num_bytes_read;

    /// Number of write accesses to the stack cache.
    unsigned int Num_write_accesses;

    /// Number of bytes written to the stack cache.
    unsigned int Num_bytes_written;

    /// Return the number of blocks currently reserved.
    inline unsigned int get_num_reserved_blocks() const
    {
      return Content.size() / NUM_BLOCK_BYTES;
    }
  public:
    /// Construct a black-based stack cache.
    /// @param memory The memory to spill/fill.
    /// @param num_blocks Size of the stack cache in blocks.
    block_stack_cache_t(memory_t &memory, unsigned int num_blocks) :
        ideal_stack_cache_t(memory), Num_blocks(num_blocks),
        Phase(IDLE), Memory(memory),
        Num_transfer_blocks(0),
        Num_blocks_reserved_total(0), Max_blocks_allocated(0),
        Max_blocks_reserved(0), Num_blocks_spilled(0), Max_blocks_spilled(0),
        Num_blocks_filled(0), Max_blocks_filled(0), Num_free_empty(0),
        Num_read_accesses(0), Num_bytes_read(0), Num_write_accesses(0),
        Num_bytes_written(0)
    {
      Buffer = new byte_t[num_blocks * NUM_BLOCK_BYTES];
    }

    /// Reserve a given number of bytes, potentially spilling stack data to some 
    /// memory.
    /// @param size The number of bytes to be reserved.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually reserved on the cache,
    /// false otherwise.
    virtual bool reserve(uword_t size, uword_t &stack_spill, uword_t &stack_top)
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
          bool result = ideal_stack_cache_t::reserve(
                                      size_blocks * NUM_BLOCK_BYTES, stack_spill, 
                                      stack_top);
          assert(result);

          // update statistics
          Num_blocks_reserved_total += size_blocks;
          Max_blocks_reserved = std::max(Max_blocks_reserved, size_blocks);
          Max_blocks_allocated = std::max(Max_blocks_allocated,
                                          (unsigned int)(
                                             Content.size() / NUM_BLOCK_BYTES));

          // need to spill some blocks?
          if(get_num_reserved_blocks() <= Num_blocks)
          {
            // done.
            return true;
          }
          else
          {
            // yes? spill some blocks ...
            Num_transfer_blocks = get_num_reserved_blocks() - Num_blocks;

            // copy data to a buffer to allow contiguous transfer to the memory.
            for(unsigned int i = 0; i < Num_transfer_blocks * NUM_BLOCK_BYTES;
                i++)
            {
              Buffer[Num_transfer_blocks * NUM_BLOCK_BYTES - i - 1] =
                                                                Content.front();
              Content.erase(Content.begin());
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
          if (Memory.write(stack_spill - Num_transfer_blocks * NUM_BLOCK_BYTES,
                           &Buffer[0], Num_transfer_blocks * NUM_BLOCK_BYTES))
          {
            // update statistics
            Num_blocks_spilled += Num_transfer_blocks;
            Max_blocks_spilled = std::max(Max_blocks_spilled,
                                          Num_transfer_blocks);

            // update the stack top pointer of the processor 
            stack_spill -= Num_transfer_blocks * NUM_BLOCK_BYTES;

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
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the stack space is actually freed in the cache, false
    /// otherwise.
    virtual bool free(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      // we do not expect any transfers at this point
      assert(Phase == IDLE && Num_transfer_blocks == 0);

      // convert byte-level size to block size.
      unsigned int size_blocks = std::ceil((float)size/(float)NUM_BLOCK_BYTES);
      unsigned int freed_spilled_blocks =
                                (size_blocks <= get_num_reserved_blocks()) ? 0 :
                                        size_blocks - get_num_reserved_blocks();

      // ensure that the stack cache size is not exceeded
      if(size_blocks > Num_blocks)
      {
        simulation_exception_t::stack_exceeded();
      }

      // also free space in memory?
      if (freed_spilled_blocks)
      {
        assert(Content.size() == 0);

        // update the stack top pointer of the processor
        stack_spill += freed_spilled_blocks * NUM_BLOCK_BYTES;

        // update statistics
        Num_free_empty++;
      }

      // free space on the stack (updates stack_top and stack_spill)
      bool result = ideal_stack_cache_t::free(size_blocks * NUM_BLOCK_BYTES,
                                              stack_spill, stack_top);
      assert(result);

      return true;
    }

    /// Ensure that a given number of bytes are actually on the stack.
    /// @param size The number of bytes that have to be available.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool ensure(uword_t size, uword_t &stack_spill, uword_t &stack_top)
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

          // need to transfer blocks from memory?
          if (get_num_reserved_blocks() >= size_blocks)
          {
            // no? -- done
            return true;
          }
          else
          {
            // yes? -- fill from memory
            Num_transfer_blocks = size_blocks - get_num_reserved_blocks();

            // proceed to next phase -- fill from memory
            // NOTE: the fill commences immediately
            Phase = FILL;
          }
        }
        case FILL:
        {
          assert(Num_transfer_blocks != 0);

          // copy the data from memory into a temporary buffer
          if (Memory.read(stack_spill, Buffer,
                          Num_transfer_blocks * NUM_BLOCK_BYTES))
          {
            // copy the data back into the stack cache
            for(unsigned int i = 0; i < Num_transfer_blocks * NUM_BLOCK_BYTES;
                i++)
            {
              Content.insert(Content.begin(), Buffer[i]);
            }

            // update statistics
            Num_blocks_filled += Num_transfer_blocks;
            Max_blocks_filled = std::max(Max_blocks_filled,
                                         Num_transfer_blocks);

            // update the stack top pointer of the processor 
            stack_spill += Num_transfer_blocks * NUM_BLOCK_BYTES;

            // terminate transfer -- goto IDLE state
            Phase = IDLE;
            Num_transfer_blocks = 0;
            return true;
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

    /// Spill the given number of bytes from the stack.
    /// @param size The number of bytes that have to be spilled.
    /// @param stack_spill Reference to the current value of the stack spill 
    /// pointer (might be updated).
    /// @param stack_top Reference to the current value of the stack top
    /// pointer (might be updated).
    /// @return True when the requested data is actually in the cache, false
    /// otherwise.
    virtual bool spill(uword_t size, uword_t &stack_spill, uword_t &stack_top)
    {
      // convert byte-level size to block size.
      unsigned int size_blocks = std::ceil((float)size/(float)NUM_BLOCK_BYTES);

      switch (Phase)
      {
        case IDLE:
        {
          assert(Num_transfer_blocks == 0);

          Num_transfer_blocks = size_blocks;

          // copy data to a buffer to allow contiguous transfer to the memory.
          for(unsigned int i = 0; i < Num_transfer_blocks * NUM_BLOCK_BYTES;
              i++)
          {
            Buffer[Num_transfer_blocks * NUM_BLOCK_BYTES - i - 1] =
                                                                Content.front();
            Content.erase(Content.begin());
          }

          // proceed to spill phase ...
          // NOTE: the spill commences immediately
          Phase = SPILL;
        }
        case SPILL:
        {
          assert(Num_transfer_blocks != 0);

          // spill the content of the stack buffer to the memory.
          if (Memory.write(stack_spill - Num_transfer_blocks * NUM_BLOCK_BYTES,
                           &Buffer[0], Num_transfer_blocks * NUM_BLOCK_BYTES))
          {
            // update statistics
            Num_blocks_spilled += Num_transfer_blocks;
            Max_blocks_spilled = std::max(Max_blocks_spilled,
                                          Num_transfer_blocks);

            // update the stack top pointer of the processor 
            stack_spill -= Num_transfer_blocks * NUM_BLOCK_BYTES;

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

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the cache.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      // read data
      bool result = ideal_stack_cache_t::read(address, value, size);
      assert(result);

      // update statistics
      Num_read_accesses++;
      Num_bytes_read += size;

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
      // read data
      bool result = ideal_stack_cache_t::write(address, value, size);
      assert(result);

      // update statistics
      Num_write_accesses++;
      Num_bytes_written += size;

      return true;
    }

    /// Print the internal state of the stack cache to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      os << boost::format("  %|1$5|: Reserved: %2$4d (%3%)\n")
         % Phase % get_num_reserved_blocks() % Num_blocks;

      // print stack cache content
      ideal_stack_cache_t::print(os);
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // stack cache statistics
      os << boost::format("\n\nStack Cache Statistics:\n"
                          "                           total        max.\n"
                          "   Blocks Spilled   : %1$10d  %2$10d\n"
                          "   Blocks Filled    : %3$10d  %4$10d\n"
                          "   Blocks Allocated : %5$10d  %6$10d\n"
                          "   Blocks Reserved  :          -  %7$10d\n"
                          "   Reads            : %8$10d\n"
                          "   Bytes Read       : %9$10d\n"
                          "   Writes           : %10$10d\n"
                          "   Bytes Written    : %11$10d\n"
                          "   Emptying Frees   : %12$10d\n\n")
        % Num_blocks_spilled % Max_blocks_spilled
        % Num_blocks_filled  % Max_blocks_filled
        % Num_blocks_reserved_total % Max_blocks_allocated % Max_blocks_reserved
        % Num_read_accesses % Num_bytes_read
        % Num_write_accesses % Num_bytes_written
        % Num_free_empty;
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


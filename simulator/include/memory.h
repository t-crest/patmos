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
// Basic definitions of interfaces to simulate the main memory of Patmos.
//

#ifndef PATMOS_MEMORY_H
#define PATMOS_MEMORY_H

#include "exception.h"
#include "simulation-core.h"

#include <boost/format.hpp>

#include <cassert>
#include <cmath>

namespace patmos
{
  /// Basic interface to access main memory during simulation.
  class memory_t
  {
  public:
    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a read port to read a fixed size.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @return True when the data is available from the read port.
    template<typename T>
    inline bool read_fixed(uword_t address, T &value)
    {
      return read(address, (byte_t*)&value, sizeof(T));
    }

    /// A simulated access to a write port a fixed size.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    template<typename T>
    bool write_fixed(uword_t address, T &value)
    {
      return write(address, (byte_t*)&value, sizeof(T));
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING, just read.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size) = 0;

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size) = 0;

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready() = 0;

    /// Notify the memory that a cycle has passed.
    virtual void tick() = 0;

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const = 0;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os) = 0;
  };

  /// An ideal memory.
  class ideal_memory_t : public memory_t
  {
  protected:
    /// The size of the memory in bytes.
    unsigned int Memory_size;

    /// Offset up to which the memory has been initialized.
    unsigned int Initialized_offset;

    /// The content of the memory.
    byte_t *Content;

    /// Ensure that the content is initialize up to the given address.
    /// @param address The address that should be accessed.
    /// @param size The access size.
    void check_initialize_content(uword_t address, uword_t size)
    {
      // check if the access exceeds the memory size
      if((address > Memory_size) || (size > Memory_size - address))
      {
        simulation_exception_t::unmapped(address);
      }

      // initialize memory content
      size = std::max(1024u, size);
      for(; Initialized_offset < std::min(address + size, Memory_size);
          Initialized_offset++)
      {
        Content[Initialized_offset] = 0;
      }
    }
  public:
    /// Construct a new memory instance.
    /// @param memory_size The size of the memory in bytes.
    ideal_memory_t(unsigned int memory_size) :
        Memory_size(memory_size), Initialized_offset(0)
    {
      Content = new byte_t[memory_size];
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      // check if the access exceeds the memory size and lazily initialize
      // memory content
      check_initialize_content(address, size);

      // read the data from the memory
      for(unsigned int i = 0; i < size; i++)
      {
        *value++ = Content[address++];
      }

      return true;
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      // check if the access exceeds the memory size and lazily initialize
      // memory content
      check_initialize_content(address, size);

      // write the data to the memory
      for(unsigned int i = 0; i < size; i++)
      {
        Content[address++] = *value++;
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
      bool result = read(address, value, size);
      assert(result);
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      bool result = write(address, value, size);
      assert(result);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      // always ready
      return true;
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      // do nothing here
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      // nothing to be done here
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // nothing to be done here
    }

    /// Destroy the memory instance and free its memory.
    virtual ~ideal_memory_t()
    {
      delete[] Content;
    }
  };

  /// A memory with fixed access times to transfer fixed-sized blocks.
  /// Memory accesses are performed in blocks (NUM_BLOCK_BYTES) with a fixed 
  /// access delay (Num_ticks_per_block).
  template<int NUM_BLOCK_BYTES=NUM_MEMORY_BLOCK_BYTES>
  class fixed_delay_memory_t : public ideal_memory_t
  {
  private:
    /// Memory access time per block in cycles.
    unsigned int Num_ticks_per_block;

    /// Number of ticks until the currently pending request is completed.
    uword_t Pending_ticks;

#ifndef NDEBUG
    /// Flag indicating whether the pending memory access is a load or a store.
    bool Pending_is_load;

    /// Address of pending memory access, if any.
    uword_t Pending_address;

    /// Number of bytes requested by pending memory access, if any.
    uword_t Pending_size;
#endif // NDEBUG
  public:
    /// Construct a new memory instance.
    /// @param memory_size The size of the memory in bytes.
    /// @param num_ticks_per_block Memory access time per block in cycles.
    fixed_delay_memory_t(unsigned int memory_size,
                         unsigned int num_ticks_per_block) :
        ideal_memory_t(memory_size), Num_ticks_per_block(num_ticks_per_block),
        Pending_ticks(0)
    {
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      if(Pending_ticks == 0)
      {
        // check if the access exceeds the memory size and lazily initialize
        // memory content
        check_initialize_content(address, size);

        // set up memory transfer
        Pending_ticks = Num_ticks_per_block * std::ceil((float)size /
                                                          NUM_BLOCK_BYTES);

#ifndef NDEBUG
        Pending_is_load = true;
        Pending_address = address;
        Pending_size = size;
#endif // NDEBUG
        return false;
      }

#ifndef NDEBUG
      assert(Pending_is_load &&
             Pending_address == address && Pending_size == size);
#endif // NDEBUG

      if(Pending_ticks == 1)
      {
        Pending_ticks = 0;
        return ideal_memory_t::read(address, value, size);
      }
      else
      {
        return false;
      }
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      if(Pending_ticks == 0)
      {
        // check if the access exceeds the memory size and lazily initialize
        // memory content
        check_initialize_content(address, size);

        // set up memory transfer
        Pending_ticks = Num_ticks_per_block * std::ceil((float)size /
                                                          NUM_BLOCK_BYTES);

#ifndef NDEBUG
        Pending_is_load = false;
        Pending_address = address;
        Pending_size = size;
#endif // NDEBUG
      }

#ifndef NDEBUG
      assert(!Pending_is_load &&
             Pending_address == address && Pending_size == size);
#endif // NDEBUG

      if(Pending_ticks == 1)
      {
        Pending_ticks = 0;
        return ideal_memory_t::write(address, value, size);
      }
      else
      {
        return false;
      }
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      bool result = ideal_memory_t::read(address, value, size);
      assert(result);
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      bool result = ideal_memory_t::write(address, value, size);
      assert(result);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      // always ready
      return Pending_ticks == 0;
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      if (Pending_ticks > 1)
      {
        Pending_ticks--;
      }
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      if (Pending_ticks == 0)
      {
        os << " IDLE\n";
      }
      else
      {
#ifdef NDEBUG
        os << boost::format(" Transfer: %1\n") % Pending_ticks;
#else  // NDEBUG
        os << boost::format(" %1%: %2% (%3$0x8 %4%)\n")
           % (Pending_is_load ? "LOAD" : "STORE") % Pending_ticks
           % Pending_address % Pending_size;
#endif // NDEBUG
      }
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // TODO:
    }
  };
}

#endif // PATMOS_MEMORY_H


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
    virtual ~memory_t() {}
    
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

    
    /// Read some values of a fixed size from the memory -- DO NOT SIMULATE TIMING, just read.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    template<typename T>
    inline void peek_fixed(uword_t address, T &value)
    {
      read_peek(address, (byte_t*)&value, sizeof(T));
    }

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

  /// Information about an outstanding memory request.
  struct request_info_t
  {
    /// The address of the request.
    uword_t Address;

    /// The size of the request.
    uword_t Size;

    /// Flag indicating whether the request is a load or store.
    bool Is_load;

    /// Number of ticks remaining until the request completes.
    unsigned int Num_ticks_remaining;
  };

  /// A memory with fixed access times to transfer fixed-sized blocks.
  /// Memory accesses are performed in blocks (NUM_BLOCK_BYTES) with a fixed 
  /// access delay (Num_ticks_per_block).
  class fixed_delay_memory_t : public ideal_memory_t
  {
  private:
    /// Set of pending requests processed by the memory.
    typedef std::vector<request_info_t> requests_t;

    /// Memory access time per block in cycles.
    unsigned int Num_ticks_per_block;

    /// Block transfer size
    unsigned int Num_bytes_per_block;
    
    /// Outstanding requests to the memory.
    requests_t Requests;

    /// Find or create a request given an address, size, and load/store flag.
    /// @param address The address of the request.
    /// @param size The number of bytes request by the access.
    /// @param is_load A flag indicating whether the access is a load or store.
    /// @return An existing or newly created request info object.
    /// \see request_info_t
    const request_info_t &find_or_create_request(uword_t address, uword_t size,
                                                 bool is_load)
    {
      // check if the access exceeds the memory size and lazily initialize
      // memory content
      check_initialize_content(address, size);

      // see if the request already exists
      for(requests_t::const_iterator i(Requests.begin()), ie(Requests.end());
          i != ie; i++)
      {
        // found matching request?
        if (i->Address == address && i->Size == size && i->Is_load == is_load)
          return *i;
      }

      uword_t start = (address/Num_bytes_per_block) * Num_bytes_per_block;
      uword_t end   = (((address + size - 1)/Num_bytes_per_block) + 1) *
                      Num_bytes_per_block;
      word_t aligned_size = end - start;
      
      // no matching request found, create a new one
      unsigned int num_blocks = (((aligned_size-1) / Num_bytes_per_block) + 1); 
      unsigned int num_ticks = Num_ticks_per_block * num_blocks;
      
      request_info_t tmp = {address, size, is_load, num_ticks};
      Requests.push_back(tmp);

      // return the newly created request
      return Requests.back();
    }
    
  public:
    /// Construct a new memory instance.
    /// @param memory_size The size of the memory in bytes.
    /// @param num_ticks_per_block Memory access time per block in cycles.
    /// @param num_bytes_per_block Memory block size.
    fixed_delay_memory_t(unsigned int memory_size,
                         unsigned int num_ticks_per_block, 
                         unsigned int num_bytes_per_block
                        ) :
        ideal_memory_t(memory_size), Num_ticks_per_block(num_ticks_per_block),
        Num_bytes_per_block(num_bytes_per_block)
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
      // get the request info
      const request_info_t &req(find_or_create_request(address, size, true));

      // check if the request has finished
      if(req.Num_ticks_remaining == 0)
      {
#ifndef NDEBUG
        // check request
        request_info_t &tmp(Requests.front());
        assert(tmp.Address == req.Address && tmp.Size == req.Size &&
               tmp.Is_load == req.Is_load);
#endif

        // clean-up the request
        Requests.erase(Requests.begin());

        // read the data
        return ideal_memory_t::read(address, value, size);
      }
      else
      {
        // not yet finished
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
      // get the request info
      const request_info_t &req(find_or_create_request(address, size, false));

      // check if the request has finished
      if(req.Num_ticks_remaining == 0)
      {
#ifndef NDEBUG
        // check request
        request_info_t &tmp(Requests.front());
        assert(tmp.Address == req.Address && tmp.Size == req.Size &&
               tmp.Is_load == req.Is_load);
#endif

        // clean-up the request
        Requests.erase(Requests.begin());

        // write the data
        return ideal_memory_t::write(address, value, size);
      }
      else
      {
        // not yet finished
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
      return Requests.empty();
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      if (!Requests.empty() && Requests.front().Num_ticks_remaining)
      {
        Requests.front().Num_ticks_remaining--;
      }
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      if (Requests.empty())
      {
        os << " IDLE\n";
      }
      else
      {
        for(requests_t::const_iterator i(Requests.begin()), ie(Requests.end());
            i != ie; i++)
        {
          os << boost::format(" %1%: %2% (0x%3$08x %4%)\n")
            % (i->Is_load ? "LOAD " : "STORE") % i->Num_ticks_remaining
            % i->Address % i->Size;
        }
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


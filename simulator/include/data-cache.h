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
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      Memory.read_peek(address, value, size);
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
      Memory.write_peek(address, value, size);
      return true;
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      Memory.read_peek(address, value, size);
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      Memory.write_peek(address, value, size);
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
    virtual void print_stats(std::ostream &os)
    {
      // nothing to be done here
    }
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
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      return Memory.read(address, value, size);
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      return Memory.write(address, value, size);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      return Memory.is_ready();
    }
  };


  /// An associative, block-based data cache using an LRU replacement policy and
  /// a write-through strategy with no write allocation.
  template<unsigned int ASSOCIATIVITY,
           unsigned int NUM_BLOCK_BYTES = NUM_DATA_CACHE_BLOCK_BYTES>
  class lru_data_cache_t : public ideal_data_cache_t
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

    /// Tags of the data cache.
    typedef cache_tag_t cache_tags_t[ASSOCIATIVITY];

    /// The number of blocks in the cache.
    unsigned int Num_blocks;

    /// The number of indexes.
    /// i.e., Num_blocks / ASSOCIATIVITY.
    unsigned int Num_indexes;

    /// Flag indicating whether the cache is waiting for a pending request.
    bool Is_busy;

    /// Tag information of all the data cache's content.
    cache_tags_t *Content;

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
    static unsigned int get_block_address(uword_t address, uword_t size)
    {
      // align to block addresses
      unsigned int block_address = (address / NUM_BLOCK_BYTES) *
                                      NUM_BLOCK_BYTES;
      assert(block_address == (((address + size - 1) / NUM_BLOCK_BYTES) *
                                NUM_BLOCK_BYTES));

      return block_address;
    }

  public:
    /// Construct a new data cache instance.
    /// @param memory The memory that is accessed through the cache.
    /// @param num_blocks The size of the cache in blocks.
    lru_data_cache_t(memory_t &memory, unsigned int num_blocks) :
        ideal_data_cache_t(memory), Num_blocks(num_blocks),
        Num_indexes(num_blocks / ASSOCIATIVITY), Is_busy(false),
        Num_read_hits(0), Num_read_misses(0), Num_read_hit_bytes(0),
        Num_read_miss_bytes(0), Num_write_hits(0), Num_write_misses(0),
        Num_write_hit_bytes(0), Num_write_miss_bytes(0)
    {
      assert(num_blocks % ASSOCIATIVITY == 0);
      Content = new cache_tags_t[Num_indexes];

      // initialize tags
      for(unsigned int i = 0; i < Num_indexes; i++)
      {
        for(unsigned int j = 0; j < ASSOCIATIVITY; j++)
        {
          Content[i][j].Is_valid = false;
        }
      }
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      // temporary buffer
      byte_t buf[NUM_BLOCK_BYTES];

      // get block address
      unsigned int block_address = get_block_address(address, size);

      // get tag information
      unsigned int entry_index = (block_address / NUM_BLOCK_BYTES)
                                 % Num_indexes;
      cache_tags_t &tags(Content[entry_index]);

      // check if content is in the cache
      unsigned int tag_index = ASSOCIATIVITY - 1;
      for(unsigned int i = 0; i < ASSOCIATIVITY; i++)
      {
        if (tags[i].Block_address == block_address && tags[i].Is_valid)
        {
          tag_index = i;
          break;
        }
      }

      // update cache state and read data
      if (tag_index != ASSOCIATIVITY - 1 || Memory.read(block_address, buf,
                                                        NUM_BLOCK_BYTES))
      {
        // update LRU ordering
        for(unsigned int i = tag_index; i != 0; i--)
        {
          tags[i] = tags[i -1];
        }

        // set tag information
        tags[0].Is_valid = 1;
        tags[0].Block_address = block_address;

        // actually read data from memory without stalling
        Memory.read_peek(address, value, size);

        // update statistics
        if (tag_index != ASSOCIATIVITY - 1)
        {
          Num_read_hits++;
          Num_read_hit_bytes += size;
        }
        else
        {
          Num_read_misses++;
          Num_read_miss_bytes += size;
        }

        Is_busy = false;
        return true;
      }

      Is_busy = true;
      return false;
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      // get block address
      unsigned int block_address = get_block_address(address, size);

      // read block data to simulate a block-based write
      byte_t buf[NUM_BLOCK_BYTES];
      Memory.read_peek(block_address, buf, NUM_BLOCK_BYTES);

      if (Memory.write(block_address, buf, NUM_BLOCK_BYTES))
      {
        // get tag information
        unsigned int entry_index = (block_address / NUM_BLOCK_BYTES)
                                   % Num_indexes;
        cache_tags_t &tags(Content[entry_index]);

        // check if content is in the cache
        unsigned int tag_index = ASSOCIATIVITY - 1;
        for(unsigned int i = 0; i < ASSOCIATIVITY; i++)
        {
          if (tags[i].Block_address == block_address && tags[i].Is_valid)
          {
            tag_index = i;
            break;
          }
        }

        // update the LRU ordering only when the data was in the cache
        if (tag_index != ASSOCIATIVITY - 1)
        {
          for(unsigned int i = tag_index; i != 0; i--)
          {
            tags[i] = tags[i -1];
          }

          // set tag information
          tags[0].Is_valid = 1;
          tags[0].Block_address = block_address;
        }

        // update statistics
        if (tag_index != ASSOCIATIVITY - 1)
        {
          Num_write_hits++;
          Num_write_hit_bytes += size;
        }
        else
        {
          Num_write_misses++;
          Num_write_miss_bytes += size;
        }
        // actually write the data
        Memory.write_peek(address, value, size);

        Is_busy = false;
        return true;
      }
      else {
        Is_busy = true;
        return false;
      }
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      return Is_busy;
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      for(unsigned int i = 0; i < Num_indexes; i++)
      {
        bool is_empty = true;
        for(unsigned int j = 0; j < ASSOCIATIVITY; j++)
        {
          if (Content[i][j].Is_valid)
          {
            if (is_empty)
              os << boost::format("%1$03d:") % i;

            os << boost::format("  %1$08x") % Content[i][j].Block_address;
            is_empty = false;
          }
        }

        if (!is_empty)
          os << "\n";
      }

      os << "\n";
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      // data cache statistics
      unsigned int total_reads = Num_read_hits + Num_read_misses;
      unsigned int read_miss_rate = total_reads == 0 ? 0 :
                                          (Num_read_misses * 100) / total_reads;
      unsigned int read_transfer_bytes = Num_read_misses * NUM_BLOCK_BYTES;
      unsigned int total_read_bytes = Num_read_hit_bytes + Num_read_miss_bytes;
      float        read_reuse = (float)total_read_bytes /
                                (float)read_transfer_bytes;

      unsigned int total_writes = Num_write_hits + Num_write_misses;
      unsigned int write_miss_rate = total_writes == 0 ? 0 :
                                        (Num_write_misses * 100) / total_writes;
      unsigned int write_transfer_bytes = total_writes * NUM_BLOCK_BYTES;
      unsigned int total_write_bytes = Num_write_hit_bytes + Num_write_miss_bytes;
      float        write_reuse = (float)total_write_bytes /
                                 (float)write_transfer_bytes;

      os << boost::format("\n\nData Cache Statistics:\n"
                          "                           total        hit      miss    miss-rate     reuse\n"
                          "   Reads            : %1$10d %2$10d %3$10d %4$10d%%\n"
                          "   Bytes Read       : %5$10d %6$10d %7$10d          - %8$10.2f\n"
                          "   Writes           : %9$10d %10$10d %11$10d %12$10d%%\n"
                          "   Bytes Written    : %13$10d %14$10d %15$10d          - %16$10.2f\n")
        % total_reads % Num_read_hits % Num_read_misses % read_miss_rate
        % total_read_bytes % Num_read_hit_bytes % Num_read_miss_bytes
        % read_reuse
        % total_writes % Num_write_hits % Num_write_misses % write_miss_rate
        % total_write_bytes % Num_write_hit_bytes % Num_write_miss_bytes
        % write_reuse;
    }

    /// free tag information.
    ~lru_data_cache_t()
    {
      delete[] Content;
    }
  };
}

#endif // PATMOS_DATA_CACHE_H


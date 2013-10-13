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
  public:
    virtual ~data_cache_t() {}
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
    virtual void print_stats(const simulator_t &s, std::ostream &os)
    {
      // nothing to be done here
    }
    
    virtual void reset_stats() {}
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


  /// An associative, block-based data cache using LRU or FIF replacement policy and
  /// a write-through strategy with no write allocation.
  template<bool LRU_REPLACEMENT>
  class set_assoc_data_cache_t : public ideal_data_cache_t
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

    /// Array of cache tags
    typedef cache_tag_t* cache_tags_t;

    /// The number of blocks in the cache.
    unsigned int Num_blocks;

    /// Number of bytes per block.
    unsigned int Num_block_bytes;

    /// Associativity of the cache
    unsigned int Associativity;

    /// The number of indexes.
    /// i.e., Num_blocks / Associativity.
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
    unsigned int get_block_address(uword_t address, uword_t size)
    {
      // align to block addresses
      unsigned int block_address = (address / Num_block_bytes) *
                                      Num_block_bytes;
      assert(block_address == (((address + size - 1) / Num_block_bytes) *
                                Num_block_bytes));

      return block_address;
    }

  public:
    /// Construct a new data cache instance.
    /// @param memory The memory that is accessed through the cache.
    /// @param num_blocks The size of the cache in blocks.
    set_assoc_data_cache_t(memory_t &memory, unsigned int associativity, unsigned int num_blocks,
                     unsigned int num_block_bytes) :
        ideal_data_cache_t(memory), Num_blocks(num_blocks),
        Num_block_bytes(num_block_bytes),
        Associativity(associativity),
        Num_indexes(num_blocks / Associativity), Is_busy(false),
        Num_read_hits(0), Num_read_misses(0), Num_read_hit_bytes(0),
        Num_read_miss_bytes(0), Num_write_hits(0), Num_write_misses(0),
        Num_write_hit_bytes(0), Num_write_miss_bytes(0)
    {
      assert(num_blocks % Associativity == 0);
      Content = new cache_tags_t[Num_indexes];

      // initialize tags
      for(unsigned int i = 0; i < Num_indexes; i++)
      {
        Content[i] = new cache_tag_t[Associativity];
        for(unsigned int j = 0; j < Associativity; j++)
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
      byte_t buf[Num_block_bytes];

      // get block address
      unsigned int block_address = get_block_address(address, size);

      // get tag information
      unsigned int entry_index = (block_address / Num_block_bytes)
                                 % Num_indexes;
      cache_tags_t &tags(Content[entry_index]);

      // tag_index corresponds to age of the cache block; we have
      // (tag_index == Associativity) iff tag is not in the cache
      unsigned int tag_index = Associativity;

      // check if content is in the cache
      for(unsigned int i = 0; i < Associativity; i++)
      {
        if (tags[i].Is_valid && tags[i].Block_address == block_address)
        {
          tag_index = i;
          break;
        }
      }

      bool cache_hit = (tag_index < Associativity);

      // update cache state and read data
      if (cache_hit || Memory.read(block_address, buf, Num_block_bytes))
      {
        // update LRU ordering
        unsigned int last_index_changed;
        if (cache_hit)
          last_index_changed = tag_index;
        else
          last_index_changed = Associativity-1;

        // no update on cache hit for FIFO
        if (LRU_REPLACEMENT || ! cache_hit)
        {
          for(unsigned int i = last_index_changed; i != 0; i--)
          {
            tags[i] = tags[i -1];
          }

          // set tag information
          tags[0].Is_valid = 1;
          tags[0].Block_address = block_address;
        }

        // actually read data from memory without stalling
        // TODO we should keep the data in the cache and read it from
        // there to detect consistency problems with multi-cores.
        Memory.read_peek(address, value, size);

        // update statistics
        if (cache_hit)
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
      byte_t buf[Num_block_bytes];
      Memory.read_peek(block_address, buf, Num_block_bytes);

      if (Memory.write(block_address, buf, Num_block_bytes))
      {
        // get tag information
        unsigned int entry_index = (block_address / Num_block_bytes)
                                   % Num_indexes;
        cache_tags_t &tags(Content[entry_index]);

        // check if content is in the cache
        unsigned int tag_index = Associativity;
        for(unsigned int i = 0; i < Associativity; i++)
        {
          if (tags[i].Is_valid && tags[i].Block_address == block_address)
          {
            tag_index = i;
            break;
          }
        }

        // No write allocate; contents of cache is updated on write-hit,
        // but as the simulator implementation does not store contents in
        // the cache, we simply omit cache updates for the write-through D$

        bool cache_hit = (tag_index < ASSOCIATIVITY);

        // update statistics
        if (cache_hit)
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
        for(unsigned int j = 0; j < Associativity; j++)
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
    virtual void print_stats(const simulator_t &s, std::ostream &os)
    {
      // data cache statistics
      unsigned int total_reads = Num_read_hits + Num_read_misses;
      unsigned int read_miss_rate = total_reads == 0 ? 0 :
                                          (Num_read_misses * 100) / total_reads;
      unsigned int read_transfer_bytes = Num_read_misses * Num_block_bytes;
      unsigned int total_read_bytes = Num_read_hit_bytes + Num_read_miss_bytes;
      float        read_reuse = (float)total_read_bytes /
                                (float)read_transfer_bytes;

      unsigned int total_writes = Num_write_hits + Num_write_misses;
      unsigned int write_miss_rate = total_writes == 0 ? 0 :
                                        (Num_write_misses * 100) / total_writes;
      unsigned int write_transfer_bytes = total_writes * Num_block_bytes;
      unsigned int total_write_bytes = Num_write_hit_bytes + Num_write_miss_bytes;
      float        write_reuse = (float)total_write_bytes /
                                 (float)write_transfer_bytes;

      os << boost::format("                           total        hit      miss    miss-rate     reuse\n"
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

    virtual void reset_stats()
    {
      Num_read_hits = 0;
      Num_read_misses = 0;
      Num_read_hit_bytes = 0;
      Num_read_miss_bytes = 0;
      Num_write_hits = 0;
      Num_write_misses = 0;
      Num_write_hit_bytes = 0;
      Num_write_miss_bytes = 0;
    }

    /// free tag information.
    ~set_assoc_data_cache_t()
    {
      delete[] Content;
    }
  };
}

#endif // PATMOS_DATA_CACHE_H


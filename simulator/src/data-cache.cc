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
// Implementation of data caches.
//
#include "data-cache.h"

#include "memory.h"
#include "exception.h"
#include "simulation-core.h"

#include <boost/format.hpp>

using namespace patmos;

template<bool LRU_REPLACEMENT>
unsigned int set_assoc_data_cache_t<LRU_REPLACEMENT>::
             get_block_address(uword_t address, uword_t size)
{
  // align to block addresses
  unsigned int block_address = (address / Num_block_bytes) *
				  Num_block_bytes;
  assert(block_address == (((address + size - 1) / Num_block_bytes) *
			    Num_block_bytes));

  return block_address;
}

template<bool LRU_REPLACEMENT>
set_assoc_data_cache_t<LRU_REPLACEMENT>::
set_assoc_data_cache_t(memory_t &memory, unsigned int associativity, 
                       unsigned int num_blocks,
                       unsigned int num_block_bytes) :
    ideal_data_cache_t(memory), Num_blocks(num_blocks),
    Num_block_bytes(num_block_bytes),
    Associativity(associativity),
    Num_indexes(num_blocks / Associativity), Is_busy(false),
    Num_stall_cycles(0),
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

template<bool LRU_REPLACEMENT>
set_assoc_data_cache_t<LRU_REPLACEMENT>::~set_assoc_data_cache_t()
{
  // free tag information.
  delete[] Content;
}

template<bool LRU_REPLACEMENT>
bool set_assoc_data_cache_t<LRU_REPLACEMENT>::
     read(uword_t address, byte_t *value, uword_t size)
{
  // temporary buffer
  byte_t buf[Num_block_bytes];

  // get block address
  unsigned int block_address = get_block_address(address, size);

  if (address - block_address + size > Num_block_bytes) {
    // Either size too big or not properly aligned
    simulation_exception_t::unaligned(address);
  }
  
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
  Num_stall_cycles++;
  return false;
}

template<bool LRU_REPLACEMENT>
bool set_assoc_data_cache_t<LRU_REPLACEMENT>::
     write(uword_t address, byte_t *value, uword_t size)
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

    bool cache_hit = (tag_index < Associativity);

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
    Num_stall_cycles++;
    return false;
  }
}

template<bool LRU_REPLACEMENT>
bool set_assoc_data_cache_t<LRU_REPLACEMENT>::is_ready()
{
  return !Is_busy;
}

template<bool LRU_REPLACEMENT>
void set_assoc_data_cache_t<LRU_REPLACEMENT>::
     print(std::ostream &os) const
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

template<bool LRU_REPLACEMENT>
void set_assoc_data_cache_t<LRU_REPLACEMENT>::
     print_stats(const simulator_t &s, std::ostream &os)
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
  
  // Ratio of bytes loaded from memory to bytes fetched from cache.
  // For every miss we load one full cache line.
  unsigned int bytes_transferred = Num_read_misses * Num_block_bytes + 
				   Num_write_misses * Num_block_bytes;
  float transfer_ratio = total_reads == 0 && total_writes == 0 ? 0 :
			(float)bytes_transferred / 
			(float)(Num_read_hit_bytes + Num_read_miss_bytes +
				Num_write_hit_bytes + Num_write_miss_bytes);
				      
			     
  os << boost::format("   Bytes Transferred   : %1$10d\n"
		      "   Transfer Ratio      : %2$10.3f\n"
		      "   Miss Stall Cycles   : %3$10d  %4$10.2f%%\n\n") 
    % bytes_transferred
    % transfer_ratio
    % Num_stall_cycles % (100.0 * (float)Num_stall_cycles/(float)s.Cycle);

    
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

template<bool LRU_REPLACEMENT>
void set_assoc_data_cache_t<LRU_REPLACEMENT>::reset_stats()
{
  Num_stall_cycles = 0;
  Num_read_hits = 0;
  Num_read_misses = 0;
  Num_read_hit_bytes = 0;
  Num_read_miss_bytes = 0;
  Num_write_hits = 0;
  Num_write_misses = 0;
  Num_write_hit_bytes = 0;
  Num_write_miss_bytes = 0;
}

template<bool LRU_REPLACEMENT>
void set_assoc_data_cache_t<LRU_REPLACEMENT>::flush_cache() 
{
  for(unsigned int i = 0; i < Num_indexes; i++) 
  {
    for(unsigned int j = 0; j < Associativity; j++)
    {
      Content[i][j].Is_valid = false;
    }
  }
}

// Explicit instantiation of template class for linking.
template class set_assoc_data_cache_t<false>;
template class set_assoc_data_cache_t<true>;

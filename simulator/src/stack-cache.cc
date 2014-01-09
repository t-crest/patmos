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
// Basic implementation of the stack cache of Patmos.
//
#include "stack-cache.h"

#include "memory.h"
#include "exception.h"
#include "simulation-core.h"

#include <cmath>
#include <ostream>

#include <boost/format.hpp>

using namespace patmos;

void stack_cache_t::write_peek(uword_t address, byte_t *value, uword_t size)
{
  assert(false);
  abort();
}

bool stack_cache_t::is_ready()
{
  assert(false);
  abort();
}


word_t ideal_stack_cache_t::prepare_reserve(uword_t size, 
                              uword_t &stack_spill, uword_t &stack_top)
{
  stack_top -= size;
  return 0;
}

word_t ideal_stack_cache_t::prepare_free(uword_t size, 
                            uword_t &stack_spill, uword_t &stack_top)
{
  stack_top += size;
  stack_spill = std::max(stack_spill, stack_top);
  return 0;
}

word_t ideal_stack_cache_t::prepare_ensure(uword_t size, 
                              uword_t &stack_spill, uword_t &stack_top)
{
  if (stack_spill < stack_top) {
    std::stringstream ss;
    ss << boost::format("Stack spill pointer %1$08x"
          " is below stack top pointer %2$08x") % stack_spill % stack_top;
    simulation_exception_t::stack_exceeded(ss.str());
  }

  // Being careful with unsigned here..
  word_t delta = stack_spill < stack_top + size ? stack_top + size - stack_spill
                                                : 0;
  stack_spill = std::max(stack_spill, stack_top + size);
  return delta;
}

word_t ideal_stack_cache_t::prepare_spill(uword_t size, 
                            uword_t &stack_spill, uword_t &stack_top)
{
  // check if stack size is exceeded
  if (stack_top > stack_spill - size)
  {
    std::stringstream ss;
    ss << boost::format("Spilling %1% bytes would move stack spill pointer"
            "%2$08x below stack top pointer %3$08x") 
          % size % stack_spill % stack_top;
    simulation_exception_t::stack_exceeded(ss.str());
  }

  stack_spill = stack_spill - size;
  return size;
}

bool ideal_stack_cache_t::reserve(uword_t size, word_t delta,
                                  uword_t new_spill, uword_t new_top)
{
  Content.resize(Content.size() + size);
  return true;
}

bool ideal_stack_cache_t::free(uword_t size, word_t delta,
                               uword_t new_spill, uword_t new_top)                               
{
  // check if stack size is exceeded
  if (Content.size() < size)
  {
    simulation_exception_t::stack_exceeded("Freeing more than the current "
                                            "size of the stack cache.");
  }

  Content.resize(Content.size() - size);
  return true;
}

bool ideal_stack_cache_t::ensure(uword_t size, word_t delta,
                                 uword_t new_spill, uword_t new_top)                                 
{
  // check if stack size is exceeded
  if (Content.size() < size)
  {
    Content.insert(Content.begin(), size - Content.size(), 0);
  }
  // fill back from memory
  for (int sp = new_spill - delta; sp < new_spill; sp++) {
    byte_t c;
    Memory.read_peek(sp, &c, 1);
    Content[Content.size() - (sp - new_top) - 1] = c;
  }
  return true;
}

bool ideal_stack_cache_t::spill(uword_t size, word_t delta,
                                uword_t new_spill, uword_t new_top)                                
{
  // write back to memory
  for (int i = 0; i < delta; i++) {
    byte_t c = Content[Content.size() - (new_spill - new_top) - i - 1];
    Memory.write_peek(new_spill + i, &c, 1);
  }
  return true;
}

bool ideal_stack_cache_t::read(uword_t address, byte_t *value, uword_t size)
{
  // if access exceeds the stack size
  if (Content.size() < address + size)
  {
    simulation_exception_t::stack_exceeded("Reading beyond the current size"
                                            " of the stack cache");
  }

  // read the value
  for(unsigned int i = 0; i < size; i++)
  {
    *value++ = Content[Content.size() - address - i - 1];
  }

  return true;
}

bool ideal_stack_cache_t::write(uword_t address, byte_t *value, uword_t size)
{
  // if access exceeds the stack size
  if (Content.size() < address + size)
  {
    simulation_exception_t::stack_exceeded("Writing beyond the current size"
                                            " of the stack cache");
  }

  // store the value
  for(unsigned int i = 0; i < size; i++)
  {
    Content[Content.size() - address - i - 1] = *value++;
  }

  return true;
}

void ideal_stack_cache_t::read_peek(uword_t address, byte_t *value, uword_t size)
{
  // we do not simulate timing here anyway..
  read(address, value, size);
}

void ideal_stack_cache_t::print(std::ostream &os) const
{
  unsigned int idx = 0;
  assert(((Content.size() % 4) == 0) && 
         "Stack cache size (in bytes) needs to be multiple of 4");

  for(std::vector<byte_t>::const_reverse_iterator i(Content.rbegin()),
      ie(Content.rend()); i != ie; i+=4, idx+=4)
  {
    os << boost::format(" %08x:  %02x%02x%02x%02x\n") % idx
                                                      % (uword_t)(ubyte_t)*(i+0)
                                                      % (uword_t)(ubyte_t)*(i+1)
                                                      % (uword_t)(ubyte_t)*(i+2)
                                                      % (uword_t)(ubyte_t)*(i+3);
  }

  os << "\n";
}

uword_t ideal_stack_cache_t::size() const
{
  return Content.size();
}



bool proxy_stack_cache_t::read(uword_t address, byte_t *value, uword_t size)
{
  return Memory.read(stack_top + address, value, size);
}

bool proxy_stack_cache_t::write(uword_t address, byte_t *value, uword_t size, uword_t &lazy_pointer)
{
  lazy_pointer = std::max(stack_top + address + size, lazy_pointer);
  return Memory.write(stack_top + address, value, size);
}

void proxy_stack_cache_t::read_peek(uword_t address, byte_t *value, uword_t size)
{
  return Memory.read_peek(stack_top + address, value, size);
}




block_stack_cache_t::block_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                    unsigned int num_block_bytes) :
    ideal_stack_cache_t(memory), Num_blocks(num_blocks),
    Num_block_bytes(num_block_bytes), Phase(IDLE), Memory(memory),
    Num_blocks_reserved(0), 
    Max_blocks_reserved(0), Num_blocks_spilled(0), Max_blocks_spilled(0),
    Num_blocks_filled(0), Max_blocks_filled(0), Num_free_empty(0),
    Num_read_accesses(0), Num_bytes_read(0), Num_write_accesses(0),
    Num_bytes_written(0), Num_stall_cycles(0)
{
  Buffer = new byte_t[num_blocks * Num_block_bytes];
}

block_stack_cache_t::~block_stack_cache_t()
{
  delete[] Buffer;
}

word_t block_stack_cache_t::prepare_reserve(uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top)
{
  // convert byte-level size to block size.
  unsigned int size_blocks = size ? (size - 1)/Num_block_bytes + 1 : 0;

  // ensure that the stack cache size is not exceeded
  if (size_blocks > Num_blocks)
  {
    simulation_exception_t::stack_exceeded("Reserving more blocks than"
      "the number of blocks in the stack cache");
  }
  if (size_blocks * Num_block_bytes != size) {
    simulation_exception_t::stack_exceeded("Reserving a frame size that is not "
      "a multiple of the stack block size.");
  }

  if (stack_top < size_blocks * Num_block_bytes) {
    simulation_exception_t::stack_exceeded("Stack top pointer decreased beyond "
                                           "lowest possible adddress.");
  }
  
  // update stack_top first
  stack_top -= size_blocks * Num_block_bytes;
  
  uword_t transfer_blocks = 0;
  
  uword_t reserved_blocks = get_num_reserved_blocks(stack_spill, stack_top);
  
  // need to spill some blocks?
  if (reserved_blocks > Num_blocks) {
    // yes? spill some blocks ...
    transfer_blocks = reserved_blocks - Num_blocks;
  }
  
  // update the stack top pointer of the processor 
  stack_spill -= transfer_blocks * Num_block_bytes;
  
  // update statistics
  Num_blocks_reserved += size_blocks;
  Max_blocks_reserved = std::max(Max_blocks_reserved, size_blocks);
  Num_blocks_spilled += transfer_blocks;
  Max_blocks_spilled = std::max(Max_blocks_spilled, transfer_blocks);

  return transfer_blocks * Num_block_bytes;
}

bool block_stack_cache_t::reserve(uword_t size, word_t delta,
                                  uword_t new_spill, uword_t new_top)                                  
{
  switch (Phase)
  {
    case IDLE:
    {
      // convert byte-level size to block size.
      unsigned int size_blocks = size ? (size - 1) / Num_block_bytes + 1 : 0;

      // reserve stack space
      Content.resize(Content.size() + size_blocks * Num_block_bytes);
      
      // need to spill some blocks?
      if(delta > 0) 
      {
        // copy data to a buffer to allow contiguous transfer to the memory.
        for(unsigned int i = 0; i < delta; i++)
        {
          Buffer[delta - i - 1] = Content.front();
          Content.erase(Content.begin());
        }

        // proceed to spill phase ...
        // NOTE: the spill commences immediately
        Phase = SPILL;
      }
      else {
        // Nothing to spill, we are done
        return true;
      }
    }
    case SPILL:
    {
      assert(delta);

      // spill the content of the stack buffer to the memory.
      if (Memory.write(new_spill, &Buffer[0], delta)) 
      {
        // the transfer is done, go back to IDLE phase
        Phase = IDLE;
        return true;
      }
      else {
        // keep waiting until the transfer is completed.
        Num_stall_cycles++;
        return false;
      }
    }
    case FILL:
      // should never be reached
      break;
  }
  // we should not get here.
  assert(false);
  abort();
}


word_t block_stack_cache_t::prepare_free(uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top)
{
  // convert byte-level size to block size.
  unsigned int size_blocks = size ? (size - 1)/Num_block_bytes + 1 : 0;
  unsigned int reserved_blocks = get_num_reserved_blocks(stack_spill, stack_top);
  
  unsigned int freed_spilled_blocks = (size_blocks <= reserved_blocks) ? 0 :
                                       size_blocks - reserved_blocks;

  // ensure that the stack cache size is not exceeded
  if(size_blocks > Num_blocks)
  {
    simulation_exception_t::stack_exceeded("Freeing more blocks than"
      " the number of blocks in the stack cache");
  }
  if (size_blocks * Num_block_bytes != size) {
    simulation_exception_t::stack_exceeded("Freeing a frame size that is not "
      "a multiple of the stack block size.");
  }

  // also free space in memory?
  if (freed_spilled_blocks)
  {
    // update the stack top pointer of the processor
    stack_spill += freed_spilled_blocks * Num_block_bytes;
  }
  
  stack_top += size_blocks * Num_block_bytes;
  
  // update statistics
  if (stack_top == stack_spill) {
    Num_free_empty++;
  }

  return 0;
}

bool block_stack_cache_t::free(uword_t size, word_t delta,
                               uword_t new_spill, uword_t new_top)
{
  // we do not expect any transfers at this point
  assert(Phase == IDLE);

    // convert byte-level size to block size.
  unsigned int size_blocks = size ? (size - 1) / Num_block_bytes + 1 : 0;
  
  if (Content.size() <= size_blocks * Num_block_bytes) {
    Content.clear();
  } else {
    Content.resize(Content.size() - size_blocks * Num_block_bytes);
  }

  return true;
}


word_t block_stack_cache_t::prepare_ensure(uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top)
{
  // convert byte-level size to block size.
  unsigned int size_blocks = size ? (size - 1)/Num_block_bytes + 1 : 0;

  // ensure that the stack cache size is not exceeded
  if (size_blocks > Num_blocks)
  {
    simulation_exception_t::stack_exceeded("Ensuring more blocks than"
        " the number of blocks in the stack cache");
  }
  if (size_blocks * Num_block_bytes != size) {
    simulation_exception_t::stack_exceeded("Ensuring a frame size that is not "
      "a multiple of the stack block size.");
  }


  uword_t transfer_blocks = 0;
  
  uword_t reserved_blocks = get_num_reserved_blocks(stack_spill, stack_top);
  
  // need to transfer blocks from memory?
  if (reserved_blocks < size_blocks) {
    transfer_blocks = size_blocks - reserved_blocks;
  }
  
  // update the stack top pointer of the processor 
  stack_spill += transfer_blocks * Num_block_bytes;

  // update statistics
  Num_blocks_filled += transfer_blocks;
  Max_blocks_filled = std::max(Max_blocks_filled, transfer_blocks);

  return transfer_blocks * Num_block_bytes;
}

bool block_stack_cache_t::ensure(uword_t size, word_t delta,
                                 uword_t new_spill, uword_t new_top)
{
  // do we need to fill?
  if (!delta) {
    // no, done.
    return true;
  }

  Phase = FILL;
  
  // copy the data from memory into a temporary buffer
  if (Memory.read(new_spill - delta, Buffer, delta))
  {
    // Ensure the size of the stack cache
    if (Content.size() < size)
    {
      Content.insert(Content.begin(), size - Content.size(), 0);
    }
    
    // get the offset of the old spill pointer in the content array
    uword_t old_size = new_spill - delta - new_top;
    
    // copy the data back into the stack cache
    for(unsigned int i = 0; i < delta; i++)
    {
      Content[Content.size() - old_size - i - 1]  = Buffer[i];
    }

    // terminate transfer -- goto IDLE state
    Phase = IDLE;
    return true;
  }
  else {
    // wait until the transfer from the memory is completed.
    Num_stall_cycles++;
    return false;
  }
}


word_t block_stack_cache_t::prepare_spill(uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top)
{
  // convert byte-level size to block size.
  unsigned int size_blocks = size ? (size - 1)/Num_block_bytes + 1 : 0;

  uword_t transfer_blocks = size_blocks;

  if (size_blocks * Num_block_bytes != size) {
    simulation_exception_t::stack_exceeded("Spilling a frame size that is not "
      "a multiple of the stack block size.");
  }

  
  // update the stack top pointer of the processor 
  stack_spill -= transfer_blocks * Num_block_bytes;

  // update statistics
  Num_blocks_spilled += transfer_blocks;
  Max_blocks_spilled = std::max(Max_blocks_spilled, transfer_blocks);

  return transfer_blocks * Num_block_bytes;
}

bool block_stack_cache_t::spill(uword_t size, word_t delta,
                                uword_t new_spill, uword_t new_top)
{
  switch (Phase)
  {
    case IDLE:
    {
      // copy data to a buffer to allow contiguous transfer to the memory.
      for(unsigned int i = 0; i < delta; i++)
      {
        Buffer[delta - i - 1] = Content.front();
        Content.erase(Content.begin());
      }

      // proceed to spill phase ...
      // NOTE: the spill commences immediately
      Phase = SPILL;
    }
    case SPILL:
    {
      assert(delta);

      // spill the content of the stack buffer to the memory.
      if (Memory.write(new_spill, &Buffer[0], delta))
      {
        // the transfer is done, go back to IDLE phase
        Phase = IDLE;
        return true;
      }
      else {
        // keep waiting until the transfer is completed.
        Num_stall_cycles++;
        return false;
      }
    }
    case FILL:
      // should never be reached
      break;
  };

  // we should not get here.
  assert(false);
  abort();
}


bool block_stack_cache_t::read(uword_t address, byte_t *value, uword_t size)
{
  // read data
  bool result = ideal_stack_cache_t::read(address, value, size);
  assert(result);

  // update statistics
  Num_read_accesses++;
  Num_bytes_read += size;

  return true;
}

bool block_stack_cache_t::write(uword_t address, byte_t *value, uword_t size)
{
  // read data
  bool result = ideal_stack_cache_t::write(address, value, size);
  assert(result);

  // update statistics
  Num_write_accesses++;
  Num_bytes_written += size;

  return true;
}

void block_stack_cache_t::print(std::ostream &os) const
{
  uword_t reserved_blocks = Content.size() / Num_block_bytes;
  
  os << boost::format("  %|1$5|: Reserved: %2$4d (%3%)\n")
      % Phase % reserved_blocks % Num_blocks;

  // print stack cache content
  ideal_stack_cache_t::print(os);
}

void block_stack_cache_t::print_stats(const simulator_t &s, std::ostream &os, 
                                      bool short_stats)
{
  unsigned int bytes_transferred = Num_blocks_filled * Num_block_bytes +
                                   Num_blocks_spilled * Num_block_bytes;
  float transfer_ratio = (float)bytes_transferred /
                         (float)(Num_bytes_read + Num_bytes_written);
  
  // stack cache statistics
  os << boost::format("                              total        max.\n"
                      "   Blocks Spilled      : %1$10d  %2$10d\n"
                      "   Blocks Filled       : %3$10d  %4$10d\n"
                      "   Blocks Reserved     : %5$10d  %6$10d\n"
                      "   Bytes Transferred   : %7$10d  %8$10d\n"
                      "   Reads               : %9$10d\n"
                      "   Bytes Read          : %10$10d\n"
                      "   Writes              : %11$10d\n"
                      "   Bytes Written       : %12$10d\n"
                      "   Emptying Frees      : %13$10d\n"
                      "   Transfer Ratio      : %14$10.3f\n"
                      "   Miss Stall Cycles   : %15$10d  %16$10.2f%%\n\n")
    % Num_blocks_spilled % Max_blocks_spilled
    % Num_blocks_filled  % Max_blocks_filled
    % Num_blocks_reserved % Max_blocks_reserved
    % bytes_transferred 
    % (std::max(Max_blocks_filled, Max_blocks_spilled) * Num_block_bytes)
    % Num_read_accesses % Num_bytes_read
    % Num_write_accesses % Num_bytes_written
    % Num_free_empty
    % transfer_ratio
    % Num_stall_cycles % (100.0 * (float)Num_stall_cycles/(float)s.Cycle);
}

void block_stack_cache_t::reset_stats() 
{
  Num_blocks_spilled = 0;
  Max_blocks_spilled = 0;
  Num_blocks_filled = 0;
  Max_blocks_filled = 0;
  Num_blocks_reserved = 0;
  Max_blocks_reserved = 0;
  Num_read_accesses = 0;
  Num_bytes_read = 0;
  Num_write_accesses = 0;
  Num_bytes_written = 0;
  Num_free_empty = 0;
  Num_stall_cycles = 0;
}

block_lazy_stack_cache_t::block_lazy_stack_cache_t(memory_t &memory, unsigned int num_blocks, 
                    unsigned int num_block_bytes) :
    block_stack_cache_t(memory, num_blocks, num_block_bytes), 
    Num_blocks(num_blocks),
    Num_block_bytes(num_block_bytes), Phase(IDLE), Memory(memory),
    Num_blocks_reserved(0), 
    Max_blocks_reserved(0), Num_blocks_spilled(0), Max_blocks_spilled(0),
    Num_blocks_filled(0), Max_blocks_filled(0), Num_free_empty(0),
    Num_read_accesses(0), Num_bytes_read(0), Num_write_accesses(0),
    Num_bytes_written(0), Num_stall_cycles(0)
{
 // Buffer = new byte_t[num_blocks * Num_block_bytes];
}

block_lazy_stack_cache_t::~block_lazy_stack_cache_t()
{
  
}

word_t block_lazy_stack_cache_t::prepare_reserve(uword_t size, 
                                   uword_t &stack_spill, uword_t &stack_top, uword_t &lazy_pointer, bool lp_pulldown)
{
    // convert byte-level size to block size.
  unsigned int size_blocks = (size - 1)/Num_block_bytes + 1;

  // ensure that the stack cache size is not exceeded
  if (size_blocks > Num_blocks)
  {
    simulation_exception_t::stack_exceeded("Reserving more blocks than"
      "the number of blocks in the stack cache");
  }

  if (stack_top < size_blocks * Num_block_bytes) {
    simulation_exception_t::stack_exceeded("Stack top pointer decreased beyond "
                                           "lowest possible adddress.");
  }
  
  lp_pulldown = stack_top == lazy_pointer;

  // update stack_top first
  stack_top -= size_blocks * Num_block_bytes;
  
  uword_t transfer_blocks = 0;
  
  uword_t reserved_blocks = get_num_reserved_blocks(lazy_pointer, stack_top);

  uword_t lazy_spilled = get_num_reserved_blocks(stack_spill, lazy_pointer);
  
  uword_t non_spilled_blocks = 0;
  
  // need to spill some blocks?
  if (reserved_blocks > Num_blocks) {
    // yes? spill some blocks ...
    transfer_blocks = reserved_blocks - Num_blocks;
  }
 
  uword_t non_transfer_blocks = transfer_blocks - lazy_spilled;

  if (lazy_spilled < transfer_blocks) {
    transfer_blocks = lazy_spilled;
  }

  
  // update the stack top pointer of the processor
  stack_spill -= transfer_blocks * Num_block_bytes;
  
  if (lp_pulldown) {
          // no need to spill uninitialized stack data
         lazy_pointer -= stack_top;
  }
  else if (lazy_pointer > stack_spill) {
          // no need to spill stack data that is already spilled
         lazy_pointer = stack_spill;
   }

  // update statistics
  Num_blocks_reserved += size_blocks;
  Max_blocks_reserved = std::max(Max_blocks_reserved, size_blocks);
  Num_blocks_spilled += transfer_blocks;
  Max_blocks_spilled = std::max(Max_blocks_spilled, transfer_blocks);

  return transfer_blocks * Num_block_bytes;
}

word_t block_lazy_stack_cache_t::prepare_free(uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top, uword_t &lazy_pointer)
{
  // convert byte-level size to block size.
  unsigned int size_blocks = (size - 1)/Num_block_bytes + 1;
  unsigned int reserved_blocks = get_num_reserved_blocks(stack_spill, stack_top);
  
  unsigned int freed_spilled_blocks = (size_blocks <= reserved_blocks) ? 0 :
                                       size_blocks - reserved_blocks;

  // ensure that the stack cache size is not exceeded
  if(size_blocks > Num_blocks)
  {
    simulation_exception_t::stack_exceeded("Freeing more blocks than"
      " the number of blocks in the stack cache");
  }

  // also free space in memory?
  if (freed_spilled_blocks)
  {
    // update the stack top pointer of the processor
    stack_spill += freed_spilled_blocks * Num_block_bytes;
  }
  
  stack_top += size_blocks * Num_block_bytes;
  if (stack_top > lazy_pointer) {
	  lazy_pointer = stack_top;
  }
  
  // update statistics
  if (stack_top == stack_spill) {
    Num_free_empty++;
  }

  return 0;
}



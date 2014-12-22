/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

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

void stack_cache_t::write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  assert(false);
  abort();
}

bool stack_cache_t::is_ready()
{
  assert(false);
  abort();
}


word_t ideal_stack_cache_t::prepare_reserve(simulator_t &s, uword_t size, 
                              uword_t &stack_spill, uword_t &stack_top)
{
  stack_top -= size;
  return 0;
}

word_t ideal_stack_cache_t::prepare_free(simulator_t &s, uword_t size, 
                            uword_t &stack_spill, uword_t &stack_top)
{
  stack_top += size;
  stack_spill = std::max(stack_spill, stack_top);
  return 0;
}

word_t ideal_stack_cache_t::prepare_ensure(simulator_t &s, uword_t size, 
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

word_t ideal_stack_cache_t::prepare_spill(simulator_t &s, uword_t size, 
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

bool ideal_stack_cache_t::reserve(simulator_t &s, uword_t size, word_t delta,
                                  uword_t new_spill, uword_t new_top)
{
  Content.resize(Content.size() + size);
  return true;
}

bool ideal_stack_cache_t::free(simulator_t &s, uword_t size, word_t delta,
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

bool ideal_stack_cache_t::ensure(simulator_t &s, uword_t size, word_t delta,
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
    Memory.read_peek(s, sp, &c, 1);
    Content[Content.size() - (sp - new_top) - 1] = c;
  }
  return true;
}

bool ideal_stack_cache_t::spill(simulator_t &s, uword_t size, word_t delta,
                                uword_t new_spill, uword_t new_top)                                
{
  // write back to memory
  for (int i = 0; i < delta; i++) {
    byte_t c = Content[Content.size() - (new_spill - new_top) - i - 1];
    Memory.write_peek(s, new_spill + i, &c, 1);
  }
  return true;
}

bool ideal_stack_cache_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
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

bool ideal_stack_cache_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
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

void ideal_stack_cache_t::read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  // we do not simulate timing here anyway..
  read(s, address, value, size);
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



bool proxy_stack_cache_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  return Memory.read(s, stack_top + address, value, size);
}

bool proxy_stack_cache_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  return Memory.write(s, stack_top + address, value, size);
}

void proxy_stack_cache_t::read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  return Memory.read_peek(s, stack_top + address, value, size);
}




block_stack_cache_t::block_stack_cache_t(memory_t &memory, 
                                         unsigned int num_blocks, 
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

word_t block_stack_cache_t::prepare_reserve(simulator_t &s, uword_t size, 
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
                                           "lowest possible address.");
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

bool block_stack_cache_t::reserve(simulator_t &s, uword_t size, word_t delta,
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
      if (Memory.write(s, new_spill, &Buffer[0], delta)) 
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


word_t block_stack_cache_t::prepare_free(simulator_t &s, uword_t size, 
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

bool block_stack_cache_t::free(simulator_t &s, uword_t size, word_t delta,
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


word_t block_stack_cache_t::prepare_ensure(simulator_t &s, uword_t size, 
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

bool block_stack_cache_t::ensure(simulator_t &s, uword_t size, word_t delta,
                                 uword_t new_spill, uword_t new_top)
{
  // do we need to fill?
  if (!delta) {
    // no, done.
    return true;
  }

  Phase = FILL;

  // copy the data from memory into a temporary buffer
  if (Memory.read(s, new_spill - delta, Buffer, delta))
  {
    // Ensure the size of the stack cache is larger than the block that needs to 
    // be loaded
    uword_t new_size = new_spill > new_top ? new_spill - new_top : 0;
    Content.insert(Content.begin(), 
                   new_size > Content.size() ? new_size - Content.size() : 0, 
                   0);
    
    // copy the data back into the stack cache
    for(unsigned int i = 0; i < delta; i++)
    {
      assert(delta - i - 1 >= 0);
      assert(delta - i - 1 < Content.size());
      Content[delta - i - 1]  = Buffer[i];
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


word_t block_stack_cache_t::prepare_spill(simulator_t &s, uword_t size, 
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

bool block_stack_cache_t::spill(simulator_t &s, uword_t size, word_t delta,
                                uword_t new_spill, uword_t new_top)
{
  switch (Phase)
  {
    case IDLE:
    {
      // do we need to spill?
      if (!delta) {
        // no, done.
        return true;
      }

      if (Content.size() < delta) {
        simulation_exception_t::stack_exceeded("Trying to spill more than the current size of the stack.");
      }
      
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
      if (Memory.write(s, new_spill, &Buffer[0], delta))
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


bool block_stack_cache_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  // read data
  bool result = ideal_stack_cache_t::read(s, address, value, size);
  assert(result);

  // update statistics
  Num_read_accesses++;
  Num_bytes_read += size;

  return true;
}

bool block_stack_cache_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  // read data
  bool result = ideal_stack_cache_t::write(s, address, value, size);
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
                                      const stats_options_t& options)
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
                      "   Miss Stall Cycles   : %15$10d  %16$10.2f%%\n")
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

block_aligned_stack_cache_t::block_aligned_stack_cache_t(memory_t &memory, 
                          unsigned int num_blocks, unsigned int num_block_bytes)
  : block_stack_cache_t(memory, (num_blocks*num_block_bytes)/4, 4), 
    Num_transfer_block_bytes(num_block_bytes),
    Num_words_spilled(0), Max_words_spilled(0),
    Num_words_filled(0), Max_words_filled(0),
    Num_words_free_filled(0), Max_words_free_filled(0)
{
}

word_t block_aligned_stack_cache_t::prepare_reserve(simulator_t &s, 
                                                    uword_t size,
                                                    uword_t &stack_spill,
                                                    uword_t &stack_top)
{
  // ensure that the reserved space does not exceed the stack cache's size minus
  // one block.
  assert(size <= Num_block_bytes * Num_blocks);

  // pre-compute required spilling
  word_t retval = block_stack_cache_t::prepare_reserve(s, size, stack_spill, 
                                                       stack_top);

  // round stack spill down to transfer block size
  uword_t alignment_fixup = stack_spill % Num_transfer_block_bytes;
  stack_spill -= alignment_fixup;

  // update stats
  Num_words_spilled += alignment_fixup / 4;
  Max_words_spilled = std::max(Max_words_spilled, alignment_fixup / 4);

  // increment number of blocks to be actually spilled
  return retval + alignment_fixup;
}

word_t block_aligned_stack_cache_t::prepare_ensure(simulator_t &s, uword_t size,
                                                   uword_t &stack_spill,
                                                   uword_t &stack_top)
{
  // ensure that the reserved space does not exceed the stack cache's size minus
  // one block.
  assert(size <= Num_block_bytes * Num_blocks);

  // pre-compute required filling
  word_t retval = block_stack_cache_t::prepare_ensure(s, size, stack_spill, 
                                                      stack_top);

  // was the transfer alligned?
  uword_t alignment_fixup = stack_spill % Num_transfer_block_bytes;
  if (alignment_fixup != 0) 
  {
    // compute actual fixup
    alignment_fixup = Num_transfer_block_bytes - alignment_fixup;

    // round stack spill up to transfer block size
    stack_spill +=  alignment_fixup;

    // update stats
    Num_words_filled += alignment_fixup / 4;
    Max_words_filled = std::max(Max_words_filled, alignment_fixup / 4);
  }

  // increment number of blocks to be actually filled
  return retval + alignment_fixup;
}

word_t block_aligned_stack_cache_t::prepare_free(simulator_t &s, uword_t size, 
                                                 uword_t &stack_spill, 
                                                 uword_t &stack_top)
{
  // ensure that the reserved space does not exceed the stack cache's size minus
  // one block.
  assert(size <= Num_block_bytes * Num_blocks);

  // pre-compute required spilling
  word_t retval = block_stack_cache_t::prepare_free(s, size, stack_spill, 
                                                    stack_top);
  assert(retval == 0);

  // was the transfer alligned?
  uword_t alignment_fixup = stack_spill % Num_transfer_block_bytes;
  if (alignment_fixup != 0) 
  {
    // compute actual fixup
    alignment_fixup = Num_transfer_block_bytes - alignment_fixup;

    // round stack spill up to transfer block size
    stack_spill +=  alignment_fixup;

    // update stats
    Num_words_free_filled += Num_transfer_block_bytes / 4;
    Max_words_free_filled = std::max(Max_words_filled, 
                                     Num_transfer_block_bytes / 4);

    return Num_transfer_block_bytes;
  }
  else 
  {
    return 0;
  }
}

bool block_aligned_stack_cache_t::free(simulator_t &s, uword_t size, 
                                       word_t delta, uword_t new_spill, 
                                       uword_t new_top)
{
  // no transfer needed
  if (delta == 0)
    return block_stack_cache_t::free(s, size, delta, new_spill, new_top);

  // ensure that a single block is to be filled
  assert(delta == Num_transfer_block_bytes);

  // only perform this the first time the function gets called
  if(Phase == IDLE)
  {
    // first, clear the stack cache content ...
    Content.clear();
  }

  //  ... then execute a one-block fill, if needed ...
  bool retval = ensure(s, delta, delta, new_spill, new_spill - delta);

  // ... then make sure that the content matches the actual number of blocks 
  // reserved in the cache.
  if (retval) 
  {
    Content.resize(get_num_reserved_blocks(new_spill, new_top)*4);
  }

  return retval;
}

void block_aligned_stack_cache_t::print_stats(const simulator_t &s, 
                                              std::ostream &os, 
                                              const stats_options_t& options)
{
  // print generic stack cache statistics
  block_stack_cache_t::print_stats(s, os, options);

  // print stack cache statistics related to lazy pointer
  os << boost::format("   Align trans. (spill): %1$10d  %2$10d\n"
                      "   Align trans. (fill) : %3$10d  %4$10d\n"
                      "   Align trans. (free) : %5$10d  %6$10d\n")
    % Num_words_spilled % Max_words_spilled
    % Num_words_filled % Max_words_filled
    % Num_words_free_filled % Max_words_free_filled;
}

void block_aligned_stack_cache_t::reset_stats() 
{
  block_stack_cache_t::reset_stats();

  Num_words_spilled = 0;
  Max_words_spilled = 0;

  Num_words_filled = 0;
  Max_words_filled = 0;

  Num_words_free_filled = 0;
  Max_words_free_filled = 0;
}

block_lazy_stack_cache_t::block_lazy_stack_cache_t(memory_t &memory, 
                                                 unsigned int num_blocks, 
                                                 unsigned int num_block_bytes) :
    block_stack_cache_t(memory, num_blocks, num_block_bytes), 
    Lazy_pointer(0), Num_blocks_to_evict(0),
    Num_blocks_not_spilled(0), Max_blocks_not_spilled(0)
{
}

word_t block_lazy_stack_cache_t::prepare_reserve(simulator_t &s, uword_t size, 
                                                 uword_t &stack_spill, 
                                                 uword_t &stack_top)
{
  // ensure that he lazy pointer is valid -- by initializing the 
  // Next_Lazy_pointer
  assert(Lazy_pointer >= 0 && stack_top <= stack_spill && 
         Num_blocks_to_evict == 0); 
  Next_Lazy_pointer = std::min(Lazy_pointer, stack_spill - stack_top);
  
  // check if all data in the stack cache is coherent with main memory. if so,
  // the "uninitialized" data just reserved is interpreted as coherent as well.
  bool lp_pulldown = (Next_Lazy_pointer == 0);

  // also mark all data coherent when the entire stack cache is reserved.
  bool lp_spillall = (size == Num_block_bytes * Num_blocks);
                     
  // number of blocks not spilled due to lazy pointer
  unsigned int num_blocks_not_spilled = 0;
  
  // from the perspective of the original stack cache the Lazy_pointer is equal 
  // to the current stack spill pointer
  uword_t lazy_stack_spill = stack_top + Next_Lazy_pointer;
  assert(lazy_stack_spill <= stack_spill);
  word_t retval = block_stack_cache_t::prepare_reserve(s, size, lazy_stack_spill, 
                                                       stack_top);
  assert(lazy_stack_spill <= stack_spill);

  // update the stack spill pointer, if needed
  if (stack_spill - stack_top > Num_blocks * Num_block_bytes) {
    num_blocks_not_spilled = (stack_spill - stack_top - 
                              Num_blocks * Num_block_bytes - retval) / 
                                Num_block_bytes;
    stack_spill = stack_top + Num_blocks * Num_block_bytes;
    
    // either we are not spilling or otherwise the spill pointer as to be equal
    // to the lazy_stack_spill pointer here
    assert(retval == 0 || lazy_stack_spill == stack_spill);
    
    // Store the number of blocks that have to be evicted by the reserve later.
    Num_blocks_to_evict = num_blocks_not_spilled;
    
    // make sure that at most the number of blocks above the lazy pointer are 
    // considered
    assert(num_blocks_not_spilled 
             <= Num_blocks * Num_block_bytes - Next_Lazy_pointer);
  }
  else {
    // check that if the data is only spilled when the stack cache is actually 
    // full
    assert(retval == 0);
  }

  // update the lazy pointer
  if (lp_pulldown || lp_spillall) {
    // check that nothing has been spilled if all data was coherent before
    assert(retval == 0 || lp_spillall);

    // no need to spill uninitialized stack data
    Next_Lazy_pointer = 0;
  }
  else {
    // update the lazy pointer
    Next_Lazy_pointer = lazy_stack_spill - stack_top;
  }

  // update statistics
  Num_blocks_not_spilled += num_blocks_not_spilled;
  Max_blocks_not_spilled = std::max(Max_blocks_not_spilled, 
                                    num_blocks_not_spilled);

  return retval;
}

word_t block_lazy_stack_cache_t::prepare_free(simulator_t &s, uword_t size, 
                                       uword_t &stack_spill, uword_t &stack_top)
{
  // no need to ensure that he lazy pointer is valid here
  word_t tmp_lazy_pointer = Lazy_pointer + stack_top;
  
  // perform the usual a sfree for the stack cache
  word_t retval = block_stack_cache_t::prepare_free(s, size, stack_spill, 
                                                    stack_top);
  
  // update lazy pointer, if needed
  if (tmp_lazy_pointer < stack_top) {
    Next_Lazy_pointer = 0;
  }
  else {
    Next_Lazy_pointer = tmp_lazy_pointer - stack_top;
  }
  
  return retval;
}

bool block_lazy_stack_cache_t::free(simulator_t &s, uword_t size, word_t delta,
                                    uword_t new_spill, uword_t new_top)                               
{
  // finally, update the lazy pointer
  Lazy_pointer = Next_Lazy_pointer;

  // free the space  
  return block_stack_cache_t::free(s, size, delta, new_spill, new_top);
}

bool block_lazy_stack_cache_t::reserve(simulator_t &s, uword_t size, word_t delta,
                                       uword_t new_spill, uword_t new_top)
{
  // update the lazy pointer
  Lazy_pointer = Next_Lazy_pointer;
  
  // check if we need to evict blocks that need no spilling.
  if(Num_blocks_to_evict != 0) {
    assert(Phase == IDLE);
  
    // evict the blocks
    Content.erase(Content.begin(), Content.begin() + 
                                     Num_blocks_to_evict * Num_block_bytes);
    
    // reset the counter
    Num_blocks_to_evict = 0;
  }
  
  return block_stack_cache_t::reserve(s, size, delta, new_spill, new_top);
}

bool block_lazy_stack_cache_t::write(simulator_t &s, uword_t address, byte_t *value, 
                                     uword_t size)
{
  // we cannot ensure that he lazy pointer is valid here
  
  // update the lazy pointer: if the address of the last modified byte is larger 
  // than the lazy pointer, push the lazy pointer up -- aligned to block 
  // addresses.
  uword_t max_modified_block_aligned = ((address + size + Num_block_bytes - 1) / 
                                             Num_block_bytes) * Num_block_bytes;

  assert(address + size <= max_modified_block_aligned &&
         max_modified_block_aligned <= Num_blocks * Num_block_bytes);
  
  Lazy_pointer = std::max(max_modified_block_aligned, Lazy_pointer);
  
  // just to a regular write from now on
  return block_stack_cache_t::write(s, address, value, size);
}

void block_lazy_stack_cache_t::print(std::ostream &os) const
{
    // print lazy pointer 
  os << boost::format("    LP : %1%\n") % Lazy_pointer;
  
  // print original stack cache content
  block_stack_cache_t::print(os);
}

void block_lazy_stack_cache_t::print_stats(const simulator_t &s, 
                                           std::ostream &os,
                                           const stats_options_t& options)
{
  // print generic stack cache statistics
  block_stack_cache_t::print_stats(s, os, options);
 
  // print stack cache statistics related to lazy pointer
  os << boost::format("   Blocks Not Spilled  : %1$10d  %2$10d\n")
    % Num_blocks_not_spilled % Max_blocks_not_spilled;
}

void block_lazy_stack_cache_t::reset_stats() 
{
  block_stack_cache_t::reset_stats();
  Num_blocks_not_spilled = 0;
  Max_blocks_not_spilled = 0;
}

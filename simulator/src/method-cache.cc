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
// Method-cache implementations.
//

#include "method-cache.h"

#include "basic-types.h"
#include "endian-conversion.h"
#include "instr-cache.h"
#include "simulation-core.h"
#include "symbol.h"

#include <cassert>
#include <cmath>
#include <map>
#include <ostream>
#include <limits>

#include <boost/format.hpp>
#include "exception.h"


using namespace patmos;

void ideal_method_cache_t::initialize(simulator_t &s, uword_t address)
{
  current_base = address;
}

bool ideal_method_cache_t::fetch(simulator_t &s, uword_t base, uword_t address,
				 word_t iw[2])
{
  Memory.read_peek(s, address, reinterpret_cast<byte_t*>(&iw[0]),
		   sizeof(word_t)*2);
  return true;
}

bool ideal_method_cache_t::load_method(simulator_t &s, word_t address,
				       word_t offset)
{
  current_base = address;
  return true;
}

bool ideal_method_cache_t::is_available(simulator_t &s, word_t address)
{
  return true;
}

uword_t ideal_method_cache_t::get_active_method_base()
{
  return current_base;
}

void ideal_method_cache_t::tick(simulator_t &s)
{
  // do nothing here
}

void ideal_method_cache_t::print(const simulator_t &s, std::ostream &os)
{
  // nothing to do here either, since the cache has no internal state.
}

void ideal_method_cache_t::print_stats(const simulator_t &s, std::ostream &os, 
                                       const stats_options_t& options)
{
  // nothing to do here either, since the cache has no internal state.
}
    

void method_stats_info_t::add_stall_cycles(unsigned int stalls, 
                                           unsigned int data_stalls)
{
  Num_stall_cycles += stalls;
  Num_data_stall_cycles += data_stalls;
  Min_stall_cycles = std::min(Min_stall_cycles, stalls);
  Max_stall_cycles = std::max(Max_stall_cycles, stalls);
  Min_data_stall_cycles = std::min(Min_data_stall_cycles, data_stalls);
  Max_data_stall_cycles = std::max(Max_data_stall_cycles, data_stalls);
}

void lru_method_cache_t::method_info_t::update(uword_t address, 
                                               uword_t num_blocks,
                                               uword_t num_bytes,
                                               bool    is_disposable)
{
  Address = address;
  Num_blocks = num_blocks;
  Num_bytes = num_bytes;
  Is_disposable = is_disposable;
  reset_utilization();
}

void lru_method_cache_t::method_info_t::reset_utilization() {
  Utilization.clear();
  Utilization.resize(Num_bytes / sizeof(uword_t) + 3);        
}

unsigned int lru_method_cache_t::method_info_t::get_utilized_bytes() {
  uword_t utilized_bytes = 0;
  for (int i = 0; i < Utilization.size(); i++) {
    if (Utilization[i]) {
      utilized_bytes += sizeof(uword_t);
    }
  }
  return utilized_bytes;
}


bool lru_method_cache_t::do_fetch(simulator_t &s, method_info_t &current_method,
                                  uword_t address, word_t iw[2])
{
  uword_t end_address = current_method.Address + current_method.Num_bytes;
 
  if ((Phase != IDLE && Phase != TRANSFER_DELAYED) ||
      address < current_method.Address ||
      end_address + sizeof(word_t) * NUM_SLOTS * 3 <= address)
  {
    simulation_exception_t::illegal_pc(current_method.Address);
  }
  
  // Automagically fall though to the next block by inserting a BRCF instruction
  // on the fly when we read beyond the last instruction.
#ifdef MC_AUTO_FALLTHROUGH
  if (address == end_address) {

    // TODO use the Instructions array and binary-fmt class to get the 
    //      instruction encodings
    
    // BRCFND <next block address>
    uword_t imm = s.get_next_method_base(address) >> 2;
    iw[0] = to_big_endian<big_uword_t,uword_t>(0x05000000 | imm);
    // NOP
    iw[1] = to_big_endian<big_uword_t,uword_t>(0x00400000);
    
    return true;
  }
#endif
  
  // Stall in fetch stage if code has not been fetched yet, but only while
  // some transfers are still outstanding, otherwise we would stall infinitely
  // on non-delayed branches where fetching would continue after the end of 
  // the function
  if (Phase == TRANSFER_DELAYED) 
  {
    uword_t fetch_end = Current_fetch_address;
    // Also allow execution within currently fetched burst.
    fetch_end += Current_burst_transferred;
    if ((address + sizeof(word_t) * NUM_SLOTS) > fetch_end)
    {
      // We are still waiting for the transfer to finish for this address.
      // Note: we count stall cycles here only if Phase == TRANSFER_DELAYED
      Num_stall_cycles++;
      Curr_stall_cycles++;
      if (!Memory.is_serving_request(Current_fetch_address)) {
        Curr_data_stall_cycles++;
      }
      return false;
    }
  }
  
  // get instruction word from the method's instructions
  byte_t *iwp = reinterpret_cast<byte_t*>(&iw[0]);
  
  // TODO read from Cache buffer, get read position(s) from method_info.
  
  Memory.read_peek(s, address, iwp, sizeof(word_t)*NUM_SLOTS);
    
  for (unsigned int i = 0; i < NUM_SLOTS; i++) {
    unsigned int word = (address-current_method.Address)/sizeof(word_t) + i;
    if (word >= current_method.Num_bytes/sizeof(word_t)) {
      break;
    }
    current_method.Utilization[word] = true;
  }
  
  Num_bytes_fetched += sizeof(word_t) * NUM_SLOTS;
  
  return true;
}

bool lru_method_cache_t::lookup(simulator_t &s, uword_t address)
{
  int index = get_index(s, address);

  // check if the address is in the cache
  if (index >= 0) {
    // update the ordering of the methods to match LRU.

    // store the currently accessed entry
    method_info_t tmp = Methods[index];

    // shift all methods between the location of the currently accessed
    // entry and the previously most recently used entry.
    for(unsigned int j = index; j > 0; j--)
    {
      Methods[j] = Methods[j - 1];
    }

    // reinsert the current entry at the head of the table and update the
    // current method pointer
    Methods[0] = tmp;
    
    Active_method = 0;

    return true;
  }

  // No entry matches the given address.
  return false;
}

void lru_method_cache_t::update_utilization_stats(method_info_t &method,
                                                  uword_t utilized_bytes)
{
  
  float utilization = (float)utilized_bytes / (float)method.Num_bytes;

  Method_stats[method.Address].Max_utilization = std::max(utilization, 
                              Method_stats[method.Address].Max_utilization);
  Method_stats[method.Address].Min_utilization = std::min(utilization,
                              Method_stats[method.Address].Min_utilization);      
}

void lru_method_cache_t::update_evict_stats(method_info_t &method, 
                                            uword_t new_method, 
                                            eviction_type_e type)
{
  if (type != EVICT_FLUSH) {
    std::pair<unsigned int, unsigned int> &eviction_stats(
                          Method_stats[method.Address].Evictions[new_method]);
    if (type == EVICT_TAG)
    {
      eviction_stats.second++;
      Num_evictions_tag++;
    }
    else if (type == EVICT_CAPACITY)
    {
      eviction_stats.first++;
      Num_evictions_capacity++;
    }
  }

  unsigned int utilized_bytes = method.get_utilized_bytes();
  
  Num_bytes_utilized += utilized_bytes;
  
  update_utilization_stats(method, utilized_bytes);
}

unsigned int lru_method_cache_t::evict_method(unsigned int index, 
                                              uword_t evictor_address)
{
  method_info_t &method(Methods[index]);

  eviction_type_e eviction_type;
  if (method.Is_disposable) {
    eviction_type = EVICT_DISP;
  } else {
    // is this a cache miss due to the limited number of tag?
    if (Num_active_blocks + Current_allocate_blocks <= Num_blocks) {
      eviction_type = EVICT_TAG;
    } else {
      eviction_type = EVICT_CAPACITY;
    }
  }

  // update eviction statistics
  update_evict_stats(method, evictor_address, eviction_type);
  
  // evict the method from the cache
  if (index == Active_method || !method.Is_disposable) {
    Num_active_blocks -= method.Num_blocks;
    Num_active_methods--;
  }

  return method.Num_blocks;
}

uword_t lru_method_cache_t::finish_eviction(unsigned int evicted_blocks, 
                                         unsigned int evictor_blocks)
{
  uword_t blocks_freed = evicted_blocks > evictor_blocks ? 
                         evicted_blocks - evictor_blocks : 0;
  
  Num_blocks_freed += blocks_freed;
  Max_blocks_freed = std::max(Max_blocks_freed, blocks_freed);

  return blocks_freed;
}

void lru_method_cache_t::print_cache_state(simulator_t& s, 
                                           std::ostream& dout) const
{
  dout << "     { ";
  for (int i = 0; i < Num_active_methods; i++) {
    if (i > 0 && (i % 4 == 0)) {
      dout << " |\n";
      dout << "       ";
    } else if (i > 0) {
      dout << " | ";
    }
    if (i == Active_method) {
      dout << "*";
    } else {
      dout << " ";
    }
    
    std::stringstream ss;
    std::string symbol;
    s.Symbols.print(ss, Methods[i].Address, true);
    ss >> symbol;
    
    dout << boost::format("0x%1$08x: %2$-22s")
         % Methods[i].Address
         % symbol;
  }
  dout << "   }\n";
}

void lru_method_cache_t::print_hit(simulator_t &s, std::ostream& dout) const
{
  dout << boost::format("M$ HIT:  Entry %1$2d: 0x%2$08x %3$-30s Size: %4$6d, Used: %5$6d\n") 
       % Active_method
       % Methods[Active_method].Address
       % s.Symbols.find(Methods[Active_method].Address) 
       % Methods[Active_method].Num_bytes
       % Methods[Active_method].get_utilized_bytes();
  
  print_cache_state(s, dout);
  
  dout << "\n";
}

void lru_method_cache_t::print_miss(simulator_t &s, std::ostream& dout,
                                    uword_t evicted_methods, 
                                    uword_t evicted_blocks, 
                                    uword_t blocks_freed, 
                                    bool capacity_miss) const
{
  dout << boost::format("M$ MISS: Entry %1$2d: 0x%2$08x %3$-30s Size: %4$6d, Used: %5$6d\n") 
       % Active_method
       % Methods[Active_method].Address
       % s.Symbols.find(Methods[Active_method].Address) 
       % Methods[Active_method].Num_bytes
       % Methods[Active_method].get_utilized_bytes();
  if (evicted_methods > 0) {
    if (Methods[Active_method].Is_disposable) {
      dout << "         DISPOSABLE    ";
    } else if (capacity_miss) {
      dout << "         CAPACITY miss ";
    } else {
      dout << "         TAG SIZE miss ";
    }
    dout << boost::format("replaces %1$2d methods of %2$5d bytes, frees %3$5d bytes\n")
         % evicted_methods 
         % (evicted_blocks * Num_block_bytes) 
         % (blocks_freed * Num_block_bytes);
  } else {
    dout << "         COLD miss\n";
  }
  
  print_cache_state(s, dout);
  
  dout << "\n";
}

bool lru_method_cache_t::peek_function_size(simulator_t &s, 
                                            word_t function_base,
                                            uword_t &result_size,
                                            bool &disposable)
{
  uword_t num_bytes_big_endian;
  Memory.read_peek(s, function_base - sizeof(uword_t),
      reinterpret_cast<byte_t*>(&num_bytes_big_endian),
      sizeof(uword_t));

  // convert method size to native endianess and compute size in blocks
  result_size = from_big_endian<big_uword_t>(num_bytes_big_endian);
  
  // Extract the disposable flag that is located in bit 0 of the high half-word.
  disposable = result_size & (0x10000);
  // Apply a mask to extract the size as well.
  result_size &= ~(0x10000);
  
  return true;
}


uword_t lru_method_cache_t::get_num_blocks_for_bytes(uword_t num_bytes)
{
  return ((num_bytes - 1) / Num_block_bytes) + 1;
}

uword_t lru_method_cache_t::get_transfer_start(uword_t address) {
  return address - 4;
}

uword_t lru_method_cache_t::get_transfer_size()
{
  uword_t base = get_active_method_base();
  uword_t remaining;
  if (Current_fetch_address <= base) {
    // first transfer needs to include size word.
    remaining = Current_method_size + 4;
  } else {
    remaining = Current_method_size - 
                 (Current_fetch_address - get_active_method_base());
  }
  
  if (Transfer_mode != TM_BLOCKING) {
    // TODO ensure next transfer is read aligned!?
    return std::min(Transfer_block_size, remaining);
  } else {
    // Transfer everything in one big burst.
    // Memory controller aligns to burst size itself.
    return remaining;
  }
}

bool lru_method_cache_t::cache_fill(simulator_t& s)
{
  // TODO implement as actual cache, keep track of where to store
  // methods to in the cache buffer, and keep pointers into the cache in
  // the method_infos.

  // Perform the burst request(s).
  while (Memory.read_burst(s, Current_fetch_address, Cache, 
                        get_transfer_size(), Current_burst_transferred,
			true, Transfer_interruptable))
  {
    Current_fetch_address += Current_burst_transferred;

    // Update statistics
    Num_bytes_transferred += Current_burst_transferred;
    Num_max_bytes_transferred = std::max(Num_max_bytes_transferred,
                                         Current_burst_transferred);

    Current_burst_transferred  = 0;
   
    // If everything has been fetched, return to IDLE
    if (get_transfer_size() == 0) 
    {
      Current_allocate_blocks = Current_method_size = 0;
      Phase = IDLE;
      
      // Update statistics
      uword_t address = get_active_method_base();      
      Method_stats[address].add_stall_cycles(Curr_stall_cycles,
                                             Curr_data_stall_cycles);

      return true;
    }
  }

  // If we are transferring data in the background in tick(), resume execution.
  return Transfer_mode != TM_BLOCKING;
}


lru_method_cache_t::lru_method_cache_t(memory_t &memory, 
                    unsigned int num_blocks, 
                    unsigned int num_block_bytes, 
                    unsigned int max_active_methods,
                    transfer_mode_e transfer_mode,
                    unsigned int transfer_block_size) :
    method_cache_t(memory), Num_blocks(num_blocks), 
    Num_block_bytes(num_block_bytes), Phase(IDLE), Transfer_mode(transfer_mode), 
    Current_fetch_address(0), Current_burst_transferred(0),
    Current_allocate_blocks(0), Current_method_size(0), Num_active_methods(0),
    Num_active_blocks(0), Num_blocks_allocated(0), Num_cache_entries(0),
    Active_method(0),
    Num_max_blocks_allocated(0), Num_bytes_transferred(0),
    Num_max_bytes_transferred(0), Num_bytes_fetched(0), 
    Num_max_active_methods(0),
    Num_hits(0), Num_misses(0), Num_misses_ret(0), Num_evictions_capacity(0),
    Num_evictions_tag(0), Num_stall_cycles(0),
    Curr_stall_cycles(0), Curr_data_stall_cycles(0),
    Num_bytes_utilized(0), Num_blocks_freed(0), Max_blocks_freed(0)
{
  unsigned int cache_size = num_block_bytes * num_blocks;
  
  Max_methods = max_active_methods ? max_active_methods : num_blocks;
  Transfer_block_size = transfer_block_size ? transfer_block_size 
                                            : cache_size;
  Transfer_interruptable = (transfer_block_size == 0);
                                            
  // Size of the Methods array. We basically only need to store Num_max_methods,
  // but in order to keep disposable methods in the cache to track avoidable
  // misses, we make it a bit larger.
  Max_cache_entries = Max_methods * 2;
  
  Methods = new method_info_t[Max_cache_entries];
  for(unsigned int i = 0; i < Max_cache_entries; i++)
    Methods[i] = method_info_t();
  
  // Buffer for loading a method. Max method size is the size of the cache, plus
  // the size word.
  Cache = new byte_t[cache_size + 4];
}

void lru_method_cache_t::initialize(simulator_t &s, uword_t address)
{
  assert(Num_active_blocks == 0 && Num_active_methods == 0);

  // get 'most-recent' method of the cache
  method_info_t &current_method = Methods[0];
  Active_method = 0;

  // we assume it is an ordinary function entry with size specification
  // (the word before) and copy it in the cache.
  uword_t num_bytes, num_blocks;
  bool    isDisposable;
  peek_function_size(s, address, num_bytes, isDisposable);
  num_blocks = get_num_blocks_for_bytes(num_bytes);

  current_method.update(address, num_blocks, num_bytes, isDisposable);
  Num_active_blocks = num_blocks;

  Num_cache_entries = 1;
  Num_active_methods = 1;
  
  Num_max_active_methods = std::max(Num_max_active_methods, 1U);
}

bool lru_method_cache_t::fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[2])
{
  // fetch from 'most-recent' method of the cache
  return do_fetch(s, Methods[Active_method], address, iw);
}

bool lru_method_cache_t::load_method(simulator_t &s, word_t address, word_t offset)
{

  // If we try to load a new method while there is still a transfer taking place,
  // stall until the current transfer is finished.
  if (Phase == TRANSFER_DELAYED) {
    
    // Abort the transfer instead, if the current method is disposable?
    // We can take care of it in the compiler and split instead!
    
    return false;
  }
    
  bool available = false;
  uword_t evicted_blocks = 0;
  uword_t evicted_methods = 0;
  
  // check status of the method cache
  switch(Phase)
  {
    // a new request has to be started.
    case IDLE:
    {
      // Before we start fetching the new function, dispose the current one in
      // any case if it is a disposable function.
      // This needs to be done before we do the lookup, as it might change the
      // current active method, and we need to do it independent of whether
      // this is a hit or a miss.
      if (Methods[Active_method].Is_disposable) {
        // By definition, a disposable method is evicted after leaving its code 
        // region. Any active disposable method *must* thus be the most recent
        // cache entry.
        assert(Active_method == 0);
        
        // evict the method from the cache and update the statistics
        evicted_blocks += evict_method(Active_method, address);
        
        // Note: We keep the disposable method in Methods[] (i.e., it remains to
        // be a *valid* entry), hence Num_cache_entries is NOT changed, but the 
        // disposable method is now no longer active (and neither is any other 
        // disposable method).
        // Also note that it does not matter that the most recent method is
        // no longer active (i.e, active and non-active disposable methods
        // can appear in any order in Methods[]).
        assert(Num_cache_entries > Num_active_methods);
      }      
      
      available = lookup(s, address);

      assert(Current_allocate_blocks == 0 && Current_method_size == 0);

      // Method is available and not disposable?
      if (available && !Methods[Active_method].Is_disposable)
      {
        // method is in the cache ... done!
        
        // Finish eviction of previous active disposable method
        if (evicted_blocks) {
          finish_eviction(evicted_blocks, Current_allocate_blocks);
        }
        
        // update statistics
        Num_hits++;
        Method_stats[address].Accesses[offset].hits++;
	assert(!Method_stats[address].Is_disposable);
        
        if (s.Dbg_stack.get_stats_options().debug_cache == patmos::DC_ALL &&
            s.Dbg_stack.is_printing())
        {
          print_hit(s, *s.Dbg_stack.get_stats_options().debug_out);
        }
        return true;
      }
      else
      {
        if (available)
        {
          // Method is disposable but would have still been in the cache
          // TODO This is not quite correct, since we do not track when
          // the method would have actually been dismissed based on the total
          // size, i.e., this is an overapproximation, assuming a larger cache.
          Avoidable_misses++;
          Method_stats[address].Accesses[offset].avoidable_misses++;
        }
        Num_misses++;
        if (offset != 0) {
          Num_misses_ret++;
        }
        Method_stats[address].Accesses[offset].misses++;

        // proceed to next phase ... fetch the size from memory.
        // NOTE: the next phase starts immediately.
        Phase = SIZE;
      }
    }

    // the size of the method has to be fetched from memory.
    case SIZE:
    {
      assert(Current_allocate_blocks == 0 && Current_method_size == 0);

      bool disposable;
      
      // get the size & the disposable flag of the method that should be loaded
      bool success = peek_function_size(s, address, Current_method_size, 
                                                    disposable);
      // At this point, we do not simulate any timing for reading the size word.
      // We simulate reading the size word later when fetching the content, but
      // we update the cache now already. In the hardware this will be done 
      // together with the first burst read.
      assert(success);

      Current_allocate_blocks = get_num_blocks_for_bytes(Current_method_size);

      // TODO should we also store how many bytes are actually transferred
      // by the memory? Ask the Memory for the actual transfer size.
      Method_stats[address].Num_method_bytes = Current_method_size;
      Method_stats[address].Num_blocks_allocated = Current_allocate_blocks;
      Method_stats[address].Is_disposable = disposable;

      // check method size against cache size.
      if (Current_allocate_blocks == 0 || Current_allocate_blocks > Num_blocks)
      {
        simulation_exception_t::code_exceeded(address);
      }

      bool capacity_miss = (Num_active_blocks + Current_allocate_blocks > Num_blocks);

      // throw other entries out of the cache if needed
      while (Num_active_blocks + Current_allocate_blocks > Num_blocks ||
             Num_active_methods >= Max_methods)
      {
        assert(Num_active_methods > 0);
        
        // Take the last one down..
        evicted_blocks += evict_method(Num_cache_entries - 1, address);
        evicted_methods++;
        
        // Make the entry invalid in any case, we simulate overwriting the
        // tail here! Any disposable or non-disposable methods are now gone.
        Num_cache_entries--;
      }
      
      uword_t blocks_freed = finish_eviction(evicted_blocks, Current_allocate_blocks);
      
      // Check how many entries we need to move in order to make space for the
      // new one.
      unsigned int last;
      
      // If the method was available, then it has to be a disposable method.
      // Then there are two possibilities: either the entry is still valid, or
      // we evicted it earlier to make space for the new active entry.
      // In the first case, we could reuse the entry at the old position,
      // but we would violate the property that the active disposable entry
      // is always at index 0, so for convenience we move it to the front.
      // In the other case, the entry is no longer valid so we handle it
      // like the normal, "non-available" case.
      if (available && Active_method < Num_cache_entries) {
        // If the method was available and we reached this phase,
        // then it has to be a disposable method.
        assert(Methods[Active_method].Is_disposable);

        // We need to make sure that we remove the old entry.
        // We reload the method and make it the most recent entry later.
        // In case of LRU, the old entry became the most recent one by the
        // activation earlier. In case of FIFO, we have to move the method to
        // the front now (by setting last = Active_method now).
        // The number of valid entries remains constant, we just move one entry.
        last = Active_method;
      }
      else if (Num_cache_entries < Max_cache_entries) {
        // Enough space available .. just move everything down, keep all
        // disposable methods.
        // TODO This makes the cache for disposable methods larger than it is,
        //      the avoidable_misses will be an overapproximation. Maybe throw
        //      out the oldest disposable methods earlier, but using which
        //      criteria?? We would need to co-simulate a different cache.
        last = Num_cache_entries;
        // We shift to an unused position, so the number of valid entries
        // *increases* by one due to the method we are going to fetch.
        Num_cache_entries++;
      } else {
        // Throw out the oldest disposable method to make space.

        // There must be at least one disposable method somewhere ...
        assert(Num_cache_entries > Num_active_methods);
        
        last = Num_cache_entries - 1;
        // ... therefore this loop terminates
        while (!Methods[last].Is_disposable) {
          last--;
        }
        
        // We overwrite a valid (but non-active) entry by shifting and insert
        // a new entry at the top, so the total number does not change, i.e.,
        // Num_cache_entries remains constant.
      }
      
      // shift the remaining blocks
      for(unsigned int j = last; j > 0; j--)
      {
        Methods[j] = Methods[j - 1];
      }

      // Insert the new entry at the head of the table
      // Also, update the current method pointer and reset utilization.
      Methods[0].update(address, Current_allocate_blocks,
                        Current_method_size, disposable);

      Active_method = 0;

      // update counters
      Num_active_methods++;
      Num_active_blocks += Current_allocate_blocks;
      
      // update statistics
      Num_max_active_methods = std::max(Num_max_active_methods,
                                        Num_active_methods);
      Num_blocks_allocated += Current_allocate_blocks;
      Num_max_blocks_allocated = std::max(Num_max_blocks_allocated,
                                          Current_allocate_blocks);

      if (s.Dbg_stack.get_stats_options().debug_cache != patmos::DC_NONE &&
	  s.Dbg_stack.is_printing())
      {
	print_miss(s, *s.Dbg_stack.get_stats_options().debug_out,
		   evicted_methods, evicted_blocks, blocks_freed, 
		   capacity_miss);
      }

      // proceed to next phase ... the size of the method has been fetched
      // from memory, now transfer the method's instructions.
      assert(Current_allocate_blocks != 0 && Current_method_size != 0);

      // Initialize first transfer
      Current_fetch_address = get_transfer_start(address);
      Current_burst_transferred = 0;
      
      Curr_stall_cycles = 0;
      Curr_data_stall_cycles = 0;
      
      // NOTE: the next phase starts immediately.
      Phase = TRANSFER;
    }

    // begin transfer from main memory to the method cache.
    case TRANSFER:
    {
      // If we want to use delayed transfers, switch over to a delayed transfer
      if (Transfer_mode != TM_BLOCKING) 
      {
        Phase = TRANSFER_DELAYED;
      }
      
      // Perform the memory transfer
      return cache_fill(s);
    }
    
    case TRANSFER_DELAYED: 
    {
      assert("Should never be reached!");
    }
  }

  assert(false);
  abort();
}

bool lru_method_cache_t::is_available(simulator_t &s, word_t address)
{
  return (get_index(s, address) > -1);
}

int lru_method_cache_t::get_index(simulator_t &s, word_t address)
{
  // check if the address is in the cache
  for(int i = 0; i < Num_cache_entries; i++)
  {
    if (Methods[i].Address == address)
    {
      return i;
    }
  }

  return -1;
}

uword_t lru_method_cache_t::get_active_method_base()
{
  return Methods[Active_method].Address;
}

void lru_method_cache_t::tick(simulator_t &s)
{
  if (Phase == TRANSFER_DELAYED) 
  {
    // continue filling the cache.
    cache_fill(s);
    // Note: stall counters are updated in fetch stage
  }
  else if (Phase != IDLE)
  {
    // update statistics
    // Note: we count stall cycles here only if Phase != TRANSFER_DELAYED
    Num_stall_cycles++;
    Curr_stall_cycles++;
    if (!Memory.is_serving_request(Current_fetch_address)) {
      Curr_data_stall_cycles++;
    }
  }
}

void lru_method_cache_t::print(const simulator_t &s, std::ostream &os)
{
  os << boost::format(" #Methods: %1$2d #Blocks: %2$2d #Disposable: %3$2d Active: %4$2d\n")
      % Num_active_methods % Num_active_blocks 
      % (Num_cache_entries - Num_active_methods) % Active_method;

  for(int i = 0; i < Num_cache_entries; i++)
  {
    os << boost::format("   M%1$02d: 0x%2$08x (%3$8d Blk %4$8d b) %5%   %6% ")
        % i % Methods[i].Address % Methods[i].Num_blocks
        % Methods[i].Num_bytes 
        % (Methods[i].Is_disposable ? "DISP" : "    ")
        % (i == Active_method ? "*" : " ");
    
    s.Symbols.print(os, Methods[i].Address);
    
    os << "\n";
  }
  
  os << '\n';
}

void lru_method_cache_t::print_stats(const simulator_t &s, std::ostream &os, 
                                     const stats_options_t& options)
{
  uword_t bytes_utilized = Num_bytes_utilized;
  for(unsigned int j = 0; j < Num_cache_entries; j++)
  {
    // non-active disposable methods have already been evicted.
    if (j != Active_method && Methods[j].Is_disposable) continue;
    
    uword_t ub = Methods[j].get_utilized_bytes();
    
    bytes_utilized += ub;
    
    update_utilization_stats(Methods[j], ub);
  }

  // per cache miss, we load the size word, but do not store it in the cache
  uword_t bytes_allocated = Num_bytes_transferred - Num_misses * 4;
  // Utilization = Bytes used / bytes allocated in cache
  float utilization = (float)bytes_utilized / 
                      (float)(Num_blocks_allocated * Num_block_bytes);
  // Internal fragmentation = Bytes loaded to cache / Bytes allocated in cache
  float fragmentation = 1.0 - (float)bytes_allocated / 
                      (float)(Num_blocks_allocated * Num_block_bytes);
  
  // External fragmentation = Blocks evicted but not allocated / Blocks allocated
  float ext_fragmentation = (float)Num_blocks_freed / 
                            (float)Num_blocks_allocated;
                      
  // Ratio of bytes loaded from main memory to bytes fetched from the cache.
  float transfer_ratio = (float)Num_bytes_transferred/(float)Num_bytes_fetched;

  // instruction statistics
  os << boost::format("                              total        max.\n"
                      "   Blocks Allocated    : %1$10d  %2$10d\n"
                      "   Bytes Transferred   : %3$10d  %4$10d\n"
                      "   Bytes Allocated     : %5$10d  %6$10d\n"
                      "   Bytes Used          : %7$10d\n"
                      "   Block Utilization   : %8$10.2f%%\n"
                      "   Int. Fragmentation  : %9$10.2f%%\n"
                      "   Bytes Freed         : %10$10d  %11$10d\n"
                      "   Ext. Fragmentation  : %12$10.2f%%\n"
                      "   Max Methods in Cache: %13$10d\n"
                      "   Cache Hits          : %14$10d  %15$10.2f%%\n"
                      "   Cache Misses        : %16$10d  %17$10.2f%%\n"
                      "   Cache Misses Returns: %18$10d  %19$10.2f%%\n"
                      "   Evictions Capacity  : %20$10d  %21$10.2f%%\n"
                      "   Evictions Tag       : %22$10d  %23$10.2f%%\n"
                      "   Transfer Ratio      : %24$10.3f\n"
                      "   Miss Stall Cycles   : %25$10d  %26$10.2f%%\n\n")
    % Num_blocks_allocated % Num_max_blocks_allocated
    % Num_bytes_transferred % Num_max_bytes_transferred
    % bytes_allocated % (Num_max_bytes_transferred - 4)
    % bytes_utilized % (utilization * 100.0) % (fragmentation * 100.0)
    % (Num_blocks_freed * Num_block_bytes) 
    % (Max_blocks_freed * Num_block_bytes) 
    % (ext_fragmentation * 100.0)
    % Num_max_active_methods 
    % Num_hits % (100.0 * Num_hits / (Num_hits + Num_misses))
    % Num_misses % (100.0 * Num_misses / (Num_hits + Num_misses))
    % Num_misses_ret % (100.0 * Num_misses_ret / Num_misses)
    % Num_evictions_capacity
    % (100.0*Num_evictions_capacity/(Num_evictions_capacity+Num_evictions_tag))
    % Num_evictions_tag
    % (100.0 * Num_evictions_tag / (Num_evictions_capacity + Num_evictions_tag))
    % transfer_ratio
    % Num_stall_cycles % (100.0 * Num_stall_cycles / (float)s.Cycle);

  if (options.short_stats) 
    return;
  
  // print stats per method
  os << "       Method:      #hits    #misses #av_misses methodsize   blocks  disposable  "
        "min-util    max-util  stall-cycles   min      max";
  if (options.extended_stall_stats) {
    os << "  data-stalls    min      max";
  }
  os << "\n";
  
  for(method_stats_t::iterator i(Method_stats.begin()),
      ie(Method_stats.end()); i != ie; i++)
  {
    unsigned int hits = 0;
    unsigned int misses = 0;
    unsigned int avoidable_misses = 0;

    for(offset_stats_t::iterator j(i->second.Accesses.begin()),
        je(i->second.Accesses.end()); j != je; j++)
    {
      hits += j->second.hits;
      misses += j->second.misses;
      avoidable_misses += j->second.avoidable_misses;
    }

    // Skip all stats entries that are never accessed since the last stats reset
    if (hits+misses == 0) {
      continue;
    }

    os << boost::format("   0x%1$08x: %2$10d %3$10d %4$10d %5$10d %6$8d %7$10d"
                        "%8$10.2f%% %9$10.2f%% %10$10d %11$8d %12$8d")
        % i->first % hits % misses % avoidable_misses
        % i->second.Num_method_bytes % i->second.Num_blocks_allocated
        % i->second.Is_disposable
        % (i->second.Min_utilization * 100.0)
        % (i->second.Max_utilization * 100.0)
        % i->second.Num_stall_cycles 
        % i->second.Min_stall_cycles % i->second.Max_stall_cycles;
     
        if (options.extended_stall_stats) 
     {
       os << boost::format(" %1$10d %2$8d %3$8d")  
          % i->second.Num_data_stall_cycles 
          % i->second.Min_data_stall_cycles % i->second.Max_data_stall_cycles;
     }
     
     os << "   ";
     s.Symbols.print(os, i->first);
     os << "\n";

    // print hit/miss statistics per offset
    if (options.hitmiss_stats)
    {
      for(offset_stats_t::iterator j(i->second.Accesses.begin()),
          je(i->second.Accesses.end()); j != je; j++)
      {
        os << boost::format("     0x%1$08x: %2$10d  %3$10d  %4$10d %5%\n")
            % (i->first + j->first)
            % j->second.hits
            % j->second.misses
            % j->second.avoidable_misses
            % s.Symbols.find(i->first + j->first);
      }
    }
  }

  // print Eviction statistics
  if (options.hitmiss_stats) {
    os << "\n       Method:    #capacity        #tag          by\n";
    for(method_stats_t::iterator i(Method_stats.begin()), ie(Method_stats.end());
        i != ie; i++)
    {
      // count number of evictions
      unsigned int num_capacity_evictions = 0;
      unsigned int num_tag_evictions = 0;
      for(eviction_stats_t::iterator j(i->second.Evictions.begin()),
          je(i->second.Evictions.end()); j != je; j++)
      {
        num_capacity_evictions += j->second.first;
        num_tag_evictions += j->second.second;
      }

      // print address and name of current method
      os << boost::format("   0x%1$08x:   %2$10d  %3$10d          %4%\n")
          % i->first % num_capacity_evictions % num_tag_evictions
          % s.Symbols.find(i->first);

      // print other methods who evicted this method
      for(eviction_stats_t::iterator j(i->second.Evictions.begin()),
          je(i->second.Evictions.end()); j != je; j++)
      {
        os << boost::format("                 %1$10d  %2$10d  0x%3$08x  %4%\n")
            % j->second.first % j->second.second % j->first
            % s.Symbols.find(j->first);
      }
    }
  }
}

void lru_method_cache_t::reset_stats() 
{
  Num_blocks_allocated = 0;
  Num_max_blocks_allocated = 0;
  Num_bytes_transferred = 0;
  Num_max_blocks_allocated = 0;
  Num_bytes_fetched = 0;
  Num_bytes_utilized = 0;
  Num_max_active_methods = 0;
  Num_hits = 0;
  Num_misses = 0;
  Num_misses_ret = 0;
  Num_evictions_capacity = 0;
  Num_evictions_tag = 0;
  Num_stall_cycles = 0;
  Num_blocks_freed = 0;
  Max_blocks_freed = 0;
  Method_stats.clear();
  for(unsigned int j = 0; j < Num_cache_entries; j++)
  {
    Methods[j].reset_utilization();
  }
}

void lru_method_cache_t::flush_cache() 
{
  if (Num_cache_entries <= 1) return;
  
  method_info_t active = Methods[Active_method];
  
  // Ensure that the active method is not flushed out.
  if (Active_method > 0) {
    method_info_t active = Methods[Active_method];
    Methods[Active_method] = Methods[0];
    Methods[0] = active;
    
    Active_method = 0;
  }
  
  for(unsigned int j = 1; j < Num_cache_entries; j++)
  {
    eviction_type_e evt = Methods[j].Is_disposable ? EVICT_DISP : EVICT_FLUSH;
    update_evict_stats(Methods[j], active.Address, evt);
  }

  Num_active_methods = 1;
  Num_cache_entries = 1;
  Num_active_blocks = Methods[0].Num_blocks;
}

/// free dynamically allocated cache memory.
lru_method_cache_t::~lru_method_cache_t()
{
  delete [] Methods;
  delete [] Cache;
}


bool fifo_method_cache_t::lookup(simulator_t &s, uword_t address)
{
  int index = get_index(s, address);
  
  if (index >= 0) {
    Active_method = index;
    return true;
  }
  
  return false;
}


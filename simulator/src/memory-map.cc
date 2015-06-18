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
// IO mapping implementation.
//

#include "memory-map.h"
#include "excunit.h"
#include "simulation-core.h"
#include "endian-conversion.h"

#include <vector>
#include <ostream>

using namespace patmos;

bool mapped_device_t::is_word_access(uword_t address, uword_t size, uword_t offset) {
  // TODO optionally check for half/byte access (?)
  return address == Base_address + offset && size == 4;
}

uword_t mapped_device_t::get_word(byte_t *value, uword_t size) {
  uword_t data = *((uword_t*)value);
  return (uword_t)from_big_endian<big_uword_t>(data);
}

void mapped_device_t::set_word(byte_t *value, uword_t size, uword_t data) {
  uword_t big_data = to_big_endian<big_uword_t>(data);
  *((uword_t*)value) = big_data;
}


bool cpuinfo_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, Cpu_cnt);
  } else {
    simulation_exception_t::unmapped(address);
  }
  return true;
}

bool cpuinfo_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  simulation_exception_t::illegal_access(address);
}

void cpuinfo_t::peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, Cpu_cnt);
  } else {
    mapped_device_t::peek(s, address, value, size);
  }
}

bool perfcounters_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  // TODO: return actual performance statistics
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, 0); // M-Cache hits
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, 0); // M-Cache misses
  } else if (is_word_access(address, size, 0x08)) {
    set_word(value, size, 0); // D-Cache hits
  } else if (is_word_access(address, size, 0x0c)) {
    set_word(value, size, 0); // D-Cache misses
  } else if (is_word_access(address, size, 0x10)) {
    set_word(value, size, 0); // S-Cache spill transactions
  } else if (is_word_access(address, size, 0x14)) {
    set_word(value, size, 0); // S-Cache fill transactions
  } else if (is_word_access(address, size, 0x18)) {
    set_word(value, size, 0); // Write combine buffer hits
  } else if (is_word_access(address, size, 0x1c)) {
    set_word(value, size, 0); // Write combine buffer misses
  } else if (is_word_access(address, size, 0x20)) {
    set_word(value, size, 0); // External memory read transaction
  } else if (is_word_access(address, size, 0x24)) {
    set_word(value, size, 0); // External memory write transaction
  } else {
    simulation_exception_t::unmapped(address);
  }
  return true;
}

bool perfcounters_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  simulation_exception_t::illegal_access(address);
}

bool led_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Curr_state);
    return true;
  }
  else {
    simulation_exception_t::unmapped(address);
  }
}

bool led_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    uword_t state = get_word(value, size);
     
    if (state == Curr_state) return true;
    
    Out_stream << "--- LEDs: [";
    for ( int i = 0; i < 32; i++ ) {
      if (i > 0) Out_stream << " ";
      if (state & (1 << i)) {
	Out_stream << "X";
      } else {
	Out_stream << "-";
      }
    }
    Out_stream << "] ---\n";
    
    Curr_state = state;
    return true;
  }
  else {
    simulation_exception_t::unmapped(address);
  }
}    


/// Map several devices into the address space of another memory device
mapped_device_t& memory_map_t::find_device(uword_t address)
{
  for (AddressList::iterator it = Device_map.begin(), ie = Device_map.end();
       it != ie; ++it)
  {
    if (address >= it->first && address <= it->second) {
      return *Devices[it - Device_map.begin()];
    }
  }
  simulation_exception_t::unmapped(address);
}

void memory_map_t::add_device(mapped_device_t &device) 
{
  Devices.push_back(&device);
  Device_map.push_back(std::make_pair(device.get_base_address(), 
				      device.get_base_address() + device.get_num_mapped_bytes() - 1));
}

bool memory_map_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).read(s, address, value, size);
  } else {
    return Memory.read(s, address, value, size);
  }
}

bool memory_map_t::read_burst(simulator_t &s, uword_t address, byte_t *value, 
                              uword_t size, uword_t &transferred, bool low_prio)
{
  if (address >= Base_address && address <= High_address) {
    // For the moment, we do not support bursted reads on memory mapped devices
    
    if (find_device(address).read(s, address, value, size)) {
      transferred = size;
      return true;
    } else {
      transferred = 0;
      return false;
    }
  } else {
    return Memory.read_burst(s, address, value, size, transferred, low_prio);
  }
}

bool memory_map_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).write(s, address, value, size);
  } else {
    return Memory.write(s, address, value, size);
  }
}

void memory_map_t::read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    find_device(address).peek(s, address, value, size);
  } else {
    Memory.read_peek(s, address, value, size);
  }
}

void memory_map_t::write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
{
  // TODO should we pass that to the mapped devices?
  assert(address < Base_address || address > High_address);
  Memory.write_peek(s, address, value, size);
}

bool memory_map_t::is_ready()
{
  // TODO should we pass that to the mapped devices?
  return Memory.is_ready();
}

bool memory_map_t::is_serving_request(uword_t address)
{
  // TODO we should pass that to the mapped devices
  //      We just need to check if the request goes to a mapped device, but
  //      we do not know if a mapped device is busy or not..
  return Memory.is_serving_request(address);
}

void memory_map_t::tick(simulator_t &s)
{
  for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->tick(s);
  }
  Memory.tick(s);
}

void memory_map_t::print(const simulator_t &s, std::ostream &os) const
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print(s, os);
  }      
  Memory.print(s, os);
}

void memory_map_t::print_stats(const simulator_t &s, std::ostream &os, 
			 const stats_options_t& options)
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print_stats(s, os, options);
  }
  Memory.print_stats(s, os, options);
}

void memory_map_t::reset_stats()
{
  for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->reset_stats();
  }      
  Memory.reset_stats(); 
}

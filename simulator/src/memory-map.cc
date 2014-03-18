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


bool cpuinfo_t::read(uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else {
    Exception_handler.unmapped(address);
  }
  return true;
}

bool cpuinfo_t::write(uword_t address, byte_t *value, uword_t size) {
  Exception_handler.illegal_access(address);
  return true;
}

void cpuinfo_t::peek(uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Cpu_id);
  } else if (is_word_access(address, size, 0x04)) {
    set_word(value, size, Cpu_freq);
  } else {
    mapped_device_t::peek(address, value, size);
  }
}

bool led_t::read(uword_t address, byte_t *value, uword_t size) {
  if (is_word_access(address, size, 0x00)) {
    set_word(value, size, Curr_state);
  }
  else {
    Exception_handler.unmapped(address);
  }
  return true;
}

bool led_t::write(uword_t address, byte_t *value, uword_t size) {
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
  }
  else {
    Exception_handler.unmapped(address);
  }
  return true;
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

bool memory_map_t::read(uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).read(address, value, size);
  } else {
    return Memory.read(address, value, size);
  }
}

bool memory_map_t::write(uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    return find_device(address).write(address, value, size);
  } else {
    return Memory.write(address, value, size);
  }
}

void memory_map_t::read_peek(uword_t address, byte_t *value, uword_t size)
{
  if (address >= Base_address && address <= High_address) {
    find_device(address).peek(address, value, size);
  } else {
    Memory.read_peek(address, value, size);
  }
}

void memory_map_t::write_peek(uword_t address, byte_t *value, uword_t size)
{
  // TODO should we pass that to the mapped devices?
  assert(address < Base_address || address > High_address);
  Memory.write_peek(address, value, size);
}

bool memory_map_t::is_ready()
{
  // TODO should we pass that to the mapped devices?
  return Memory.is_ready();
}

void memory_map_t::tick()
{
  for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->tick();
  }
  Memory.tick();
}

void memory_map_t::print(std::ostream &os) const
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print(os);
  }      
  Memory.print(os);
}

void memory_map_t::print_stats(const simulator_t &s, std::ostream &os, 
			 bool short_stats)
{
  for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
       it != ie; ++it) 
  {
    (*it)->print_stats(s, os, short_stats);
  }
  Memory.print_stats(s, os, short_stats);
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

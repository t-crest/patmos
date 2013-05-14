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
// IO mapping interface.
//

#ifndef PATMOS_MEMORY_MAP_H
#define PATMOS_MEMORY_MAP_H

#include <ostream>
#include <vector>

#include "memory.h"
#include "simulation-core.h"
#include "endian-conversion.h"

namespace patmos
{
  /// Default address of the UART status register.
  static const uword_t IOMAP_BASE_ADDRESS = 0xF0000000;

  /// Default address of the UART data register.
  static const uword_t IOMAP_HIGH_ADDRESS = 0xFFFFFFFF;

  // Offset to the IO base address for the CPU info.
  static const uword_t CPUINFO_BASE_OFFSET = 0x0000;
  
  // Number of bytes mapped to the CPU Info.
  static const uword_t CPUINFO_MAP_SIZE = 0x0100;
  
  // Offset to the IO base address for the LED IO.
  static const uword_t LED_BASE_OFFSET = 0x0200;
  
  // Number of bytes mapped to the LED IO.
  static const uword_t LED_MAP_SIZE = 0x0004;
  
  class mapped_device_t {
  protected:
    
    /// Base address of this device
    uword_t Base_address;
    
    // Number of bytes mapped to this device
    uword_t Mapped_bytes;
    
  public:
    
    mapped_device_t(uword_t base_address, uword_t mapped_bytes) 
    : Base_address(base_address), Mapped_bytes(mapped_bytes)
    {}
    
    virtual ~mapped_device_t() {}
    
    /// Check if the address is a word access to this device.
    /// @param address the memory address to check for a match.
    /// @param size the requested size of the access
    /// @param offset the offset to the base address to check with.
    bool is_word_access(uword_t address, uword_t size, uword_t offset) {
      // TODO optionally check for half/byte access (?)
      return address == Base_address + offset && size == 4;
    }
    
    /// Read a word from memory pointer
    /// @param value the pointer to the data to read
    /// @param size the size of the value to read in bytes
    /// @return the read value
    uword_t read_word(byte_t *value, uword_t size) {
      uword_t data = *((uword_t*)value);
      return (uword_t)from_big_endian<big_uword_t>(data);
    }
    
    /// Write a word to a memory pointer
    /// @param value the pointer to the data to write
    /// @param size the size of the value to write in bytes
    /// @param data the word to write
    void write_word(byte_t *value, uword_t size, uword_t data) {
      uword_t big_data = to_big_endian<big_uword_t>(data);
      *((uword_t*)value) = big_data;
    }
    
    /// Get the base address of this device.
    virtual uword_t get_base_address() const { return Base_address; }
    
    /// Get the number of bytes that are mapped to this device.
    virtual uword_t get_num_mapped_bytes() const { return Mapped_bytes; }
    
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

    /// A simulated access to a read port. Does not update the device state or 
    /// simulate timing, just reads the value.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void peek(uword_t address, byte_t *value, uword_t size) {
      // By default, just return zero, this is primarily used for debugging and
      // should not assert if read is supported.
      write_word(value, size, 0);
    }

    /// Notify the device that a cycle has passed.
    virtual void tick() { }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const { }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os) { }
  };
  
  
  class cpuinfo_t : public mapped_device_t 
  {
    simulator_t &Simulator;
    
    // Frequency of the CPU in MHz.
    double Frequency;
    
    uword_t Cpu_id;
    
    /// Latched high word of clock counter
    uword_t High_clock;
    
    /// Latched high word of usec counter
    uword_t High_usec;
  public:
    cpuinfo_t(simulator_t &s, uword_t base_address, uword_t cpuid, double frequency)
    : mapped_device_t(base_address, CPUINFO_MAP_SIZE), Simulator(s), 
      Cpu_id(cpuid), Frequency(frequency),
      High_clock(0), High_usec(0)
    {}
    
    virtual bool read(uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        write_word(value, size, Cpu_id);
      }
      else if (is_word_access(address, size, 0x10)) {
        // read latched high word of cycle counter
        write_word(value, size, High_clock);
      }
      else if (is_word_access(address, size, 0x14)) {
        // read low word of cycle counter, latch high word
        uword_t low_clock = (uword_t)Simulator.Cycle;
        High_clock = (uword_t)(Simulator.Cycle >> 32);
        write_word(value, size, low_clock);
      }
      else if (is_word_access(address, size, 0x18)) {
        // read latched high word of usec
        write_word(value, size, High_usec);
      }
      else if (is_word_access(address, size, 0x1c)) {
        // read low word of usec, latch high word
        // TODO if Frequency == 0, use wall clock for usec
        uint64_t usec = (uint64_t)((double)Simulator.Cycle / Frequency);
        uword_t low_usec = (uword_t)usec;
        High_usec = (uword_t)(usec >> 32);
        write_word(value, size, low_usec);
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual bool write(uword_t address, byte_t *value, uword_t size) {
      simulation_exception_t::illegal_access(address);
    }
    
    virtual void peek(uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        write_word(value, size, Cpu_id);
      }
      else {
        mapped_device_t::peek(address, value, size);
      }
    }
  };
  
  class led_t : public mapped_device_t 
  {
    /// Stream to write LED status to
    std::ostream &Out_stream;
    
    uword_t Curr_state;
  public:
    led_t(uword_t base_address, std::ostream &os) 
    : mapped_device_t(base_address, LED_MAP_SIZE), Out_stream(os) {}
    
    virtual bool read(uword_t address, byte_t *value, uword_t size) {
      simulation_exception_t::illegal_access(address);
    }

    virtual void peek(uword_t address, byte_t *value, uword_t size) {
      simulation_exception_t::illegal_access(address);
    }
    
    virtual bool write(uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        uword_t state = read_word(value, size);
         
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
        simulation_exception_t::unmapped(address);
      }
      return true;
    }    
  };
  
  /// A simple UART implementation allowing memory-mapped I/O.
  class memory_map_t : public memory_t
  {
  private:
    /// Memory onto which the address range is mapped.
    memory_t &Memory;

    typedef std::vector<mapped_device_t*> DeviceList;
    typedef std::vector<std::pair<uword_t, uword_t> > AddressList;
    
    DeviceList Devices;
    
    /// List of start-address,high-address pairs per device
    AddressList Device_map;
    
    uword_t Base_address;
    
    uword_t High_address;
    
  protected:
    mapped_device_t& find_device(uword_t address) 
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
    
  public:
    /// Construct a new memory map.
    /// @param memory The memory onto which the devices are memory mapped.
    /// @param base_address The start address of the mapped address range.
    /// @param high_address The highest address of the mapped address range.
    memory_map_t(memory_t &memory, uword_t base_address, uword_t high_address) 
    : Memory(memory), Base_address(base_address), High_address(high_address) 
    {
    }

    void add_device(mapped_device_t &device) 
    {
      Devices.push_back(&device);
      Device_map.push_back(std::make_pair(device.get_base_address(), 
                                          device.get_base_address() + device.get_num_mapped_bytes() - 1));
    }
    
    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      if (address >= Base_address && address <= High_address) {
        return find_device(address).read(address, value, size);
      } else {
        return Memory.read(address, value, size);
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
      if (address >= Base_address && address <= High_address) {
        return find_device(address).write(address, value, size);
      } else {
        return Memory.write(address, value, size);
      }
    }

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(uword_t address, byte_t *value, uword_t size)
    {
      if (address >= Base_address && address <= High_address) {
        find_device(address).peek(address, value, size);
      } else {
        Memory.read_peek(address, value, size);
      }
    }

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(uword_t address, byte_t *value, uword_t size)
    {
      // TODO should we pass that to the mapped devices?
      assert(address < Base_address || address > High_address);
      Memory.write_peek(address, value, size);
    }

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready()
    {
      // TODO should we pass that to the mapped devices?
      return Memory.is_ready();
    }

    /// Notify the memory that a cycle has passed.
    virtual void tick()
    {
      for (DeviceList::iterator it = Devices.begin(), ie = Devices.end();
           it != ie; ++it) 
      {
        (*it)->tick();
      }
      Memory.tick();
    }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
           it != ie; ++it) 
      {
        (*it)->print(os);
      }      
      Memory.print(os);
    }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(std::ostream &os)
    {
      for (DeviceList::const_iterator it = Devices.begin(), ie = Devices.end();
           it != ie; ++it) 
      {
        (*it)->print_stats(os);
      }
      Memory.print_stats(os);
    }
  };
}

#endif // PATMOS_MEMORY_MAP_H


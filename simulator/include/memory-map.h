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
// IO mapping interface.
//

#ifndef PATMOS_MEMORY_MAP_H
#define PATMOS_MEMORY_MAP_H

#include "memory.h"

#include <ostream>
#include <vector>

namespace patmos
{
  class simulator_t;
  
  /// Default address of the UART status register.
  static const uword_t IOMAP_BASE_ADDRESS = 0xF0000000;

  /// Default address of the UART data register.
  static const uword_t IOMAP_HIGH_ADDRESS = 0xFFFFFFFF;

  /// Offset from IO base address for the CPU info.
  static const uword_t CPUINFO_OFFSET = 0x0000;
  
  /// Number of bytes mapped to the CPU Info.
  static const uword_t CPUINFO_MAP_SIZE = 0x000C;
  
  /// Offset from IO base address for the exception unit.
  static const uword_t EXCUNIT_OFFSET = 0x0100;
  
  /// Number of bytes mapped to the exception unit.
  static const uword_t EXCUNIT_MAP_SIZE = 0x0100;

  /// Offset from IO base address for the timer device.
  static const uword_t TIMER_OFFSET = 0x0200;
  
  /// Number of bytes mapped to the timer device.
  static const uword_t TIMER_MAP_SIZE = 0x0018;

  /// Offset from IO base address for the  performance counters device.
  static const uword_t PERFCOUNTERS_OFFSET = 0x0300;
  
  /// Number of bytes mapped to the performance counters device.
  static const uword_t PERFCOUNTERS_MAP_SIZE = 0x0028;
  
  /// Offset from IO base address for UART device.
  static const uword_t UART_OFFSET = 0x0800;

  /// Number of bytes mapped to the UART device.
  static const uword_t UART_MAP_SIZE = 0x0008;

  /// Offset from IO base address for the LED device.
  static const uword_t LED_OFFSET = 0x0900;
  
  /// Number of bytes mapped to the LED device.
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
    bool is_word_access(uword_t address, uword_t size, uword_t offset);
    
    /// Read a word from memory pointer
    /// @param value the pointer to the data to read
    /// @param size the size of the value to read in bytes
    /// @return the read value
    uword_t get_word(byte_t *value, uword_t size);
    
    /// Write a word to a memory pointer
    /// @param value the pointer to the data to write
    /// @param size the size of the value to write in bytes
    /// @param data the word to write
    void set_word(byte_t *value, uword_t size, uword_t data);
    
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
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size) = 0;

    /// A simulated access to a read port. Does not update the device state or 
    /// simulate timing, just reads the value.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void peek(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      // By default, just return zero, this is primarily used for debugging and
      // should not assert if read is supported.
      set_word(value, size, 0);
    }

    /// Notify the device that a cycle has passed.
    virtual void tick(simulator_t &s) { }

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const { }

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options) { }
    
    /// Reset the statistics.
    virtual void reset_stats() { }
  };
  
  
  class cpuinfo_t : public mapped_device_t 
  {
    // The CPU ID
    uword_t Cpu_id;
    
    // The CPU frequency (Hz)
    uword_t Cpu_freq;
       
    // The number of cores
    uword_t Cpu_cnt;

  public:
    
    /// @param freq The CPU frequency in Mhz
  cpuinfo_t(uword_t base_address, uword_t cpuid, double freq, uword_t cpucnt)
    : mapped_device_t(base_address, CPUINFO_MAP_SIZE),
      Cpu_id(cpuid),
      Cpu_freq(freq * 1000000),
      Cpu_cnt(cpucnt)
    {}
    
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);
    
    virtual void peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);
  };

  class perfcounters_t : public mapped_device_t 
  {
  public:
    
  perfcounters_t(uword_t base_address)
    : mapped_device_t(base_address, PERFCOUNTERS_MAP_SIZE)
    {}
    
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);
  };
  
  class led_t : public mapped_device_t 
  {
    /// Stream to write LED status to
    std::ostream &Out_stream;
    
    uword_t Curr_state;
  public:
    led_t(uword_t base_address, std::ostream &os)
    : mapped_device_t(base_address, LED_MAP_SIZE), Out_stream(os),
      Curr_state(0) {}

    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);
  };
  
  /// Map several devices into the address space of another memory device
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
    mapped_device_t& find_device(uword_t address);
    
  public:
    /// Construct a new memory map.
    /// @param memory The memory onto which the devices are memory mapped.
    /// @param base_address The start address of the mapped address range.
    /// @param high_address The highest address of the mapped address range.
    memory_map_t(memory_t &memory, uword_t base_address, uword_t high_address) 
    : Memory(memory), Base_address(base_address), High_address(high_address) 
    {}

    void add_device(mapped_device_t &device);
    
    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Read some values from the memory -- DO NOT SIMULATE TIMING.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void read_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Write some values into the memory -- DO NOT SIMULATE TIMING, just write.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    virtual void write_peek(simulator_t &s, uword_t address, byte_t *value, uword_t size);

    /// Check if the memory is busy handling some request.
    /// @return False in case the memory is currently handling some request,
    /// otherwise true.
    virtual bool is_ready();

    /// Notify the memory that a cycle has passed.
    virtual void tick(simulator_t &s);

    /// Print the internal state of the memory to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const;

    /// Print statistics to an output stream.
    /// @param os The output stream to print to.
    virtual void print_stats(const simulator_t &s, std::ostream &os, 
                             const stats_options_t& options);
    
    virtual void reset_stats();
  };
}

#endif // PATMOS_MEMORY_MAP_H


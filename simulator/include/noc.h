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
// basic NOC interface.
//

#ifndef PATMOS_NOC_H
#define PATMOS_NOC_H

#include <istream>
#include <ostream>
#include <cstdio>

#include "memory-map.h"
#include "exception.h"

namespace patmos
{
    /// Default address of the NOC DMA
  static const uword_t NOC_BASE_ADDRESS  = 0xE0000000;

  /// Default address of the NOC routing info.
  static const uword_t NOC_DMA_P_OFFSET  = 0x01000000;

  /// Default address of the NOC slot table.
  static const uword_t NOC_DMA_ST_OFFSET = 0x02000000;

  /// Default address of the NOC SPM.
  static const uword_t NOC_SPM_OFFSET    = 0x08000000;

  /// Default size of the NOC SPM.
  static const uword_t NOC_SPM_SIZE      = 0x01000000;


  /// A simple NOC implementation.
  class noc_t : public mapped_device_t
  {
  private:
    memory_t &SPM;

    uword_t Base_address;
    uword_t Route_address;
    uword_t Slot_table_address;
    uword_t SPM_address;

  protected:

  public:
    /// Construct a new memory-mapped NOC.
    /// @param base_address The address of the NOC device
    /// @param in_stream Stream providing data read from the NOC.
    /// @param istty Flag indicating whether the input stream is a TTY.
    /// @param out_stream Stream storing data written to the NOC.
    noc_t(uword_t base_address, uword_t route_address, uword_t st_address,
           uword_t spm_address, uword_t spmsize, memory_t &spm) :
        mapped_device_t(base_address, spm_address+spmsize-base_address),
        Base_address(base_address), Route_address(route_address),
        Slot_table_address(st_address), SPM_address(spm_address),
        SPM(spm)
    {
    }

    /// A simulated access to a read port.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True when the data is available from the read port.
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      // TODO catch reads to DMA addresses
      if (address >= SPM_address)
        return SPM.read(s, address - SPM_address, value, size);
      else
        simulation_exception_t::unmapped(address);
      return true;
    }

    /// A simulated access to a read port. Does not update the device state or 
    /// simulate timing, just reads the value.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    virtual void peek(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      // TODO catch reads to DMA addresses
      if (address >= SPM_address)
        SPM.read_peek(s, address - SPM_address, value, size);
      else
        simulation_exception_t::unmapped(address);
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
    {
      // TODO catch writes to DMA addresses
      if (address >= SPM_address)
        return SPM.write(s, address - SPM_address, value, size);
      else
        simulation_exception_t::unmapped(address);
      return true;
    }
  };
}

#endif // PATMOS_NOC_H


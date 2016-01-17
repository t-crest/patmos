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
        return SPM.read(s, address - SPM_address, value, size, false);
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
        SPM.read_peek(s, address - SPM_address, value, size, false);
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


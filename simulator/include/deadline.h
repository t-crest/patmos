/*
   Copyright 2017 Technical University of Denmark, DTU Compute.
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
// Deadline device.
//
// Store a value N to the device.  In each tick the value is decremented.
// A load to the device will stall the processor for the number of ticks
// left in the device.
// The device can be used eg as multi-cycle NOP, or as PRET-like
// deadline instruction.
//

#ifndef PATMOS_DEADLINE_H
#define PATMOS_DEADLINE_H

#include <ostream>

#include "memory-map.h"

namespace patmos
{

  class deadline_t : public mapped_device_t
  {
  private:
    /// Writable counter
    uword_t Delay_counter;

  public:
    deadline_t(uword_t base_address)
    : mapped_device_t(base_address, DEADLINE_MAP_SIZE),
      Delay_counter(0)
    {
    }

    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        // we are finished as soon as the counter has reached 0
        if (Delay_counter == 0) {
          // read cycle counter
          set_word(value, size, Delay_counter);
          return true;
        }
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return false;
    }

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        // set the delay counter
        //  +1 to ignore the current tick (as in HW)
        Delay_counter = get_word(value, size) + 1;
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual void tick(simulator_t &s) {
      if (Delay_counter > 0) Delay_counter--;
    }
  };
}

#endif /* PATMOS_DEADLINE_H */

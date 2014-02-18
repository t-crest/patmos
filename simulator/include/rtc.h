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
// basic RTC interface for I/O.
//

#ifndef PATMOS_RTC_H
#define PATMOS_RTC_H

#include <istream>
#include <ostream>
#include <cstdio>

#include "memory-map.h"
#include "endian-conversion.h"
#include "interrupts.h"

namespace patmos
{

  class rtc_t : public mapped_device_t
  {
  private:
    simulator_t &Simulator;

    // Frequency of the CPU in MHz.
    double Frequency;
    
    /// Latched high word of clock counter
    uword_t High_clock;
    
    /// Latched high word of usec counter
    uword_t High_usec;

    /// Interrupt interval register value
    udword_t Interrupt_interval;

    /// Interrupt interval register value
    uword_t ISR;

  public:

    rtc_t(uword_t base_address, simulator_t &s, double frequency)
    : mapped_device_t(base_address, TIMER_MAP_SIZE),
      Simulator(s),
      Frequency(frequency), High_clock(0), High_usec(0),
      Interrupt_interval(std::numeric_limits<udword_t>::max())
    {
      Simulator.Rtc = this;
    }

    virtual bool read(uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        // read latched high word of cycle counter
        set_word(value, size, High_clock);
      }
      else if (is_word_access(address, size, 0x04)) {
        // read low word of cycle counter, latch high word
        uword_t low_clock = (uword_t)Simulator.Cycle;
        High_clock = (uword_t)(Simulator.Cycle >> 32);
        set_word(value, size, low_clock);
      }
      else if (is_word_access(address, size, 0x08)) {
        // read latched high word of usec
        set_word(value, size, High_usec);
      }
      else if (is_word_access(address, size, 0x0c)) {
        // read low word of usec, latch high word
        // TODO if Frequency == 0, use wall clock for usec
        uint64_t usec = (uint64_t)((double)Simulator.Cycle / Frequency);
        uword_t low_usec = (uword_t)usec;
        High_usec = (uword_t)(usec >> 32);
        set_word(value, size, low_usec);
      }
      else if (is_word_access(address, size, 0x10)) {
        // read current interrupt interval counter
        set_word(value, size, (uword_t)Interrupt_interval);
      }
      else if (is_word_access(address, size, 0x14)) {
        // read latched high word of usec
        set_word(value, size, ISR);
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual bool write(uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x10)) {
        Interrupt_interval = get_word(value, size);
      } 
      else if (is_word_access(address, size, 0x14)) {
        ISR = get_word(value, size);
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual void tick() {
      Interrupt_interval--;
      /// If interrupt interval reached 0 we fire an interrupt
      if (Interrupt_interval == 0) {
        Simulator.Interrupt_handler.fire_interrupt(interval, ISR);
        Interrupt_interval = std::numeric_limits<udword_t>::max();
      }
    }
  };
}

#endif /* PATMOS_RTC_H */

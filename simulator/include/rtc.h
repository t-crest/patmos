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
#include "excunit.h"
#include "exception.h"

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

    /// Remember the last usec value to trigger only when it changed
    uint64_t Last_usec;
    
    /// Latched low word of interrupt register value
    uword_t Low_interrupt_clock;
    
    /// Latched low word of usec interrupt register value
    uword_t Low_interrupt_usec;
    
    /// Cycle interrupt register value
    uint64_t Interrupt_clock;

    /// usec interrupt register value
    uint64_t Interrupt_usec;
    
  public:

    rtc_t(simulator_t &s, uword_t base_address, double frequency)
    : mapped_device_t(base_address, TIMER_MAP_SIZE),
      Simulator(s),
      Frequency(frequency), High_clock(0), High_usec(0),
      Last_usec(0), Low_interrupt_clock(0), Low_interrupt_usec(0),
      Interrupt_clock(std::numeric_limits<uint64_t>::max()), 
      Interrupt_usec(std::numeric_limits<uint64_t>::max())
    {
      Simulator.Rtc = this;
    }

    uint64_t getCycle() {
      return Simulator.Cycle;
    }
    
    uint64_t getUSec() {
      // TODO if Frequency == 0, use wall clock for usec
      return (uint64_t)((double)Simulator.Cycle / Frequency);
    }
    
    virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        // read latched high word of cycle counter
        set_word(value, size, High_clock);
      }
      else if (is_word_access(address, size, 0x04)) {
        // read low word of cycle counter, latch high word
        uint64_t cycle = getCycle();
        uword_t low_clock = (uword_t)cycle;
        High_clock = (uword_t)(cycle >> 32);
        set_word(value, size, low_clock);
      }
      else if (is_word_access(address, size, 0x08)) {
        // read latched high word of usec
        set_word(value, size, High_usec);
      }
      else if (is_word_access(address, size, 0x0c)) {
        // read low word of usec, latch high word
        uint64_t usec = getUSec();
        uword_t low_usec = (uword_t)usec;
        High_usec = (uword_t)(usec >> 32);
        set_word(value, size, low_usec);
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size) {
      if (is_word_access(address, size, 0x00)) {
        // set the clock interrupt timer
        uword_t high_clock = get_word(value, size);
        Interrupt_clock = ((uint64_t)high_clock)<<32 | Low_interrupt_clock;
      }
      else if (is_word_access(address, size, 0x04)) {
        // latch the low word of the cycle counter
        Low_interrupt_clock = get_word(value, size);
      }
      else if (is_word_access(address, size, 0x08)) {
        // set the usec interrupt timer
        uword_t high_usec = get_word(value, size);
        Interrupt_usec = ((uint64_t)high_usec)<<32 | Low_interrupt_usec;
      }
      else if (is_word_access(address, size, 0x0c)) {
        // latch the low word of the usec counter
        Low_interrupt_usec = get_word(value, size);
      }
      else {
        simulation_exception_t::unmapped(address);
      }
      return true;
    }

    virtual void tick() {
      if (Interrupt_clock == getCycle()) {
        Simulator.Exception_handler.fire_exception(ET_INTR_CLOCK);
      }
      uint64_t usec = getUSec();
      if (Interrupt_usec == usec && usec != Last_usec) {
        Simulator.Exception_handler.fire_exception(ET_INTR_USEC);
      }
      Last_usec = usec;
    }
  };
}

#endif /* PATMOS_RTC_H */

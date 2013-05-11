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

#ifndef PATMOS_MM_RTC_H
#define PATMOS_MM_RTC_H

#include <cstdio>
#include "endian-conversion.h"
#include "interrupts.h"
#include "rtc.h"

namespace patmos
{
  /// Default address of the RTC clock cycles low register.
  static const uword_t RTC_CLOCK_CYCLES_LOW_ADDRESS = 0xF0000300;

  /// Default address of the RTC clock cycles up register.
  static const uword_t RTC_CLOCK_CYCLES_UP_ADDRESS = 0xF0000304;

  /// Default address of the RTC microseconds cycles low register.
  static const uword_t RTC_MICROSECONDS_LOW_ADDRESS = 0xF0000308;

  /// Default address of the RTC microseconds cycles up register.
  static const uword_t RTC_MICROSECONDS_UP_ADDRESS = 0xF000030C;

  /// Default address of the RTC interrupt interval register.
  static const uword_t RTC_INTERRUPT_INTERVAL_ADDRESS = 0xF0000310;

  /// Default address of the timer ISR
  static const uword_t RTC_ISR_ADDRESS = 0xF0000314;

  /// A simple RTC implementation allowing memory-mapped I/O.
  class mm_rtc_t : public mapped_device_t
  {
  private:

    /// Address at which clock cycles lower 32 bits can be read from/written to
    const uword_t Clock_cycles_address_low;

    /// Address at which clock cycles uper 32 bits can be read from/written to
    const uword_t Clock_cycles_address_up;

    /// Address at which time in microseconds lower 32 bits can be read from/written to
    const uword_t Microseconds_address_low;

    /// Address at which time in microseconds uper 32 bits can be read from/written to
    const uword_t Microseconds_address_up;

    /// Address at which interrupt intervall can be read from/written to
    const uword_t Interrupt_interval_address;

    /// Address at which timer ISR can be set
    const uword_t ISR_address;

    /// Real time clock holding clock values
    rtc_t &Rtc;

  public:
    
    /// Construct a new memory-mapped RTC.
    /// @param memory The memory onto which the RTC is memory mapped.
    /// @param clock_cycles_address The address from/to which clock cycles can be read/written
    /// @param microseconds_address The address through which microseconds can be read/written
    /// @param interrupt_interval_address The address through which interrupt interval can be
    /// read/written
    mm_rtc_t(uword_t clock_cycles_address_low, 
          uword_t clock_cycles_address_up, 
          uword_t microseconds_address_low,
          uword_t microseconds_address_up,
          uword_t interrupt_interval_address,
          uword_t isr_address,
          rtc_t& rtc) :
        mapped_device_t(clock_cycles_address_low, 
                        isr_address - clock_cycles_address_low + 4), 
        Clock_cycles_address_low(clock_cycles_address_low),
        Clock_cycles_address_up(clock_cycles_address_up),
        Microseconds_address_low(microseconds_address_low),
        Microseconds_address_up(microseconds_address_up),
        Interrupt_interval_address(interrupt_interval_address),
        ISR_address(isr_address),
        Rtc(rtc)
    {

    }

    /// A simulated access to a read RTC.
    /// @param address The memory address to read from.
    /// @param value A pointer to a destination to store the value read from
    /// the memory.
    /// @param size The number of bytes to read.
    /// @return True 
    virtual bool read(uword_t address, byte_t *value, uword_t size)
    {
      if (address == Clock_cycles_address_low && size == 4)
        return Rtc.read_clock_cycles_low(value);
      if (address == Clock_cycles_address_up && size == 4)
        return Rtc.read_clock_cycles_up(value);
      else if (address == Microseconds_address_low && size == 4)
        return Rtc.read_microseconds_low(value);
      else if (address == Microseconds_address_up && size == 4)
        return Rtc.read_microseconds_up(value);
      else if (address == Interrupt_interval_address && size == 4)
        return Rtc.read_interrupt_interval(value);
      else if (address == ISR_address && size == 4)
        return Rtc.read_ISR(value);
      else
        simulation_exception_t::unmapped(address);
      assert(false && "never reached");
    }

    /// A simulated access to a write port.
    /// @param address The memory address to write to.
    /// @param value The value to be written to the memory.
    /// @param size The number of bytes to write.
    /// @return True when the data is written finally to the memory, false
    /// otherwise.
    virtual bool write(uword_t address, byte_t *value, uword_t size)
    {
      if (address == Clock_cycles_address_low && size == 4)
        return Rtc.write_clock_cycles_low(value);
      if (address == Clock_cycles_address_up && size == 4)
        return Rtc.write_clock_cycles_up(value);
      else if (address == Microseconds_address_low && size == 4)
        return Rtc.write_microseconds_low(value);
      else if (address == Microseconds_address_up && size == 4)
        return Rtc.write_microseconds_up(value);
      else if (address == Interrupt_interval_address && size == 4)
        return Rtc.write_interrupt_interval(value);
      else if (address == ISR_address && size == 4)
        return Rtc.write_ISR(value);
      else
        simulation_exception_t::unmapped(address);
      assert(false && "never reached");
    }
  };
}

#endif // PATMOS_MM_RTC_H

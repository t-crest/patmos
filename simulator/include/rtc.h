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
#include "endian-conversion.h"
#include "interrupts.h"

namespace patmos
{

  class rtc_t 
  {
  private:

    /// Clock cycles counter
    byte_t Clock_cycles[8];

    /// Microseconds cycles counter
    byte_t Microseconds[8];

    /// Interrupt interval register value
    byte_t Interrupt_interval[4];

    /// Interrupt interval register value
    byte_t ISR[4];

    /// Interrupt handler used to fire interrupts
    interrupt_handler_t &Interrupt_handler;  

  public:

    rtc_t(interrupt_handler_t &interrupt_handler) : Interrupt_handler(interrupt_handler)
    {
      Interrupt_interval[0] = 0xff;
      Interrupt_interval[1] = 0xff;
      Interrupt_interval[2] = 0xff;
      Interrupt_interval[3] = 0xff;
    }

    /// A simulated access to clock cycles low register.
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_clock_cycles_low(byte_t *value)
    {
      *value      = Clock_cycles[4];
      *(value+1)  = Clock_cycles[5];
      *(value+2)  = Clock_cycles[6];
      *(value+3)  = Clock_cycles[7];
      return true;
    }

    /// A simulated access to clock cycles up register.
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_clock_cycles_up(byte_t *value)
    {
      *value      = Clock_cycles[0];
      *(value+1)  = Clock_cycles[1];
      *(value+2)  = Clock_cycles[2];
      *(value+3)  = Clock_cycles[3];
      return true;
    }

    /// A simulated access to microseconds low register.
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_microseconds_low(byte_t *value)
    {
      *value      = Microseconds[4];
      *(value+1)  = Microseconds[5];
      *(value+2)  = Microseconds[6];
      *(value+3)  = Microseconds[7];
      return true;
    }

    /// A simulated access to microseconds up register.
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_microseconds_up(byte_t *value)
    {
      *value      = Microseconds[0];
      *(value+1)  = Microseconds[1];
      *(value+2)  = Microseconds[2];
      *(value+3)  = Microseconds[3];

      return true;
    }

    /// A simulated access to interrupt interval register.
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_interrupt_interval(byte_t *value)
    {
      *value      = Interrupt_interval[0];
      *(value+1)  = Interrupt_interval[1];
      *(value+2)  = Interrupt_interval[2];
      *(value+3)  = Interrupt_interval[3];
      return true;
    }

    /// A simulated access to ISR address register
    /// @param value A pointer to a destination to store the value read
    /// @return True   
    virtual bool read_ISR(byte_t *value)
    {
      *value      = ISR[0];
      *(value+1)  = ISR[1];
      *(value+2)  = ISR[2];
      *(value+3)  = ISR[3];
      return true;
    }

    /// A simulated access to clock cycles low register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_clock_cycles_low(byte_t *value)
    {
      Clock_cycles[4] = *value;
      Clock_cycles[5] = *(value+1);
      Clock_cycles[6] = *(value+2);
      Clock_cycles[7] = *(value+3);
      return true;
    }

    /// A simulated access to clock cycles up register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_clock_cycles_up(byte_t *value)
    {
      Clock_cycles[0] = *value;
      Clock_cycles[1] = *(value+1);
      Clock_cycles[2] = *(value+2);
      Clock_cycles[3] = *(value+3);
      return true;
    }

    /// A simulated access to microseconds low register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_microseconds_low(byte_t *value)
    {
      Microseconds[4] = *value;
      Microseconds[5] = *(value+1);
      Microseconds[6] = *(value+2);
      Microseconds[7] = *(value+3);           
      return true;
    }

    /// A simulated access to microseconds up register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_microseconds_up(byte_t *value)
    {
      Microseconds[0] = *value;
      Microseconds[1] = *(value+1);
      Microseconds[2] = *(value+2);
      Microseconds[3] = *(value+3);   
      return true;
    }

    /// A simulated access to interrupt interval register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_interrupt_interval(byte_t *value)
    {
      Interrupt_interval[0] = *value;
      Interrupt_interval[1] = *(value+1);
      Interrupt_interval[2] = *(value+2);
      Interrupt_interval[3] = *(value+3);
      return true;
    }

    /// A simulated access to ISR address register.
    /// @param value A pointer to the value to set
    /// @return True   
    virtual bool write_ISR(byte_t *value)
    {
      ISR[0] = *value;
      ISR[1] = *(value+1);
      ISR[2] = *(value+2);
      ISR[3] = *(value+3);
      return true;
    }

    virtual void tick() {

      udword_t Clock_cycles_small                 = (udword_t)from_big_endian<big_dword_t>(
                                                    *reinterpret_cast<udword_t*>(Clock_cycles));
      Clock_cycles_small                          += 1;
      *reinterpret_cast<udword_t*>(Clock_cycles)  = to_big_endian<big_dword_t>(Clock_cycles_small);

      udword_t Microseconds_small                 = (udword_t)from_big_endian<big_dword_t>(
                                                    *reinterpret_cast<udword_t*>(Microseconds));
      Microseconds_small                          += 1;
      *reinterpret_cast<udword_t*>(Microseconds)  = to_big_endian<big_dword_t>(Microseconds_small);


      uword_t Interrupt_interval_small          = (uword_t)from_big_endian<big_uword_t>(
                                                  *reinterpret_cast<uword_t*>(Interrupt_interval));
      Interrupt_interval_small                  -= 1;
      *reinterpret_cast<uword_t*>(Interrupt_interval)  
                                                = to_big_endian<big_uword_t>(Interrupt_interval_small);

      /// If interrupt interval reached 0 we fire an interrupt
      if (Interrupt_interval_small == 0) {

        uword_t ISR_small              = (uword_t)from_big_endian<big_uword_t>(
                                                  *reinterpret_cast<uword_t*>(ISR));
        Interrupt_handler.fire_interrupt(interval, ISR_small);
       
        Interrupt_interval[0] = 0xff;
        Interrupt_interval[1] = 0xff;
        Interrupt_interval[2] = 0xff;
        Interrupt_interval[3] = 0xff;
      }
    }
  };
}

#endif /* PATMOS_RTC_H */
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
// Interrupt handler for patmos
//

#include "excunit.h"

#include "simulation-core.h"
#include "exception.h"

#include <cassert>
#include <iostream>

namespace patmos
{

  excunit_t::excunit_t(uword_t base_address) 
  : mapped_device_t(base_address, EXCUNIT_MAP_SIZE), 
    Enable_interrupts(true), Status(0), Mask(0), Pending(0), Source(0) 
  {
    for (int i = 0; i < NUM_EXCEPTIONS; i++) {
      Exception_vector[i] = NO_ISR_ADDR;
    }
  }

  bool excunit_t::pending() 
  {
    return Enable_interrupts && (Status & 0x1) && (Pending & Mask) != 0;
  }

  exception_t excunit_t::next() 
  {
    assert(pending() && "No interrupt pending when calling next()");
    
    // Determine the next interrupt to handle, set Source
    uword_t waiting = Pending & Mask;
    Source = 0;
    // TODO better way of doing ctz?
    while ((waiting & 0x1) == 0) {
      waiting >>= 1;
      Source += 1;
    }
    
    // Clear the pending flag for the interrupt
    Pending &= ~(1u << Source);
    
    // Return the interrupt data
    return get((exception_e)Source);
  }

  bool excunit_t::trap(exception_e exc, exception_t &isr) {
    if (!enabled(exc)) {
      return false;
    }
    
    // Start executing the trap by setting Source to the trap and returning the
    // ISR address
    Source = exc;
    
    isr = get(exc);
    
    return true;
  }
  
  void excunit_t::resume()
  {
    // TODO update Source?
    Status >>= 1;
  }
  
  exception_t excunit_t::get(exception_e exc) const 
  {
    exception_t intr(exc, Exception_vector[exc]);
    return intr;
  }
  
  bool excunit_t::read(simulator_t &s, uword_t address, byte_t *value, uword_t size)
  {
    if (is_word_access(address, size, 0x00)) {
      set_word(value, size, Status);
    }
    else if (is_word_access(address, size, 0x04)) {
      set_word(value, size, Mask);
    }
    else if (is_word_access(address, size, 0x08)) {
      set_word(value, size, Pending);
    }
    else if (is_word_access(address, size, 0x0c)) {
      set_word(value, size, Source);
    }
    else if (address >= Base_address+0x80 && address < Base_address+0x100) {
      int intr_addr = address & 0xFC;
      if (!is_word_access(address, size, intr_addr)) {
        simulation_exception_t::unaligned(address);
      } else {
        int intr = (intr_addr - 0x80) >> 2;
        set_word(value, size, Exception_vector[intr]);
      }
    }
    else {
      simulation_exception_t::unmapped(address);
    }
    return true;
  }

  bool excunit_t::write(simulator_t &s, uword_t address, byte_t *value, uword_t size)
  {
    if (is_word_access(address, size, 0x00)) {
      Status = get_word(value, size);
    }
    else if (is_word_access(address, size, 0x04)) {
      Mask = get_word(value, size);
    }
    else if (is_word_access(address, size, 0x08)) {
      Pending = get_word(value, size);
    }
    else if (is_word_access(address, size, 0x0c)) {
      simulation_exception_t::illegal_access(address);
    }
    else if (is_word_access(address, size, 0x10)) {
      // ignore sleep mode
    }
    else if (address >= Base_address+0x80 && address < Base_address+0x100) {
      int intr_addr = address & 0xFC;
      if (!is_word_access(address, size, intr_addr)) {
        simulation_exception_t::unaligned(address);
      } else {
        int intr = (intr_addr - 0x80) >> 2;
        Exception_vector[intr] = get_word(value, size);
      }
    }
    else {
      simulation_exception_t::unmapped(address);
    }
    return true;
  }

  void excunit_t::tick()
  {
  }
  
  void excunit_t::enable_interrupts(bool enable) 
  {
    Enable_interrupts = enable;
  }

  bool excunit_t::may_fire(exception_e exctype)
  {
    return Enable_interrupts && Exception_vector[(int)exctype] != NO_ISR_ADDR;
  }
  
  bool excunit_t::enabled(exception_e exctype)
  {
    // TODO check if an ISR has been installed as well?
    return ((1u<<(int)exctype) & Mask);
  }
  
  void excunit_t::fire_exception(exception_e exctype)
  {
    Pending |= (1u<<(int)exctype);
  }
  
  void excunit_t::illegal(uword_t iw)
  {
    if (may_fire(ET_ILLEGAL_OPERATION)) {
      fire_exception(ET_ILLEGAL_OPERATION);
    } else {
      simulation_exception_t::illegal(iw);
    }
  }

  void excunit_t::illegal(std::string msg)
  {
    if (may_fire(ET_ILLEGAL_OPERATION)) {
      fire_exception(ET_ILLEGAL_OPERATION);
    } else {
      simulation_exception_t::illegal(msg);
    }
  }
  
  void excunit_t::unmapped(uword_t address)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::unmapped(address);
    }
  }

  void excunit_t::illegal_access(uword_t address)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::illegal_access(address);
    }
  }
  
  void excunit_t::stack_exceeded(std::string msg)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::stack_exceeded(msg);
    }
  }

  void excunit_t::code_exceeded(uword_t address)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::code_exceeded(address);
    }
  }

  void excunit_t::illegal_pc(uword_t address)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::illegal_pc(address);
    }
  }

  void excunit_t::illegal_pc(std::string msg)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::illegal_pc(msg);
    }
  }
  
  void excunit_t::unaligned(uword_t address)
  {
    if (may_fire(ET_ILLEGAL_ADDRESS)) {
      fire_exception(ET_ILLEGAL_ADDRESS);
    } else {
      simulation_exception_t::unaligned(address);
    }
  }
}

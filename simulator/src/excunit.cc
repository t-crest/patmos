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
// Interrupt handler for patmos
//

#include "excunit.h"

#include "simulation-core.h"
#include "exception.h"
#include "data-cache.h"
#include "instr-cache.h"

#include <cassert>
#include <iostream>

namespace patmos
{

  excunit_t::excunit_t(uword_t base_address) 
  : mapped_device_t(base_address, EXCUNIT_MAP_SIZE), 
    Enable_interrupts(true), Enable_debug(false), 
    Status(0), Mask(0), Pending(0), Source(0)
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
    
    // Disable interrupts
    Status <<= 1;
    
    if (Enable_debug) {
      std::cerr << "*** EXC: Execute ISR " << Source 
                << " (status: 0x" << std::hex << Status
                << ", pending: 0x" << Pending << std::dec << ")\n";
    }
    
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
    
    // Disable interrupts during trap handler
    Status <<= 1;
    
    if (Enable_debug) {
      std::cerr << "*** EXC: Execute trap " << exc 
                << " (status: 0x" << std::hex << Status
                << ", pending: 0x" << Pending << std::dec << ")\n";
    }
    
    isr = get(exc);
    
    return true;
  }
  
  void excunit_t::resume()
  {
    // TODO update Source?
    Status >>= 1;
    
    if (Enable_debug) {
      std::cerr << "*** EXC: Return (status: 0x" << std::hex << Status
                << ", pending: 0x" << Pending << std::dec << ")\n";
    }
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
      if (Enable_debug) {
        std::cerr << "*** EXC: Write Status (status: 0x" << std::hex << Status
                  << ", pending: 0x" << Pending << std::dec << ")\n";
      }
    }
    else if (is_word_access(address, size, 0x04)) {
      Mask = get_word(value, size);
      if (Enable_debug) {
        std::cerr << "*** EXC: Write Mask (status: 0x" << std::hex << Status
                  << ", pending: 0x" << Pending << std::dec << ")\n";
      }
    }
    else if (is_word_access(address, size, 0x08)) {
      Pending = get_word(value, size);
      if (Enable_debug) {
        std::cerr << "*** EXC: Write Pending (status: 0x" << std::hex << Status
                  << ", pending: 0x" << Pending << std::dec << ")\n";
      }
    }
    else if (is_word_access(address, size, 0x0c)) {
      simulation_exception_t::illegal_access(address);
    }
    else if (is_word_access(address, size, 0x10)) {
      // ignore sleep mode
    }
    else if (is_word_access(address, size, 0x14)) {
      word_t Flags = get_word(value, size);
      if (Flags & 0x01) {
        s.Data_cache.flush_cache();
        s.Stack_data_cache.flush_cache();
      }
      if (Flags & 0x02) {
        s.Instr_cache.flush_cache();
      }
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

  void excunit_t::enable_debug(bool debug) 
  {
    Enable_debug = debug;
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
    
    if (Enable_debug) {
      std::cerr << "*** EXC: Fire ISR " << exctype << " (status: 0x" << std::hex << Status
                << ", pending: 0x" << Pending << std::dec << ")\n";
    }
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

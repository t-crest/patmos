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

#include "interrupts.h"
#include "simulation-core.h"
#include "instruction.h"
#include "memory.h"
#include "method-cache.h"
#include <stdio.h>

namespace patmos
{

	interrupt_handler_t::interrupt_handler_t() : Interrupt_status(INTERRUPT_DISABLED)
  {

  }

  bool interrupt_handler_t::interrupt_pending() 
  {
    return (!Interrupt_vector.empty());
  }

  interrupt_t& interrupt_handler_t::get_interrupt() 
  {
    interrupt_t& interrupt = Interrupt_vector.front();
    Interrupt_vector.pop_front();
    return interrupt;
  }

  void interrupt_handler_t::enable_interrupts() 
  {
  	Interrupt_status 			= INTERRUPT_ENABLED;
  }

  void interrupt_handler_t::disable_interrupts()
  {
  	Interrupt_status 			= INTERRUPT_DISABLED;
  }

  void interrupt_handler_t::fire_interrupt(interrupt_e interrupt_type, ISR_address interrupt_address)
  {
    // the address must be given in words not in bytes
  	interrupt_t interrupt(interrupt_type, interrupt_address/4);
  	Interrupt_vector.push_back(interrupt);
  }

}
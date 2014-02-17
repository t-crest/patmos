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
// Definition of simulation functions for every Patmos instruction.
//

#ifndef PATMOS_INTERRUPTS_H
#define PATMOS_INTERRUPTS_H

#include <deque>

#include "basic-types.h"

namespace patmos 
{

  /// Forward definition for class simulator_t
  class simulator_t;

  /// Enumeration for interrupt types
  enum interrupt_e
  {
    interval,
    NUM_INTERRUPTS
  };

  /// Type for the address of an Interrupt Service Routine
  typedef word_t ISR_address;

  /// Structure wrapping all information needed 
  /// to handle an interrupt
  struct interrupt_t {
    // interrupt identifier
    interrupt_e Type;
    // ISR address for the interrupt
    ISR_address	Address;
    // interrupt constructor
    interrupt_t(interrupt_e mType, ISR_address mAddress) : Type(mType), Address(mAddress)
    {
    }
  };

  /// Class responsible for collecting fired interrupts
  class interrupt_handler_t 
  {
    private:

      /// Static constant variables identifying interrupt activation state
      static const int INTERRUPT_ENABLED  = 1;
      static const int INTERRUPT_DISABLED = 0;

      /// Interrupt status (ENABLED/DISABLED)
      int Interrupt_status;

      /// Vector of the pending interrupts
      std::deque<interrupt_t> Interrupt_vector;

    public:

      /// Empty constructor, disables interrupts
      interrupt_handler_t         ();

      /// Returns true if an interrupt is pending
      bool interrupt_pending      ();

      /// Gets the next enqued interrupt
      interrupt_t& get_interrupt  ();

      /// Enables interrupts
      void enable_interrupts      ();

      /// Disables interrupts
      void disable_interrupts     ();

      /// Adds a new interrupt
      void fire_interrupt(interrupt_e interrupt_type, ISR_address interrupt_address);
  };
}

#endif // PATMOS_INTERRUPTS_H
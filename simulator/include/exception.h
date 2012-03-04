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
// Interface to signal exceptions during simulation.
//

#ifndef PATMOS_EXCEPTION_H
#define PATMOS_EXCEPTION_H

#include "basic-types.h"

#include <cassert>

namespace patmos
{
  /// Signal exceptions during simulation, i.e., an illegal instruction, et
  /// cetera. An exception usually causes the simulation to abort.
  class simulation_exception_t
  {
  public:
    /// Kinds of simulation exceptions.
    enum kind_t
    {
      /// A halt instruction was encountered.
      HALT,

      /// An illegal instruction has been encountered.
      ILLEGAL,

      /// An unmapped memory region has been accessed.
      UNMAPPED,

      /// A stack operation exceeded the stack size.
      STACKEXCEEDED
    };

  private:
    /// The kind of the simulation exception.
    kind_t Kind;

    /// Additional information on the exception, e.g., the address of an
    /// unmapped memory access, et cetera.
    word_t Info;

    /// Construction a simulation exception.
    /// @param kind The kind of the simulation exception.
    /// @param info Additional information on the simulation exception, e.g.,
    /// the address of an unmapped memory access, et cetera.
    simulation_exception_t(kind_t kind, word_t info = 0) :
        Kind(kind), Info(info)
    {
    }
  public:
    /// Return the kind of the simulation exception.
    /// @return The kind of the simulation exception.
    kind_t get_kind() const
    {
      return Kind;
    }

    /// Return additional information on the simulation exception.
    /// @return The kind additional information on the simulation exception.
    word_t get_info() const
    {
      assert(Kind != HALT && Kind != STACKEXCEEDED);
      return Info;
    }

    /// Throw a halt simulation exception.
    static void halt()
    {
      throw simulation_exception_t(HALT);
    }

    /// Throw an illegal instruction simulation exception.
    /// @param iw The illegal instruction word.
    static void illegal(word_t iw)
    {
      throw simulation_exception_t(ILLEGAL, iw);
    }

    /// Throw an unmapped address simulation exception.
    /// @param address The unmapped address.
    static void unmapped(word_t address)
    {
      throw simulation_exception_t(UNMAPPED, address);
    }

    /// Throw a stack-cache-size-exceeded simulation exception.
    static void stackexceeded()
    {
      throw simulation_exception_t(STACKEXCEEDED);
    }
  };
}

#endif // PATMOS_EXCEPTION_H


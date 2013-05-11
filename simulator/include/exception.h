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
  // forward declarations
  class simulator_t;

  /// Signal exceptions during simulation, i.e., an illegal instruction, et
  /// cetera. An exception usually causes the simulation to abort.
  class simulation_exception_t
  {
    friend class simulator_t;
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

      /// An illegal memory address has been accessed.
      ILLEGAL_ACCESS,
      
      /// A stack operation exceeded the stack size.
      STACK_EXCEEDED,

      /// A method exceeds the size of the method cache.
      CODE_EXCEEDED,

      /// Program counter outside of current method.
      ILLEGAL_PC,

      /// An unaligned memory access has been encountered.
      UNALIGNED
    };

  private:
    /// The kind of the simulation exception.
    kind_t Kind;

    /// Additional information on the exception, e.g., the address of an
    /// unmapped memory access, et cetera.
    uword_t Info;

    /// The value of the program counter when the exception was raised.
    uword_t PC;

    /// The value of the cycle counter when the exception was raised.
    unsigned int Cycle;

    /// Construction a simulation exception.
    /// @param kind The kind of the simulation exception.
    /// @param info Additional information on the simulation exception, e.g.,
    /// the address of an unmapped memory access, et cetera.
    simulation_exception_t(kind_t kind, uword_t info = 0, uword_t pc = 0,
                           unsigned int cycle = 0) :
        Kind(kind), Info(info), PC(pc), Cycle(cycle)
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
    uword_t get_info() const
    {
      return Info;
    }

    /// Return the value of the program counter when the exception was raised.
    /// @return The value of the program counter (PC) when the exception was
    /// raised.
    uword_t get_pc() const
    {
      return PC;
    }

    /// Return the value of the cycle counter when the exception was raised.
    /// @return The value of the cycle counter when the exception was raised.
    unsigned int get_cycle() const
    {
      return Cycle;
    }

    /// Throw a halt simulation exception.
    /// @param exit_code The exit code signaled by the simulated program, i.e.,
    /// the value of register r1 before terminating.
    static void halt(int exit_code) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(HALT, exit_code);
    }

    /// Throw an illegal instruction simulation exception.
    /// @param iw The illegal instruction word.
    static void illegal(uword_t iw) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL, iw);
    }

    /// Throw an unmapped address simulation exception.
    /// @param address The unmapped address.
    static void unmapped(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(UNMAPPED, address);
    }

    /// Throw an illegal access simulation exception.
    /// @param address The unmapped address.
    static void illegal_access(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL_ACCESS, address);
    }
    
    /// Throw a stack-cache-size-exceeded simulation exception.
    static void stack_exceeded()  __attribute__ ((noreturn))
    {
      throw simulation_exception_t(STACK_EXCEEDED);
    }

    /// Throw a method-cache-size-exceeded simulation exception.
    static void code_exceeded(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(CODE_EXCEEDED, address);
    }

    /// Thow a PC-outsize-method simulation exception.
    static void illegal_pc(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL_PC, address);
    }

    /// Throw a unaligned simulation exception.
    static void unaligned(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(UNALIGNED, address);
    }
  };
}

#endif // PATMOS_EXCEPTION_H


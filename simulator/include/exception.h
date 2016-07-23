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
// Interface to signal exceptions during simulation.
//

#ifndef PATMOS_EXCEPTION_H
#define PATMOS_EXCEPTION_H

#include "basic-types.h"
#include "symbol.h"

#include <cassert>
#include <sstream>

#include <boost/format.hpp>

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
    
    /// An optional error message.
    std::string Message;

    /// Construction a simulation exception.
    /// @param kind The kind of the simulation exception.
    /// @param info Additional information on the simulation exception, e.g.,
    /// the address of an unmapped memory access, et cetera.
    simulation_exception_t(kind_t kind, uword_t info, uword_t pc = 0,
                           unsigned int cycle = 0) :
        Kind(kind), Info(info), PC(pc), Cycle(cycle), Message("")
    {}
    
    simulation_exception_t(kind_t kind, std::string msg, uword_t pc = 0, 
                           unsigned int cycle = 0) :
        Kind(kind), Info(0), PC(pc), Cycle(cycle), Message(msg)
    {}

  public:
    void set_cycle(unsigned int cycle, uword_t pc) 
    {
      Cycle = cycle;
      PC = pc;
    }
    
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
    
    std::string get_message() const
    {
      if (Message.empty()) {
        std::stringstream ss;
        ss << boost::format("%1$08x") % Info;
        return ss.str();
      } else {
        return Message;
      }
    }
    
    std::string to_string(symbol_map_t &sym) const {
      std::string kind_msg;
      switch (Kind) {
        case CODE_EXCEEDED:
          kind_msg = "Method cache size exceeded";
          break;
        case ILLEGAL_PC:
          kind_msg = "Program counter outside current method";
          break;
        case STACK_EXCEEDED:
          kind_msg = "Stack size exceeded";
          break;
        case UNMAPPED:
          kind_msg = "Unmapped memory access";
          break;
        case ILLEGAL_ACCESS:
          kind_msg = "Illegal memory access";
          break;
        case ILLEGAL:
          kind_msg = "Illegal instruction";
          break;
        case UNALIGNED:
          kind_msg = "Unaligned memory access";
          break;
        case HALT:
          kind_msg = "Halt called";
          break;
        default:
          return "Unknown simulation error: " + get_message() + "\n";
      }
      std::stringstream ss;
      ss << boost::format("Cycle %1%: %2% at %3$08x%4%: %5%\n")
                    % Cycle % kind_msg % PC % sym.find(PC) % get_message();
      return ss.str();
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

    /// Throw an illegal instruction simulation exception.
    /// @param msg The error message
    static void illegal(std::string msg) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL, msg);
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
    
    /// Throw an illegal access simulation exception.
    /// @param address The unmapped address.
    static void illegal_access(std::string msg) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL_ACCESS, msg);
    }
    
    /// Throw a stack-cache-size-exceeded simulation exception.
    static void stack_exceeded(std::string msg)  __attribute__ ((noreturn))
    {
      throw simulation_exception_t(STACK_EXCEEDED, msg);
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

    /// Thow a PC-outsize-method simulation exception.
    static void illegal_pc(std::string msg) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(ILLEGAL_PC, msg);
    }
    
    /// Throw a unaligned simulation exception.
    static void unaligned(uword_t address) __attribute__ ((noreturn))
    {
      throw simulation_exception_t(UNALIGNED, address);
    }
  };
}

#endif // PATMOS_EXCEPTION_H


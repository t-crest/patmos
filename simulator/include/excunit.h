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
// Exception unit of Patmos.
//

#ifndef PATMOS_EXCUNIT_H
#define PATMOS_EXCUNIT_H

#include "basic-types.h"
#include "memory-map.h"

namespace patmos 
{

  /// Enumeration for interrupt types
  enum exception_e
  {
    ET_ILLEGAL_OPERATION,
    ET_ILLEGAL_ADDRESS,
    ET_EXCEPTION_2,
    ET_EXCEPTION_3,
    ET_EXCEPTION_4,
    ET_EXCEPTION_5,
    ET_EXCEPTION_6,
    ET_EXCEPTION_7,
    ET_EXCEPTION_8,
    ET_EXCEPTION_9,
    ET_EXCEPTION_10,
    ET_EXCEPTION_11,
    ET_EXCEPTION_12,
    ET_EXCEPTION_13,
    ET_EXCEPTION_14,
    ET_EXCEPTION_15,
    ET_INTR_CLOCK,
    ET_INTR_USEC,
    ET_INTR_2,
    ET_INTR_3,
    ET_INTR_4,
    ET_INTR_5,
    ET_INTR_6,
    ET_INTR_7,
    ET_INTR_8,
    ET_INTR_9,
    ET_INTR_10,
    ET_INTR_11,
    ET_INTR_12,
    ET_INTR_13,
    ET_INTR_14,
    ET_INTR_15,    
    NUM_EXCEPTIONS
  };

  /// Structure wrapping all information needed 
  /// to handle an exception
  struct exception_t {
    // interrupt identifier
    exception_e Type;
    
    // ISR address for the interrupt
    uword_t Address;
    
    exception_t()
    : Type(ET_ILLEGAL_OPERATION), Address(0)
    {}
    
    // interrupt constructor
    exception_t(exception_e mType, uword_t mAddress) 
    : Type(mType), Address(mAddress)
    {}
  };

  /// Implementation of the Patmos exception handling unit
  class excunit_t : public mapped_device_t
  {
    private:

      static const uword_t NO_ISR_ADDR = ~0u;
      
      // Enable interrupts
      bool Enable_interrupts;
      
      // Print out status changes
      bool Enable_debug;
      
      // Status flags for enabling interrupts
      uword_t Status;
      
      // Mask of enabled interrupts
      uword_t Mask;
      
      // Pending interrupts mask
      uword_t Pending;
      
      // Next interrupt that is going to be served (-1 if no pending interrupts)
      uword_t Source;
      
      /// Vector of ISR addresses
      uword_t Exception_vector[NUM_EXCEPTIONS];

    public:

      /// Empty constructor, disables interrupts
      excunit_t(uword_t base_address);

      /// Check if we should handle an interrupt or exception via an ISR.
      bool may_fire(exception_e exc);
      
      /// Check if an exception vector has been enabled, i.e. its mask bit is set.
      /// Does not check if throwing interrupts has been disabled.
      bool enabled(exception_e exc);
      
      /// Returns true if we are in privileged mode
      bool privileged();

      /// Returns true if an interrupt is pending
      bool pending();

      /// Gets the next enqued interrupt
      exception_t next();
      
      /// Check if we can trap the given exception, and return the ISR address
      /// if true. This sets up Source to the exception number, but does not
      /// call the exception handler.
      bool trap(exception_e exc, exception_t &isr);

      /// Callback when an interrupt handler has finished (i.e., on xret)
      void resume();

      /// Get the ISR address for an exception entry. Does not modify the state
      exception_t get(exception_e exc) const;
      
      virtual bool read(simulator_t &s, uword_t address, byte_t *value, uword_t size);
      
      virtual bool write(simulator_t &s, uword_t address, byte_t *value, uword_t size);
      
      virtual void tick(simulator_t &s);
      
      /// Enables firing of interupts and exception handler ISRs. Does not disable
      /// traps.
      void enable_interrupts(bool enabled);

      /// Enable debug printing
      void enable_debug(bool debug);
      
      /// Make an exception pending. Never throws a simulator-exception directly.
      /// If the exception unit is disabled, the exception is effectively ignored.
      /// If the exception unit is enabled but the ISR is not installed,
      /// an illegal PC exception is triggered later.
      /// To throw a fault, use one of the specialized functions of this class.
      void fire_exception(exception_e exctype);      

      
      /// Throw an illegal instruction exception.
      /// @param iw The instruction word
      void illegal(uword_t iw);

      /// Throw an illegal instruction exception.
      /// @param msg The error message
      void illegal(std::string msg);
      
      /// Throw an unmapped address exception.
      /// @param address The unmapped address.
      void unmapped(uword_t address);

      /// Throw an illegal access exception.
      /// @param address The unmapped address.
      void illegal_access(uword_t address);
      
      /// Throw a stack-cache-size-exceeded exception.
      void stack_exceeded(std::string msg);

      /// Throw a method-cache-size-exceeded exception.
      void code_exceeded(uword_t address);

      /// Thow a PC-outsize-method exception.
      void illegal_pc(uword_t address);

      /// Thow a PC-outsize-method exception.
      void illegal_pc(std::string msg);
      
      /// Throw a unaligned exception.
      void unaligned(uword_t address);

  };
}

#endif // PATMOS_INTERRUPTS_H

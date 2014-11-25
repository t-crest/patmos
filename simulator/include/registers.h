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
// Classes and definitions for register files and register by-passes.
//

#ifndef PATMOS_REGISTERS_H
#define PATMOS_REGISTERS_H

#include "basic-types.h"

#include <cassert>

namespace patmos
{
  /// Represent a register operand.
  /// The template argument D represents the data type to represent the register
  /// files content, while I represents the data type to index into the register
  /// file.
  template<typename I, typename D>
  class register_operand_t
  {
  private:
    /// The register operand's index.
    I Index;

    /// The register operand's value.
    D Value;
  public:
    /// Construct a register operand.
    /// @param index The register operand's index.
    /// @param value The register operand's value.
    register_operand_t(I index = (I)0, D value = 0) :
        Index(index), Value(value)
    {
    }

    /// Get the register operand's value.
    /// @return The register operand's value.
    D get() const
    {
      return Value;
    }

    /// Get the register operand's index.
    /// @return The register operand's index.
    I get_index() const
    {
      return Index;
    }
  };

  /// The content of a register file.
  /// The template argument D represents the data type to represent the register
  /// files content, while I represents the data type to index into the register
  /// file. S represents the size of the register file.
  template<typename I, typename D, unsigned int S>
  class register_file_t
  {
  protected:
    /// The content of the register file.
    D *Content;
  public:
    /// Construct a register file.
    register_file_t()
    {
      Content = new D[S];

      for(unsigned int i = 0; i < S; i++)
      {
        Content[i] = 0;
      }
    }

    /// Read a value from the register file.
    /// @param index The index of the register to be read.
    register_operand_t<I, D> get(I index) const
    {
      assert(index < S);
      return register_operand_t<I, D>(index, Content[index]);
    }

    /// Set the value of a register in the register file.
    /// @param operand The register operand carrying the index and value of the
    /// register to be written.
    void set(const register_operand_t<I, D> &operand)
    {
      set(operand.get_index(), operand.get());
    }

    /// Set the value of a register in the register file.
    /// @param index The index of the register to be written.
    /// @param value The new value to be written.
    virtual void set(I index, D value) =0;

    /// Destruct the register file and free its content.
    ~register_file_t()
    {
      assert(Content);
      delete[] Content;
    }
  };

  /// A provider for register by-passing
  template<typename I, typename D>
  class by_pass_t
  {
  private:
    /// A flag indicating whether the by-passing value should be considered.
    bool Active;

    /// The register operand to be by-passed.
    register_operand_t<I, D> Operand;
  public:
    /// Construct an inactive by-pass.
    by_pass_t() : Active(false)
    {
    }

    /// Get the value stored at an active by-pass and return its value, in case
    /// the by-pass is inactive, the supplied initial value is returned.
    /// @param initial The value
    /// @return The value stored at an active by-pass and return its value, in
    /// case the by-pass is inactive, the supplied initial value is returned.
    register_operand_t<I, D> get(register_operand_t<I, D> initial) const
    {
      if (Active && initial.get_index() == Operand.get_index())
      {
        return Operand;
      }
      else
      {
        return initial;
      }
    }

    /// Read the value from the register by-pass.
    /// @return Get the value from the by-bypass -- assert that the by-pass is
    /// active.
    register_operand_t<I, D> get() const
    {
      assert(Active);
      return Operand;
    }

    /// Assign a register operand to the by-pass and mark the by-pass as active.
    /// @param operand The register operand to assign.
    void set(const register_operand_t<I, D> &operand)
    {
      assert(!Active);
      Active = true;
      Operand = operand;
    }

    /// Assign a register operand to the by-pass and mark the by-pass as active.
    /// @param index The index of the register to be written.
    /// @param value The new value to be written.
    void set(I index, D value)
    {
      register_operand_t<I, D> tmp(index, value);
      set(tmp);
    }

    /// Reset the by-pass and mark it as inactive.
    void reset()
    {
      Active = false;
    }
  };

  /// Symbols representing the general purpose registers.
  enum GPR_e
  {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15,
    r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30,
    r31,
    NUM_GPR,
    rtr = r27, // Temporary register
    rfp = r28, // Frame pointer
    rsp = r29, // Stack Pointer
    rfb = r30, // Return info: Function base
    rfo = r31  // Return info: Funcion offset
  };

  /// Symbols representing the predicate registers.
  enum PRR_e
  {
    p0, p1, p2, p3, p4, p5, p6, p7,
    pn0, pn1, pn2, pn3, pn4, pn5, pn6, pn7,
    NUM_PRRn,
    NUM_PRR = pn0
  };

  /// Symbols representing the special purpose registers.
  enum SPR_e
  {
    s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15,
    NUM_SPR,
    sp = s0,
    sl = s2,
    sh = s3,
    ss = s5,
    st = s6,
    srb = s7,
    sro = s8,
    sxb = s9,
    sxo = s10
  };

  /// A register file for the general purpose registers.
  class GPR_t : public register_file_t<GPR_e, word_t, NUM_GPR>
  {
  public:
    void set(GPR_e index, word_t value)
    {
      if (index == r0) return;
      assert(index < NUM_GPR);
      Content[index] = value;
    }
  };

  /// A register file for the predicate registers.
  class PRR_t : public register_file_t<PRR_e, bit_t, NUM_PRRn>
  {
  public:
    PRR_t() : register_file_t<PRR_e, bit_t, NUM_PRRn>()
    {
      Content[p0] = true;
      Content[pn0] = false;

      // initialize the negative registers
      for (unsigned int p = pn1; p < NUM_PRRn; p++) {
        Content[p] = true;
      }
    }

    void set(PRR_e index, bit_t value)
    {
      if (index == p0 || index == pn0) return;
      assert(index < NUM_PRRn);
      Content[index] = value;
      // update the negative value as well
      if (index < NUM_PRR) {
        Content[NUM_PRR + index] = !value;
      } else {
        Content[index - NUM_PRR] = !value;
      }
    }
  };

  /// A register file for the special purpose registers.
  class SPR_t : public register_file_t<SPR_e, word_t, NUM_SPR>
  {
  public:
    void set(SPR_e index, word_t value)
    {
      assert(index < NUM_SPR);
      Content[index] = value;
    }
  };


  /// A register by-pass for general purpose registers
  typedef by_pass_t<GPR_e, word_t> GPR_by_pass_t;


  /// A general purpose register operand.
  typedef register_operand_t<GPR_e, word_t> GPR_op_t;

  /// A predicate register operand.
  typedef register_operand_t<PRR_e, bit_t> PRR_op_t;

  /// A special purpose register operand.
  typedef register_operand_t<SPR_e, word_t> SPR_op_t;
}

#endif // PATMOS_REGISTERS_H


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
    word_t get() const
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
  /// file. S finally represents the size of the register file.
  template<typename I, typename D, unsigned int S>
  class register_file_t
  {
  private:
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
      assert(operand.get_index() < S);
      Content[operand.get_index()] = operand.get();
    }

    /// Set the value of a register in the register file.
    /// @param index The index of the register to be written.
    /// @param value The new value to be written.
    void set(I index, D value)
    {
      register_operand_t<I, D> tmp(index, value);
      set(tmp);
    }

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

    register_operand_t<I, D> get(register_operand_t<I, D> initial)
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

    rS = r29, // Stack Pointer
    rM = r30, // Return Method
    rA = r31  // Return Offset
  };

  /// Symbols representing the predicate registers.
  enum PRR_e
  {
    p0, p1, p2, p3, p4, p5, p6, p7,
    NUM_PRR
  };

  /// A register file for the general purpose registers.
  typedef register_file_t<GPR_e, word_t, NUM_GPR> GPR_t;

  /// A register file for the predicate registers.
  typedef register_file_t<PRR_e, bit_t, NUM_PRR> PRR_t;


  /// A register by-pass for general purpose registers
  typedef by_pass_t<GPR_e, word_t> GPR_by_pass_t;

  /// A register by-pass for predicate purpose registers
  typedef by_pass_t<PRR_e, bit_t> PRR_by_pass_t;


  /// A general purpose register operand.
  typedef register_operand_t<GPR_e, word_t> GPR_op_t;

  /// A predicate register operand.
  typedef register_operand_t<PRR_e, bit_t> PRR_op_t;
}

#endif // PATMOS_REGISTERS_H


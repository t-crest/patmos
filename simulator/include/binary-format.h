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
// Basic interface to instruction formats, defining the binary encoding of
// instruction classes.
//


#ifndef PATMOS_BINARY_FORMAT_H
#define PATMOS_BINARY_FORMAT_H

#include "basic-types.h"

#include <boost/limits.hpp>

#include <cassert>

namespace patmos
{
  // forward declarations
  class instruction_t;
  class instruction_data_t;

  /// Interface to decode the binary representation of instructions.
  /// The format matches a representation when the instruction word matches
  /// the format's pattern under the format's mask, The operands of the
  /// instruction can then be derived as an instruction data instance.
  class binary_format_t
  {
  protected:
    /// Instance of the instruction for simulation.
    const instruction_t &Instruction;

    /// A bit mask applied to the instruction word before matching.
    const word_t Bit_mask;

    /// A bit pattern that has to match in order for the format to match.
    const word_t Bit_pattern;

    /// A bit mask indicating the valid slots, within a bundle, for the
    /// instruction.
    const unsigned int Slots;

    /// Indicate whether the instruction assumes a long immediate field in the
    /// second slot of the instruction bundle (exclusively for the ALUl format).
    bool Is_long;
  public:
    /// Construct a concrete instruction format for an instruction.
    /// @param instruction The simulation interface of the instruction.
    /// @param mask The bit mask of the instruction.
    /// @param pattern The bit pattern of the instruction.
    /// @param slot The set of legal slots of the instruction.
    binary_format_t(const instruction_t &instruction, word_t mask,
                    word_t pattern, unsigned int slots, bool is_long = false) :
        Instruction(instruction), Bit_mask(mask), Bit_pattern(pattern),
        Slots(slots), Is_long(is_long)
    {
      assert((pattern & mask) == pattern);

      assert(slots != 0 && slots <= 3);
      assert(!is_long || slots == 1);
    }

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw,
                                               word_t longimm) const = 0;

    /// Check whether the instruction word matches the instruction format, i.e.,
    /// check whether the format's bit pattern matches its bit mask.
    /// Furthermore, verify that the instruction appears on a legal position
    /// within the instruction bundle.
    /// @param iw The instruction word.
    /// @param slot The position of the instruction within the instruction
    /// bundle.
    /// @return True when the instruction word matches the instruction format;
    /// false otherwise.
    bool matches(word_t iw, unsigned int slot) const
    {
      assert(slot <= 2);

      // check the bit pattern against the mask and verify the slot position
      return ((iw & Bit_mask) == Bit_pattern) &&
             (Slots & (1 << (slot & 1))) != 0;
    }

    /// Return whether the instruction format is a long format (exclusively ALUl
    /// format).
    /// @return Return whether the instruction format is a long format
    /// (exclusively ALUl format).
    bool is_long() const
    {
      return Is_long;
    }
  };
}

#endif // PATMOS_BINARY_FORMAT_H

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
// Some utility functions.
//

#ifndef PATMOS_UTIL_H
#define PATMOS_UTIL_H

#include "basic-types.h"
#include "registers.h"

#include <iostream>

namespace patmos
{
  /// Check if the signed value fits into the given number of bits.
  /// @param value The value.
  /// @return True in case the value fits, false otherwise.
  inline bool fits(word_t val, unsigned int width)
  {
    word_t shift = (sizeof(word_t)*8 - width);
    word_t tmp = (val << shift) >> shift;
    return tmp == val;
  }

  /// Check if the unsigned value fits into the given number of bits.
  /// @param val The value.
  /// @return True in case the fits, false otherwise.
  inline bool fitu(uword_t val, unsigned int width)
  {
    uword_t shift = (sizeof(uword_t)*8 - width);
    uword_t tmp = (val << shift) >> shift;
    return tmp == val;
  }

  /// Assert that the register index is valid for general purpose registers.
  /// @param index The register index to verify.
  /// @return True in case the index is valid, false otherwise.
  inline bool isGPR(word_t index)
  {
    return 0 <= index && index < NUM_GPR;
  }

  /// Assert that the register index is valid for special purpose registers.
  /// @param index The register index to verify.
  /// @return True in case the index is valid, false otherwise.
  inline bool isSPR(word_t index)
  {
    return 0 <= index && index < NUM_SPR;
  }

  /// Assert that the register index is valid for predicate registers.
  /// @param index The register index to verify.
  /// @return True in case the index is valid, false otherwise.
  inline bool isPRR(word_t index)
  {
    return 0 <= index && index < NUM_PRR;
  }

  /// Assert that the register index is valid for predicate registers.
  /// @param index The register index to verify.
  /// @return True in case the index is valid, false otherwise.
  inline bool isPRRn(word_t index)
  {
    return 0 <= index && index < NUM_PRRn;
  }
}

#endif // PATMOS_UTIL_H



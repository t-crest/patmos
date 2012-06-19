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
// Same basic data types used throughout the Patmos simulator.
//

#ifndef PATMOS_BASIC_TYPES_H
#define PATMOS_BASIC_TYPES_H

#include <stdint.h>
#include <vector>

namespace patmos
{
  /// Definition of a double-word value in Patmos.
  typedef int64_t dword_t;

  /// Definition of an unsigned double-word value in Patmos.
  typedef uint64_t udword_t;

  /// Definition of a basic data value in Patmos.
  typedef int32_t word_t;

  /// Definition of an unsigned basic data value in Patmos.
  typedef uint32_t uword_t;

  /// Definition of a half-word in Patmos.
  typedef int16_t hword_t;

  /// Definition of an unsigned half-word in Patmos.
  typedef uint16_t uhword_t;

  /// Definition of a byte in Patmos.
  typedef int8_t byte_t;

  /// Definition of an unsigned byte in Patmos.
  typedef uint8_t ubyte_t;

  /// Definition of a predicate value in Patmos.
  typedef bool bit_t;
}

#endif // PATMOS_BASIC_TYPES_H


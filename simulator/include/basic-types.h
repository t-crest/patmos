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


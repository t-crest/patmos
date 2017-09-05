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
// Some utility functions.
//

#ifndef PATMOS_UTIL_H
#define PATMOS_UTIL_H

#include "basic-types.h"
#include "registers.h"

#include <iostream>

namespace patmos
{
  /// Align a value according to an alignment (round down).
  /// @param i The input value to be aligned.
  /// @param alignment The desired alignment.
  inline uword_t align_down(uword_t i, uword_t alignment)
  {
    return (i / alignment) * alignment;
  }

  /// Align a value according to an alignment (round up).
  /// @param i The input value to be aligned.
  /// @param alignment The desired alignment.
  inline uword_t align_up(uword_t i, uword_t alignment)
  {
    return ((i+alignment-1) / alignment) * alignment;
  }


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



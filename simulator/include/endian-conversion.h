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
// Conversion between big-endian data (simulator) and the host machine's format.
//

#ifndef PATMOS_ENDIAN_CONVERSION_H
#define PATMOS_ENDIAN_CONVERSION_H

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_binary.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_binary.hpp>
#include "basic-types.h"

namespace patmos
{
  /// Map the spirit generator qword to the matching Patmos dword.
  typedef boost::spirit::big_qword_type big_dword_t;

  /// Map the spirit generator dword to the matching Patmos word.
  typedef boost::spirit::big_dword_type big_word_t;

  /// Map the spirit generator dword to the matching Patmos word.
  typedef boost::spirit::big_dword_type big_uword_t;

  /// Map the spirit generator word to the matching Patmos hword.
  typedef boost::spirit::big_word_type big_hword_t;

  /// Map the spirit generator word to the matching Patmos uhword.
  typedef boost::spirit::big_word_type big_uhword_t;

  /// Map the spirit generator word to the matching Patmos byte.
  typedef byte_t big_byte_t;

  /// Map the spirit generator word to the matching Patmos byte.
  typedef byte_t big_ubyte_t;

  /// Convert a native (double/half) word to the big-endian format.
  /// @param v The native value.
  /// @return The value in the big-endian format.
  template<typename G, typename T>
  inline T to_big_endian(T v)
  {
    G g;
    T result;

    // do the conversion
    boost::spirit::karma::generate(reinterpret_cast<char*>(&result), g(v));

    return result;
  }

  /// Convert a native (double/half) word to the big-endian format.
  /// @param v The native value.
  /// @return The value in the big-endian format.
  template<>
  inline byte_t to_big_endian<big_byte_t, byte_t>(byte_t v)
  {
    // a NOP for byte-sized data.
    return v;
  }

  /// Convert a native (double/half) word to the big-endian format.
  /// @param v The native value.
  /// @return The value in the big-endian format.
  template<>
  inline ubyte_t to_big_endian<big_ubyte_t, ubyte_t>(ubyte_t v)
  {
    // a NOP for byte-sized data.
    return v;
  }

  /// Convert a big-endian (double/half) word to the native format.
  /// @param v The big-endian value.
  /// @return The value in the native format.
  template<typename P, typename T>
  inline T from_big_endian(T v)
  {
    P p;
    T result;

    // do the conversion
    char *f = reinterpret_cast<char*>(&v);
    boost::spirit::qi::parse(&f[0], &f[sizeof(T)], p, result);

    return result;
  }

  /// Convert a big-endian (double/half) word to the native format.
  /// @param v The big-endian value.
  /// @return The value in the native format.
  template<>
  inline byte_t from_big_endian<big_byte_t, byte_t>(byte_t v)
  {
    // a NOP for byte-sized data.
    return v;
  }

  /// Convert a big-endian (double/half) word to the native format.
  /// @param v The big-endian value.
  /// @return The value in the native format.
  template<>
  inline ubyte_t from_big_endian<big_ubyte_t, ubyte_t>(ubyte_t v)
  {
    // a NOP for byte-sized data.
    return v;
  }
}

#endif // PATMOS_ENDIAN_CONVERSION_H


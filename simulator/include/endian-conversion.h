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
// Conversion between big-endian data (simulator) and the host machine's format.
//

#ifndef PATMOS_ENDIAN_CONVERSION_H
#define PATMOS_ENDIAN_CONVERSION_H

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_binary.hpp>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_binary.hpp>

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


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
// Simple handling of input/output streams.
//

#ifndef PATMOS_STREAMS_H
#define PATMOS_STREAMS_H

#include <ios>
#include <iostream>

namespace patmos
{

  template< class T >
  struct StandardStream
  {
  };

  template<>
  struct StandardStream< std::basic_istream<char> >
  {
    static std::istream *stream() { return &std::cin; }

    static bool isIOStream(std::basic_istream<char> *stream) {
      return stream == &std::cin;
    }
  };

  template<>
  struct StandardStream< std::basic_ostream<char> >
  {
    static std::ostream *stream() { return &std::cout; }

    static bool isIOStream(std::basic_ostream<char> *stream) {
      return stream == &std::cout || stream == &std::cerr;
    }
  };


  /// Open a file stream, or use the given default.
  /// @param str File name or "-".
  /// @param default_stream A default stream.
  /// @return A reference to a newly created file stream, or, if str equals "-",
  /// a reference to the default stream.
  template<typename T, typename D>
  D *get_stream(const std::string &str, D &default_stream)
  {
    if (str == "")
      return &default_stream;
    else if (str == "-")
      return StandardStream<D>::stream();
    else
    {
      T *result = new T(str.c_str());

      if (!result->good())
      {
        delete result;
        throw std::ios_base::failure("Failed to open file: " + str);
      }

      return result;
    }
  }

  /// Free a stream, e.g., previously opened using get_stream, unless it refers
  /// to an IO stream.
  /// \see get_stream
  /// \param stream The stream to close.
  template<typename T>
  void free_stream(T *stream)
  {
    if (stream && !StandardStream<T>::isIOStream(stream))
      delete stream;
  }
}

#endif // PATMOS_STREAMS_H


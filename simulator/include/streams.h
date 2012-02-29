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
// Simple handling of input/output streams.
//

#ifndef PATMOS_STREAMS_H
#define PATMOS_STREAMS_H

namespace patmos
{
  /// Open a file stream, or use the given default.
  /// @param str File name or "-".
  /// @param default_stream A default stream.
  /// @return A reference to a newly created file stream, or, if str equals "-", a
  /// reference to the default stream.
  template<typename T, typename D>
  D &get_stream(char *str, D &default_stream)
  {
    if (std::string(str) == "-")
      return default_stream;
    else
      return *new T(str);
  }

  /// Free a stream, e.g., previously opened using get_stream, unless it refers 
  /// to the default stream.
  /// \see get_stream
  /// \param stream The stream to close.
  /// \param default_stream The default stream.
  template<typename T, typename D>
  void free_stream(T &stream, D &default_stream)
  {
    if (&stream != &default_stream)
      delete &stream;
  }
}

#endif // PATMOS_STREAMS_H


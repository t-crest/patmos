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
//  This is the connection interface used for low-level communication with the 
//  gdb client. Provides functions to read from the client and write to the
//  client.
//

#ifndef PATMOS_GDB_CONNECTION_H
#define PATMOS_GDB_CONNECTION_H

#include <string>

namespace patmos
{

  /// Interface used for low-level communication with the gdb client. Provides 
  /// functions to read from and write to the other end of the communication.
  class GdbConnection
  {
  public:
    virtual ~GdbConnection() {}

    /// Send a single character to the other end.
    /// @param c the character to send.
    virtual void PutChar(char c) const = 0;

    /// Get a single character from the other end.
    /// Blocks until a character is available.
    /// @return the character that was received
    virtual char GetChar() const = 0;

    /// Send a string to the other end.
    /// @param str the string to be sent.
    virtual void Write(const std::string &str) const = 0;
  };

}
#endif // PATMOS_GDB_CONNECTION_H

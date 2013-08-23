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

#ifndef PATMOS_PIPE_H
#define PATMOS_PIPE_H

namespace patmos
{

  /*
   * Simple implementation of a gdb connection using a pipe. Using std input/
   * std output to read/write from.
   */
  class Pipe : public GdbConnection
  {
  public:
    virtual void PutChar(char c) const;
    virtual char GetChar() const;
    virtual void Write(const std::string &str) const = 0;
  };

}
#endif // PATMOS_PIPE_H

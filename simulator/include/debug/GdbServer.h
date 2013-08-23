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
//  gdb server implementation, used for gdb debugging.
//

#ifndef PATMOS_GDB_SERVER_H
#define PATMOS_GDB_SERVER_H

#include <string>

#include "debug/DebugClient.h"

namespace patmos
{

  class GdbConnectionFailedException : public std::exception
  {
  public:
    GdbConnectionFailedException();
    ~GdbConnectionFailedException() throw();
    virtual const char* what() const throw();
  
  private:
    std::string m_whatMessage;
  };

  class GdbServer : public DebugClient
  {
  public:
    GdbServer(DebugInterface &debugInterface);

    void Init();

    // Implement DebugClient
    virtual void BreakpointHit(const Breakpoint &bp);
  
  private:
    DebugInterface &m_debugInterface;
  };

}
#endif // PATMOS_GDB_SERVER_H

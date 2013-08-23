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
//  debugging interface - used for gdb debugging
//

#ifndef PATMOS_DEBUG_INTERFACE_H
#define PATMOS_DEBUG_INTERFACE_H

namespace patmos
{
  class DebugClient;

  class Breakpoint
  {
  public:
    Breakpoint(int pc);

  private:
    int m_pc;
  };

  /*
   * Interface for a debugging interface.
   */
  class DebugInterface
  {
  public:
    virtual ~DebugInterface() {}
    virtual void SetDebugClient(DebugClient &debugClient) = 0;
    virtual void AddBreakpoint(Breakpoint bp) = 0;
    virtual void RemoveBreakpoint(Breakpoint bp) = 0;
  };

}
#endif // PATMOS_DEBUG_INTERFACE_H

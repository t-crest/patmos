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
//  debugging client interface - used for gdb debugging.
//  implement this interface if you want to debug the simulator
//

#ifndef PATMOS_DEBUG_CLIENT_H
#define PATMOS_DEBUG_CLIENT_H

namespace patmos
{

  class Breakpoint;
  class DebugInterface;

  /*
   * Interface for debugging clients, i.e. programs that want to debug another
   * program that implements DebugInterface
   */
  class DebugClient
  {
  public:
    virtual ~DebugClient() {}
    virtual void BreakpointHit(const Breakpoint &bp) = 0;
  };

}
#endif // PATMOS_DEBUG_CLIENT_H

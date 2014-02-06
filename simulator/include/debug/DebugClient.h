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
//  Debugging client interface - used for gdb/lldb debugging.
//

#ifndef PATMOS_DEBUG_CLIENT_H
#define PATMOS_DEBUG_CLIENT_H

namespace patmos
{
  class Breakpoint;
  class DebugInterface;

  /// Interface for debugging clients (=debugger), i.e. programs that want to
  /// debug another program that implements DebugInterface (=debuggee).
  /// The debugging client (debugger) can (and will) take control over the
  /// debugee and communicate with it via the DebugInterface interface.
  /// Functions that transfer control to the debugging client are blocking, 
  /// until the debugging client releases control again (usually via continue 
  /// or step commands)
  class DebugClient
  {
  public:
    virtual ~DebugClient() {}

    /// Establishes a connection to the debugging client. If the debugging 
    /// client is remote (i.e. a gdb server that communcates to another machine) 
    /// this might take a while and it will block, until the connection has been
    /// established.
    /// Transfers control to the debugging client as soon as a connection is
    /// established.
    virtual void Connect() = 0;

    /// Will be called by the debuggee if a previously added breakpoint is hit.
    /// Transfers control to the debugging client.
    /// @param bp the breakpoint that caused the hit.
    virtual void BreakpointHit(const Breakpoint &bp) = 0;

    /// Will be called by the debuggee if a single step has been performed.
    /// Transfers control to the debugging client.
    virtual void SingleStepDone() = 0;
  };

}
#endif // PATMOS_DEBUG_CLIENT_H

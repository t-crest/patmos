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
//  gdb server implementation, used for gdb remote debugging.
//

#ifndef PATMOS_GDB_SERVER_H
#define PATMOS_GDB_SERVER_H

#include <string>
#include <boost/scoped_ptr.hpp>

#include "debug/DebugClient.h"
#include "debug/GdbPacketHandler.h"
#include "debug/GdbMessageHandler.h"

namespace patmos
{

  class GdbConnection;

  typedef boost::scoped_ptr<GdbPacketHandler> GdbPacketHandlerPtr;
  typedef boost::scoped_ptr<GdbMessageHandler> GdbMessageHandlerPtr;

  /// Provides a GDB RSP server acting as a DebugClient. Includes lldb
  /// extensions to GDB's RSP. 
  class GdbServer : public DebugClient
  {
  public:
    /// Standard constructor
    /// @param debugInterface a reference to the debugging interface that will
    /// be debugged.
    /// @param connection a data connection to the debugging client (gdb or
    ///   lldb)
    GdbServer(DebugInterface &debugInterface,
        const GdbConnection &connection);

    /// Implement DebugClient
    void Connect();

    /// Implement DebugClient
    virtual void BreakpointHit(const Breakpoint &bp);

    /// Implement DebugClient
    virtual void SingleStepDone();

    /// Enable/Disable internal debugging. If enabled, all GDB RSP messages will
    /// be printed to stderr.
    /// @param debugMessages if true, internal debugging will be enabled.
    void SetDebugMessages(bool debugMessages);
  
  private:
    /// This will transfer the control of the debugging interface to the debug
    /// client (gdb or lldb), i.e. after a call to this function, the user of
    /// the debugger has control until he lets the target continue (e.g. with a
    /// continue or step command)
    void TransferControlToClient();

    DebugInterface &m_debugInterface;
    const GdbConnection &m_connection;
    GdbPacketHandlerPtr m_packetHandler;
    GdbMessageHandlerPtr m_messageHandler;
  };

}
#endif // PATMOS_GDB_SERVER_H

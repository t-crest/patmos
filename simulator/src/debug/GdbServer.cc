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

#include "debug/GdbServer.h"
#include "debug/GdbMessage.h"

namespace patmos
{

  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////

  GdbConnectionFailedException::GdbConnectionFailedException()
    : m_whatMessage("Error: GdbServer: Could not get a connection to the gdb client.")
  {
  }
  GdbConnectionFailedException::~GdbConnectionFailedException() throw()
  {
  }
  const char* GdbConnectionFailedException::what() const throw()
  {
    return m_whatMessage.c_str();
  }
  

  //////////////////////////////////////////////////////////////////
  // GdbServer implementation
  //////////////////////////////////////////////////////////////////
  
  GdbServer::GdbServer(DebugInterface &debugInterface,
      const GdbConnection &connection)
    : m_debugInterface(debugInterface),
      m_connection(connection),
      m_packetHandler(new GdbPacketHandler(m_connection)),
      m_messageHandler(new GdbMessageHandler(*m_packetHandler))
  {
  }

  void GdbServer::Start()
  {
    // Get the first message from the gdb client.
    // Everything else is now driven by the client.
    // The server only responds to messages that are received from the client.
    // These respond messages are created and sent by the message's 
    // Handle function.
    GdbMessagePtr message;
    bool targetContinue = false;
    while (!targetContinue)
    {
      message = m_messageHandler->ReadGdbMessage();
      message->Handle(*m_messageHandler, m_debugInterface, targetContinue);
    }
  }

  // Implement DebugClient
  void GdbServer::BreakpointHit(const Breakpoint &bp)
  {
  }
}

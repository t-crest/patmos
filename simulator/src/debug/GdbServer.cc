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

  void GdbServer::TransferControlToClient()
  {
    // Everything is now driven by the client, until a message handler sets
    // targetContinue to true, which gives back control to the target.
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

  void GdbServer::Connect()
  {
    TransferControlToClient();
  }

  void GdbServer::BreakpointHit(const Breakpoint &bp)
  {
    // Send the reason why we stopped.
    m_messageHandler->SendGdbMessage(GetStopReplyMessage("breakpoint"));
    
    TransferControlToClient();
  }

  void GdbServer::SingleStepDone()
  {
    // Send the reason why we stopped.
    m_messageHandler->SendGdbMessage(GetStopReplyMessage("trace"));
  
    TransferControlToClient();
  }

  void GdbServer::SetDebugMessages(bool debugMessages)
  {
    m_packetHandler->SetDebug(debugMessages);
  }
}

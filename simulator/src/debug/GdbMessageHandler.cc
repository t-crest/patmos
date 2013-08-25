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
//  This handles the different GDB RSP messages. It implements the server side
//  of the high level GDB RSP protocol.
//

#include "debug/GdbMessageHandler.h"
#include "debug/GdbMessage.h"
#include "debug/GdbPacketHandler.h"

#include <boost/algorithm/string/predicate.hpp>

namespace patmos
{

  //////////////////////////////////////////////////////////////////
  // GdbMessageHandler implementation
  //////////////////////////////////////////////////////////////////
  
  GdbMessageHandler::GdbMessageHandler(const GdbPacketHandler &packetHandler)
    : m_packetHandler(packetHandler)
  {
  }

  GdbMessagePtr GdbMessageHandler::ReadGdbMessage() const
  {
    GdbPacket packet = m_packetHandler.ReadGdbPacket();
    assert(packet.IsValid());

    std::string packetContent = packet.GetContent();

    if (boost::starts_with(packetContent, supportedMessage))
    {
      return GdbMessagePtr(new GdbSupportedMessage());
    }
    else if (boost::starts_with(packetContent, setThreadMessage))
    {
      return GdbMessagePtr(new GdbSetThreadMessage());
    }
    else if (packetContent == getReasonMessage)
    {
      return GdbMessagePtr(new GdbGetReasonMessage());
    }

    // unsupported message / we do not know now to handle that
    return GdbMessagePtr(new GdbUnsupportedMessage(packetContent));
  }
  
  void GdbMessageHandler::SendGdbMessage(const GdbMessagePtr &message) const
  {
    GdbPacket packet = CreateGdbPacket(message->GetMessageString());
    m_packetHandler.WriteGdbPacket(packet);
  }

}

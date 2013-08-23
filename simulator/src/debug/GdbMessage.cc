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
//  This represents the GDB RSP message layout, i.e. the high level
//  abstraction of rsp packets.
//  Each message handles its effect on its own.
//

#include "debug/GdbMessage.h"
#include "debug/GdbPacketHandler.h"

#include <sstream>
#include <boost/format.hpp>
#include <iostream> //debug only

namespace
{
  using namespace patmos;

  //////////////////////////////////////////////////////////////////
  // Messages
  // Formatted strings have boost::format syntax
  //////////////////////////////////////////////////////////////////
  
  const std::string supportedMessage = "qSupported";
  const std::string supportedMessage_response = "PacketSize=%x";
  const std::string getReasonMessage = "?";
  const std::string getReasonMessage_response = "S%02x";

  //////////////////////////////////////////////////////////////////
  // Helper functions
  //////////////////////////////////////////////////////////////////
 
  void SendMessage(const GdbPacketHandler &packetHandler,
      std::string message)
  {
    GdbPacket packet = CreateGdbPacket(message);
    packetHandler.WriteGdbPacket(packet);
  }

  //////////////////////////////////////////////////////////////////
  // qSupported
  //////////////////////////////////////////////////////////////////
  class GdbSupportedMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return supportedMessage;
    }
    virtual void Handle(const GdbPacketHandler &packetHandler) const
    {
      const int maxBytes = 0;
      const std::string response = 
        (boost::format(supportedMessage_response) % maxBytes).str();
      SendMessage(packetHandler, response);
    }
  };
  
  //////////////////////////////////////////////////////////////////
  // ?
  //////////////////////////////////////////////////////////////////
  class GdbGetReasonMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return getReasonMessage;
    }
    virtual void Handle(const GdbPacketHandler &packetHandler) const
    {
      const int signalNumber = 5;
      const std::string response = 
        (boost::format(getReasonMessage_response) % signalNumber).str();
      SendMessage(packetHandler, response);
    }
  };
  
  //////////////////////////////////////////////////////////////////
  // Unsupported message. This is used to indicate that the message
  // is unknown or not implemented. Calling Handle() on this message
  // will result in an exception.
  //////////////////////////////////////////////////////////////////
  class GdbUnsupportedMessage : public GdbMessage
  {
  public:
    GdbUnsupportedMessage(std::string packetContent)
      : m_packetContent(packetContent)
    {
    }

    virtual std::string GetMessageString() const
    {
      return "Unsupported Message: " + m_packetContent;
    }
    
    virtual void Handle(const GdbPacketHandler &packetHandler) const
    {
      throw GdbUnsupportedMessageException(m_packetContent);
    }

  private:
    std::string m_packetContent;
  };

}

namespace patmos
{
  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////

  GdbUnsupportedMessageException::GdbUnsupportedMessageException(
      std::string packetContent)
  {
    std::stringstream ss;
    ss << "Error: GdbServer: Received an unsupported message: " << packetContent;
    m_whatMessage= ss.str();
  }
  GdbUnsupportedMessageException::~GdbUnsupportedMessageException() throw()
  {
  }
  const char* GdbUnsupportedMessageException::what() const throw()
  {
    return m_whatMessage.c_str();
  }
  
  //////////////////////////////////////////////////////////////////
  // GetMessage implementation
  //////////////////////////////////////////////////////////////////
  
  GdbMessagePtr GetGdbMessage(std::string packetContent)
  {
    // first, check static messages
    if (packetContent == supportedMessage)
    {
      return GdbMessagePtr(new GdbSupportedMessage());
    }
    else if (packetContent == getReasonMessage)
    {
      return GdbMessagePtr(new GdbGetReasonMessage());
    }

    // unsupported message / we do not know now to handle that
    return GdbMessagePtr(new GdbUnsupportedMessage(packetContent));
  }
}

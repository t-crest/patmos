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
#include "debug/GdbMessageHandler.h"

#include <sstream>
#include <boost/format.hpp>
#include <iostream> //debug only

namespace patmos
{
  
  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////

  GdbUnsupportedMessageException::GdbUnsupportedMessageException(
      std::string packetContent)
  {
    m_whatMessage = "Error: GdbServer: Received an unsupported message: " + packetContent;
  }
  GdbUnsupportedMessageException::~GdbUnsupportedMessageException() throw()
  {
  }
  const char* GdbUnsupportedMessageException::what() const throw()
  {
    return m_whatMessage.c_str();
  }

  //////////////////////////////////////////////////////////////////
  // response messages
  //////////////////////////////////////////////////////////////////

  void GdbResponseMessage::Handle(const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    throw GdbUnsupportedMessageException(GetMessageString());
  }
  
  std::string GdbOKMessage::GetMessageString() const
  {
    return okMessage;
  }
  
  GdbErrorMessage::GdbErrorMessage(int errorNr)
    : m_errorNr(errorNr)
  {
  }

  std::string GdbErrorMessage::GetMessageString() const
  {
    return (boost::format(errorMessage) % m_errorNr).str();
  }
  
  //////////////////////////////////////////////////////////////////
  // qSupported
  //////////////////////////////////////////////////////////////////
  std::string GdbSupportedMessage::GetMessageString() const
  {
    return supportedMessage;
  }
  void GdbSupportedMessage::Handle(
      const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    const int maxBytes = 0;
    GdbMessagePtr response(new GdbSupportedMessageResponse(maxBytes));
    messageHandler.SendGdbMessage(response);
    targetContinue = false;
  }
  GdbSupportedMessageResponse::GdbSupportedMessageResponse(int maxBytes)
    : m_maxBytes(maxBytes)
  {
  }
  std::string GdbSupportedMessageResponse::GetMessageString() const
  {
    return (boost::format(supportedMessage_response) % m_maxBytes).str();
  }
  
  //////////////////////////////////////////////////////////////////
  // ?
  //////////////////////////////////////////////////////////////////
  std::string GdbGetReasonMessage::GetMessageString() const
  {
    return getReasonMessage;
  }
  void GdbGetReasonMessage::Handle(const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    const int signalNumber = 5;
    GdbMessagePtr response(new GdbGetReasonMessageResponse(signalNumber));
    messageHandler.SendGdbMessage(response);
    targetContinue = false;
  }
  GdbGetReasonMessageResponse::GdbGetReasonMessageResponse(int signalNumber)
    : m_signalNumber(signalNumber)
  {
  }
  std::string GdbGetReasonMessageResponse::GetMessageString() const
  {
    return (boost::format(getReasonMessage_response) % m_signalNumber).str();
  }
  
  //////////////////////////////////////////////////////////////////
  // H <op> <thread-id>
  //////////////////////////////////////////////////////////////////
  std::string GdbSetThreadMessage::GetMessageString() const
  {
    return setThreadMessage;
  }
  void GdbSetThreadMessage::Handle(const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    // currently we do not care about threads
    GdbMessagePtr response(new GdbOKMessage);
    messageHandler.SendGdbMessage(response);
    targetContinue = false;
  }

  //////////////////////////////////////////////////////////////////
  // Unsupported message. This is used to indicate that the message
  // is unknown or not implemented. Calling Handle() on this message
  // will result in an exception.
  //////////////////////////////////////////////////////////////////
  GdbUnsupportedMessage::GdbUnsupportedMessage(std::string packetContent)
    : m_packetContent(packetContent)
  {
  }

  std::string GdbUnsupportedMessage::GetMessageString() const
  {
    return "Unsupported Message: " + m_packetContent;
  }
    
  void GdbUnsupportedMessage::Handle(const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    throw GdbUnsupportedMessageException(m_packetContent);
  }
  
}

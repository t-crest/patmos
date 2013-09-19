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
#include <boost/algorithm/string/predicate.hpp>

namespace
{
  using namespace patmos;

  GdbResponseMessage GetOKMessage()
  {
    return GdbResponseMessage(okMessage);
  }

  GdbResponseMessage GetErrorMessage(int errorNr)
  {
    return GdbResponseMessage((boost::format(errorMessage) % errorNr).str());
  }

  GdbResponseMessage GetEmptyMessage()
  {
    return GdbResponseMessage("");
  }
}

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
  // response message
  //////////////////////////////////////////////////////////////////

  GdbResponseMessage::GdbResponseMessage(std::string response)
    : m_response(response)
  {
  }
  
  std::string GdbResponseMessage::GetMessageString() const
  {
    return m_response;
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
    const std::string response = 
      (boost::format(supportedMessage_response) % maxPacketSize).str();
    messageHandler.SendGdbMessage(GdbResponseMessage(response));
    targetContinue = false;
  }
  bool GdbSupportedMessage::CanHandle(std::string messageString)
  {
    return boost::starts_with(messageString, supportedMessage);
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
    const std::string response = 
          (boost::format(getReasonMessage_response) % signalNumber).str();
    messageHandler.SendGdbMessage(GdbResponseMessage(response));
    targetContinue = false;
  }
  bool GdbGetReasonMessage::CanHandle(std::string messageString)
  {
    return messageString == getReasonMessage;
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
    messageHandler.SendGdbMessage(GetOKMessage());
    targetContinue = false;
  }
  bool GdbSetThreadMessage::CanHandle(std::string messageString)
  {
    return boost::starts_with(messageString, setThreadMessage);
  }

  //////////////////////////////////////////////////////////////////
  // qC
  //////////////////////////////////////////////////////////////////
  std::string GdbGetCurrentThreadMessage::GetMessageString() const
  {
    return getCurrentThreadMessage;
  }
  void GdbGetCurrentThreadMessage::Handle(
      const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    // currently we do not care about threads
    messageHandler.SendGdbMessage(GetEmptyMessage());
    targetContinue = false;
  }
  bool GdbGetCurrentThreadMessage::CanHandle(std::string messageString)
  {
    return messageString == getCurrentThreadMessage;
  }

  //////////////////////////////////////////////////////////////////
  // qAttached
  //////////////////////////////////////////////////////////////////
  std::string GdbIsAttachedMessage::GetMessageString() const
  {
    return isAttachedMessage;
  }
  void GdbIsAttachedMessage::Handle(
      const GdbMessageHandler &messageHandler,
      bool &targetContinue) const
  {
    // currently we do not supported processes and the simulator
    // is never attached. This results in the client terminating the
    // target, rather than just detaching from it
    messageHandler.SendGdbMessage(GdbResponseMessage("0"));
    targetContinue = false;
  }
  bool GdbIsAttachedMessage::CanHandle(std::string messageString)
  {
    return messageString == isAttachedMessage;
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

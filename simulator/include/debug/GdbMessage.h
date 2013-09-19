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
//

#ifndef PATMOS_GDB_MESSAGE_H
#define PATMOS_GDB_MESSAGE_H

#include <string>
#include <boost/shared_ptr.hpp>

namespace patmos
{
  class GdbMessageHandler;

  //////////////////////////////////////////////////////////////////
  // Messages
  // Formatted strings have boost::format syntax
  //////////////////////////////////////////////////////////////////
 
  const int maxPacketSize = 512; // random for now. After some testing,
                                 // adjust this value to a more appropriate one

  const std::string supportedMessage = "qSupported";
  const std::string supportedMessage_response = "PacketSize=%x";
  const std::string getReasonMessage = "?";
  const std::string getReasonMessage_response = "S%02x";
  const std::string setThreadMessage = "H";
  const std::string getCurrentThreadMessage = "qC";
  const std::string isAttachedMessage = "qAttached";

  const std::string okMessage = "OK";
  const std::string errorMessage = "E %02x";
  
  //////////////////////////////////////////////////////////////////
  // Exceptions
  //////////////////////////////////////////////////////////////////
  
  class GdbUnsupportedMessageException : public std::exception
  {
  public:
    GdbUnsupportedMessageException(std::string packetContent);
    ~GdbUnsupportedMessageException() throw();
    virtual const char* what() const throw();
  
  private:
    std::string m_whatMessage;
  };

  //////////////////////////////////////////////////////////////////
  // Message types
  //////////////////////////////////////////////////////////////////
  
  class GdbMessage
  {
  public:
    virtual ~GdbMessage() {}
    virtual std::string GetMessageString() const = 0;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const = 0;
  };
  typedef boost::shared_ptr<GdbMessage> GdbMessagePtr;

  class GdbResponseMessage
  {
  public:
    GdbResponseMessage(std::string response);
    std::string GetMessageString() const;
  private:
    std::string m_response;
  };
  
  //////////////////////////////////////////////////////////////////
  // qSupported
  //////////////////////////////////////////////////////////////////
  
  class GdbSupportedMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
    static bool CanHandle(std::string messageString);
  };
  
  //////////////////////////////////////////////////////////////////
  // ?
  //////////////////////////////////////////////////////////////////
  
  class GdbGetReasonMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
    static bool CanHandle(std::string messageString);
  };
  
  //////////////////////////////////////////////////////////////////
  // H <op> <thread-id>
  //////////////////////////////////////////////////////////////////
  
  class GdbSetThreadMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
    static bool CanHandle(std::string messageString);
  };
  
  //////////////////////////////////////////////////////////////////
  // qC
  //////////////////////////////////////////////////////////////////
  
  class GdbGetCurrentThreadMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
    static bool CanHandle(std::string messageString);
  };
  
  //////////////////////////////////////////////////////////////////
  // qAttached
  //////////////////////////////////////////////////////////////////
  
  class GdbIsAttachedMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
    static bool CanHandle(std::string messageString);
  };
  
  //////////////////////////////////////////////////////////////////
  // Unsupported messages
  //////////////////////////////////////////////////////////////////
  
  class GdbUnsupportedMessage : public GdbMessage
  {
  public:
    GdbUnsupportedMessage(std::string packetContent);
    virtual std::string GetMessageString() const;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
  private:
    std::string m_packetContent;
  };

}
#endif // PATMOS_GDB_MESSAGE_H

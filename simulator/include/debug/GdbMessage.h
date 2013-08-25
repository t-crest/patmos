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
  
  const std::string supportedMessage = "qSupported";
  const std::string supportedMessage_response = "PacketSize=%x";
  const std::string getReasonMessage = "?";
  const std::string getReasonMessage_response = "S%02x";
  const std::string setThreadMessage = "H";


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
  // Message Implementations
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

  class GdbResponseMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const = 0;
    virtual void Handle(const GdbMessageHandler &messageHandler,
        bool &targetContinue) const;
  };

  class GdbOKMessage : public GdbResponseMessage
  {
  public:
    virtual std::string GetMessageString() const;
  };
  
  class GdbErrorMessage : public GdbResponseMessage
  {
  public:
    GdbErrorMessage(int errorNr);
    virtual std::string GetMessageString() const;
  private:
    int m_errorNr;
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
  };
  
  class GdbSupportedMessageResponse : public GdbResponseMessage
  {
  public:
    GdbSupportedMessageResponse(int maxBytes);
    virtual std::string GetMessageString() const;
  private:
    int m_maxBytes;
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
  };
  
  class GdbGetReasonMessageResponse : public GdbResponseMessage
  {
  public:
    GdbGetReasonMessageResponse(int signalNumber);
    virtual std::string GetMessageString() const;
  private:
    int m_signalNumber;
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

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

#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace
{
  //////////////////////////////////////////////////////////////////
  // Message strings
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
  const std::string startNoAckModeMessage = "QStartNoAckMode";
  const std::string hostInfoMessage = "qHostInfo";

  const std::string okMessage = "OK";
  const std::string errorMessage = "E %02x";
  
  // unsupported features:
  const std::string unsupportedFeatureMessage = "Unsupported Feature";
  const std::string threadSuffixSupportedMessage = "QThreadSuffixSupported";
  const std::string listThreadsInStopReplyMessage = "QListThreadsInStopReply";
  const std::string contSupportedMessage = "vCont?";
  const std::string attachOrWaitSupportedMessage = "qVAttachOrWaitSupported";

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
    m_whatMessage = "Error: GdbServer: Tried to handle an unsupported message: " + packetContent;
  }
  GdbUnsupportedMessageException::~GdbUnsupportedMessageException() throw()
  {
  }
  const char* GdbUnsupportedMessageException::what() const throw()
  {
    return m_whatMessage.c_str();
  }

  //////////////////////////////////////////////////////////////////
  // Key Value Response - a response message consisting of key/value
  // data pairs
  //////////////////////////////////////////////////////////////////
  class GdbKeyValueResponseMessage : public GdbResponseMessage
  {
  public:
    GdbKeyValueResponseMessage()
      : GdbResponseMessage("")
    {
    }

    template <class T>
    void AddKeyValue(std::string key, T value)
    {
      m_ss << key << ":" << value << ";";
    }

    virtual std::string GetMessageString() const
    {
      return m_ss.str();
    }

  private:
    std::stringstream m_ss;
  };

  //////////////////////////////////////////////////////////////////
  // qSupported
  //////////////////////////////////////////////////////////////////
  class GdbSupportedMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return supportedMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const std::string response = 
        (boost::format(supportedMessage_response) % maxPacketSize).str();
      messageHandler.SendGdbMessage(GdbResponseMessage(response));
    };

    static bool CanHandle(std::string messageString)
    {
      return boost::starts_with(messageString, supportedMessage);
    };
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
    };
  
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const int signalNumber = 5;
      const std::string response = 
            (boost::format(getReasonMessage_response) % signalNumber).str();
      messageHandler.SendGdbMessage(GdbResponseMessage(response));
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == getReasonMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // H <op> <thread-id>
  //////////////////////////////////////////////////////////////////
  class GdbSetThreadMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return setThreadMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // currently we do not care about threads
      messageHandler.SendGdbMessage(GetOKMessage());
    };

    static bool CanHandle(std::string messageString)
    {
      return boost::starts_with(messageString, setThreadMessage);
    };
  };

  //////////////////////////////////////////////////////////////////
  // qC
  //////////////////////////////////////////////////////////////////
  class GdbGetCurrentThreadMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return getCurrentThreadMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // currently we do not care about threads
      messageHandler.SendGdbMessage(GetEmptyMessage());
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == getCurrentThreadMessage;
    };
  };

  //////////////////////////////////////////////////////////////////
  // qAttached
  //////////////////////////////////////////////////////////////////
  class GdbIsAttachedMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return isAttachedMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // currently we do not supported processes and the simulator
      // is never attached. This results in the client terminating the
      // target, rather than just detaching from it
      messageHandler.SendGdbMessage(GdbResponseMessage("0"));
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == isAttachedMessage;
    };
  };

  //////////////////////////////////////////////////////////////////
  // (lldb extension) QStartNoAckMode
  //////////////////////////////////////////////////////////////////
  class GdbStartNoAckModeMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return startNoAckModeMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SetUseAck(false);
      messageHandler.SendGdbMessage(GetOKMessage());
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == startNoAckModeMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) qHostInfo
  //////////////////////////////////////////////////////////////////
  class GdbHostInfoMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return hostInfoMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      GdbKeyValueResponseMessage msg;
      HostInfo info = debugInterface.GetHostInfo();
      msg.AddKeyValue<int>        ("cputype"    , info.cputype);
      msg.AddKeyValue<int>        ("cpusubtype" , info.cpusubtype);
      msg.AddKeyValue<std::string>("ostype"     , info.ostype);
      msg.AddKeyValue<std::string>("vendor"     , info.vendor);
      msg.AddKeyValue<std::string>("endian"     , info.endian);
      msg.AddKeyValue<int>        ("ptrsize"    , info.ptrsize);
      messageHandler.SendGdbMessage(msg);
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == hostInfoMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) all unsupported features - reply empty msg
  //////////////////////////////////////////////////////////////////
  class GdbUnsupportedFeatureMessage : public GdbMessage
  {
  public:
    virtual std::string GetMessageString() const
    {
      return unsupportedFeatureMessage;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetEmptyMessage());
    };

    static bool CanHandle(std::string messageString)
    {
      return (
        messageString == threadSuffixSupportedMessage ||
        messageString == listThreadsInStopReplyMessage ||
        messageString == contSupportedMessage ||
        messageString == attachOrWaitSupportedMessage
      );
    };
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
    };

    virtual std::string GetMessageString() const
    {
      return "Unsupported Message: " + m_packetContent;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      throw GdbUnsupportedMessageException(m_packetContent);
    };

  private:
    std::string m_packetContent;
  };
  
  //////////////////////////////////////////////////////////////////
  // GdbMessageHandler implementation
  //////////////////////////////////////////////////////////////////
  
  GdbMessageHandler::GdbMessageHandler(GdbPacketHandler &packetHandler)
    : m_packetHandler(packetHandler)
  {
  }

  GdbMessagePtr GdbMessageHandler::ReadGdbMessage() const
  {
    GdbPacket packet = m_packetHandler.ReadGdbPacket();

    std::string packetContent = packet.GetContent();

    if (GdbSupportedMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbSupportedMessage());
    }
    else if (GdbSetThreadMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbSetThreadMessage());
    }
    else if (GdbGetReasonMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbGetReasonMessage());
    }
    else if (GdbGetCurrentThreadMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbGetCurrentThreadMessage());
    }
    else if (GdbIsAttachedMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbIsAttachedMessage());
    }
    else if (GdbStartNoAckModeMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbStartNoAckModeMessage());
    }
    else if (GdbHostInfoMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbHostInfoMessage());
    }
    else if (GdbUnsupportedFeatureMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbUnsupportedFeatureMessage());
    }

    // unsupported message / we do not know now to handle that
    return GdbMessagePtr(new GdbUnsupportedMessage(packetContent));
  }
  
  void GdbMessageHandler::SendGdbMessage(
      const GdbResponseMessage &message) const
  {
    GdbPacket packet = CreateGdbPacket(message.GetMessageString());
    m_packetHandler.WriteGdbPacket(packet);
  }

  void GdbMessageHandler::SetUseAck(bool useAck)
  {
    m_packetHandler.SetUseAck(useAck);
  }

}

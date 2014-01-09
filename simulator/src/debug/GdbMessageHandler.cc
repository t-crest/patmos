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
  const int maxPacketSize = 512; // random for now. After some testing,
                                 // adjust this value to a more appropriate one
  const int dummyProcessId = 1;
  const int dummyThreadId = 1;
  
  //////////////////////////////////////////////////////////////////
  // Message strings
  // Formatted strings have boost::format syntax
  //////////////////////////////////////////////////////////////////

  const std::string supportedMessage = "qSupported";
  const std::string supportedResponse = "PacketSize=%x";
  const std::string getReasonMessage = "?";
  const std::string setThreadMessage = "H";
  const std::string getCurrentThreadMessage = "qC";
  const std::string isAttachedMessage = "qAttached";
  const std::string contSupportedMessage = "vCont?";
  const std::string contSupportedResponse = "vCont;c;s;t";
  const std::string attachMessage = "vAttach;";
  const std::string threadInfoStartMessage = "qfThreadInfo";
  const std::string threadInfoStartResponse = "m1";
  const std::string threadInfoNextMessage = "qsThreadInfo";
  const std::string threadInfoNextResponse = "l";

  const std::string killMessage = "k";
  
  // lldb extensions:
  const std::string startNoAckModeMessage = "QStartNoAckMode";
  const std::string hostInfoMessage = "qHostInfo";
  const std::string registerInfoMessage = "qRegisterInfo";
  const std::string threadStopInfoMessage = "qThreadStopInfo";

  // standard response messages:
  const std::string okMessage = "OK";
  const std::string errorMessage = "E %02x";
  const std::string stopReplyMessage = "T%02x";
  
  // unsupported features:
  const std::string threadSuffixSupportedMessage = "QThreadSuffixSupported";
  const std::string listThreadsInStopReplyMessage = "QListThreadsInStopReply";
  const std::string attachOrWaitSupportedMessage = "qVAttachOrWaitSupported";
  const std::string processInfoMessage = "qProcessInfo";

  using namespace patmos;

  class KeyValueStringBuilder
  {
  public:
    template <class T>
    void AddKeyValue(std::string key, T value)
    {
      m_ss << key << ":" << value << ";";
    }

    virtual std::string GetString() const
    {
      return m_ss.str();
    }

  private:
    std::stringstream m_ss;
  };

  //////////////////////////////////////////////////////////////////
  // Default Messages
  //////////////////////////////////////////////////////////////////
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

  GdbResponseMessage GetStopReplyMessage(int signalNumber)
  {
    KeyValueStringBuilder params;
    params.AddKeyValue<int>("thread", dummyThreadId);

    std::stringstream ss; 
    ss << (boost::format(stopReplyMessage) % signalNumber).str();
    ss << params.GetString();
    return GdbResponseMessage(ss.str());
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
      m_builder.AddKeyValue<T>(key, value);
    }

    virtual std::string GetMessageString() const
    {
      return m_builder.GetString();
    }

  private:
    KeyValueStringBuilder m_builder;
  };

  //////////////////////////////////////////////////////////////////
  // qSupported
  //////////////////////////////////////////////////////////////////
  class GdbSupportedMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const std::string response = 
        (boost::format(supportedResponse) % maxPacketSize).str();
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
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const int signalNumber = 17;
      messageHandler.SendGdbMessage(GetStopReplyMessage(signalNumber));
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
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // currently we do not care about threads.
      // Empty response means keep the previous thread:
      //messageHandler.SendGdbMessage(GetEmptyMessage());
      // OR return 1 to fake one thread:
       messageHandler.SendGdbMessage(GdbResponseMessage("QC1"));
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
  // vCont?
  //////////////////////////////////////////////////////////////////
  class GdbContSupportedMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(contSupportedResponse));
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == contSupportedMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // vAttach;<x>
  //////////////////////////////////////////////////////////////////
  class GdbAttachMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const int signalNumber = 17;
      messageHandler.SendGdbMessage(GetStopReplyMessage(signalNumber));
    };

    static bool CanHandle(std::string messageString)
    {
      return boost::starts_with(messageString, attachMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // qfThreadInfo
  //////////////////////////////////////////////////////////////////
  class GdbThreadInfoStartMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(threadInfoStartResponse));
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == threadInfoStartMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // qsThreadInfo
  //////////////////////////////////////////////////////////////////
  class GdbThreadInfoNextMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(threadInfoNextResponse));
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == threadInfoNextMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // k
  //////////////////////////////////////////////////////////////////
  class GdbKillMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // TODO: change to a better target control
      targetContinue = true;
    };

    static bool CanHandle(std::string messageString)
    {
      return messageString == killMessage;
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) QStartNoAckMode
  //////////////////////////////////////////////////////////////////
  class GdbStartNoAckModeMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetOKMessage());
      messageHandler.SetUseAck(false);
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
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      GdbKeyValueResponseMessage msg;
      HostInfo info = debugInterface.GetHostInfo();

      if (info.cputype > 0)
        msg.AddKeyValue<int>        ("cputype"    , info.cputype);
      
      if (info.cpusubtype > 0)
        msg.AddKeyValue<int>        ("cpusubtype" , info.cpusubtype);
      
      if (info.triple.size() > 0)
        msg.AddKeyValue<std::string>("triple"     , "patmos--linux");
      else
      {
        msg.AddKeyValue<std::string>("ostype"     , info.ostype);
        msg.AddKeyValue<std::string>("vendor"     , info.vendor);
      }

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
  // (lldb extension) qRegisterInfo
  //////////////////////////////////////////////////////////////////
  
  const std::string registerEncodingStrings[] = {
    "uint",
    "sint",
    "ieee754",
    "vector"
  };

  const std::string registerFormatStrings[] = {
    "binary",
    "decimal",
    "hex",
    "float",
    "vector_sint8",
    "vector_uint8",
    "vector_sint16",
    "vector_uint16",
    "vector_sint32",
    "vector_uint32",
    "vector_float32",
    "vector_uint128"
  };

  const std::string registerTypeStrings[] = {
    "",
    "pc",   
    "sp",   
    "fp",   
    "ra",   
    "flags",
    "arg1",
    "arg2", 
    "arg3",
    "arg4",
    "arg5",
    "arg6",
    "arg7",
    "arg8",
  };

  class GdbRegisterInfoMessage : public GdbMessage
  {
  public:
    GdbRegisterInfoMessage(std::string messageString)
      : m_registerNumber(0)
    {
      const std::string::size_type pos = registerInfoMessage.size();
      const std::string registerNumberString =
        messageString.substr(pos);
      std::stringstream ss(registerNumberString);
      ss >> m_registerNumber;
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      GdbKeyValueResponseMessage msg;

      const RegisterInfo info = debugInterface.GetRegisterInfo();
      
      // Check if there is a register with that number.
      if (m_registerNumber < 0 | m_registerNumber >= info.regCount)
      {
        messageHandler.SendGdbMessage(GetEmptyMessage());
        return;
      }

      // Register exists - lets report it's parameters
      const RegisterInfoEntry reg = info.registers[m_registerNumber];
      
      msg.AddKeyValue<std::string>  ("name",     reg.name);
      msg.AddKeyValue<int>          ("bitsize",  reg.bitsize);
      msg.AddKeyValue<std::string>  ("encoding", registerEncodingStrings[reg.encoding]);
      msg.AddKeyValue<std::string>  ("format",   registerFormatStrings[reg.format]);
      msg.AddKeyValue<std::string>  ("set",      reg.setName);
      msg.AddKeyValue<int>          ("dwarf",    reg.dwarfNumber);

      if (reg.type > 0)
      {
        // only add generic type if available (otherwise omit this value)
        msg.AddKeyValue<std::string>("generic", registerTypeStrings[reg.type]);
      }

      messageHandler.SendGdbMessage(msg);
    };

    static bool CanHandle(std::string messageString)
    {
      return boost::starts_with(messageString, registerInfoMessage);
    };

  private:
    int m_registerNumber;
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) qThreadStopInfo<x>
  //////////////////////////////////////////////////////////////////
  class GdbThreadStopInfoMessage : public GdbMessage
  {
  public:
    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const int signalNumber = 17;
      messageHandler.SendGdbMessage(GetStopReplyMessage(signalNumber));
    };

    static bool CanHandle(std::string messageString)
    {
      return boost::starts_with(messageString, threadStopInfoMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // all unsupported features - reply empty msg
  //////////////////////////////////////////////////////////////////
  class GdbUnsupportedFeatureMessage : public GdbMessage
  {
  public:
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
        messageString == attachOrWaitSupportedMessage ||
        messageString == processInfoMessage
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

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      throw GdbUnsupportedMessageException(m_packetContent);
    };

  private:
    std::string m_packetContent;
  };

} // unnamed namespace

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
    else if (GdbContSupportedMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbContSupportedMessage());
    }
    else if (GdbAttachMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbAttachMessage());
    }
    else if (GdbThreadInfoStartMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbThreadInfoStartMessage());
    }
    else if (GdbThreadInfoNextMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbThreadInfoNextMessage());
    }
    else if (GdbKillMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbKillMessage());
    }
    else if (GdbStartNoAckModeMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbStartNoAckModeMessage());
    }
    else if (GdbHostInfoMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbHostInfoMessage());
    }
    else if (GdbRegisterInfoMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbRegisterInfoMessage(packetContent));
    }
    else if (GdbThreadStopInfoMessage::CanHandle(packetContent))
    {
      return GdbMessagePtr(new GdbThreadStopInfoMessage());
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

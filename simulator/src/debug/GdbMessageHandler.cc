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
#include "debug/DebugInterface.h"

#include <sstream>
#include <stdint.h>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace
{
  const int maxPacketSize = 512; // random for now. After some testing,
                                 // adjust this value to a more appropriate one
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
  const std::string contSupportedResponse = "vCont;c;s";
  const std::string attachMessage = "vAttach;";
  const std::string threadInfoStartMessage = "qfThreadInfo";
  const std::string threadInfoStartResponse = "m1";
  const std::string threadInfoNextMessage = "qsThreadInfo";
  const std::string threadInfoNextResponse = "l";
  const std::string killMessage = "k";
  const std::string continueMessage = "c";
  const std::string readRegisterMessage = "p";
  const std::string setBreakpointMessage = "Z0";
  const std::string removeBreakpointMessage = "z0";
  const std::string stepMessage = "vCont;s";
  const std::string readMemoryMessage = "m";

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
  const std::string shlibInfoAddrMessage = "qShlibInfoAddr";
  const std::string memoryRegionInfoMessage = "qMemoryRegionInfo";

  const std::string messageListDelimiter = ",";
  const std::string messageParameterDelimiter = ";";
  const std::string messageParameterValueDelimiter = ":";

  using namespace patmos;

  class KeyValueStringBuilder
  {
  public:
    template <class T>
    void AddKeyValue(const std::string& key, T value)
    {
      m_ss << key << messageParameterValueDelimiter << value << messageParameterDelimiter;
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
    void AddKeyValue(const std::string& key, T value)
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
    GdbSupportedMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      const std::string response = 
        (boost::format(supportedResponse) % maxPacketSize).str();
      messageHandler.SendGdbMessage(GdbResponseMessage(response));
    };

    static bool CanHandle(const std::string& messageString)
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
    GdbGetReasonMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetStopReplyMessage());
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, getReasonMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // H <op> <thread-id>
  //////////////////////////////////////////////////////////////////
  class GdbSetThreadMessage : public GdbMessage
  {
  public:
    GdbSetThreadMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // Currently we do not care about threads.
      // So all of these operations are ignored.
      messageHandler.SendGdbMessage(GetOKMessage());
    };

    static bool CanHandle(const std::string& messageString)
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
    GdbGetCurrentThreadMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

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

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, getCurrentThreadMessage);
    };
  };

  //////////////////////////////////////////////////////////////////
  // qAttached
  //////////////////////////////////////////////////////////////////
  class GdbIsAttachedMessage : public GdbMessage
  {
  public:
    GdbIsAttachedMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // currently we do not supported processes and the simulator
      // is never attached. This results in the client terminating the
      // target, rather than just detaching from it
      messageHandler.SendGdbMessage(GdbResponseMessage("0"));
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, isAttachedMessage);
    };
  };

  //////////////////////////////////////////////////////////////////
  // vCont?
  //////////////////////////////////////////////////////////////////
  class GdbContSupportedMessage : public GdbMessage
  {
  public:
    GdbContSupportedMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(contSupportedResponse));
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, contSupportedMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // vAttach;<x>
  //////////////////////////////////////////////////////////////////
  class GdbAttachMessage : public GdbMessage
  {
  public:
    GdbAttachMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetStopReplyMessage());
    };

    static bool CanHandle(const std::string& messageString)
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
    GdbThreadInfoStartMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(threadInfoStartResponse));
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, threadInfoStartMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // qsThreadInfo
  //////////////////////////////////////////////////////////////////
  class GdbThreadInfoNextMessage : public GdbMessage
  {
  public:
    GdbThreadInfoNextMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GdbResponseMessage(threadInfoNextResponse));
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, threadInfoNextMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // k
  //////////////////////////////////////////////////////////////////
  class GdbKillMessage : public GdbMessage
  {
  public:
    GdbKillMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // TODO: change to a better target control
      targetContinue = true;
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, killMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // c
  //////////////////////////////////////////////////////////////////
  class GdbContinueMessage : public GdbMessage
  {
  public:
    GdbContinueMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      // TODO: change to a better target control
      targetContinue = true;
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, continueMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // d
  //////////////////////////////////////////////////////////////////
  class GdbReadRegisterMessage : public GdbMessage
  {
  public:
    GdbReadRegisterMessage(const std::string& messageString)
      : GdbMessage(messageString)
    {
      const std::string::size_type pos = readRegisterMessage.size();
      if (pos < messageString.size())
      {
        const std::string registerNumberString =
          messageString.substr(pos);
        std::stringstream ss(registerNumberString);
        ss << std::hex;
        ss >> m_registerNumber;
      }
    }

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      RegisterContent registerContent = 
        debugInterface.GetRegisterContent(m_registerNumber);
      messageHandler.SendGdbMessage(registerContent);
    }

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, readRegisterMessage);
    }

  private:
    int m_registerNumber;
  };
  
  //////////////////////////////////////////////////////////////////
  // Z0
  // z0
  //////////////////////////////////////////////////////////////////
  
  int ExtractBreakpointAddress(const std::string& messageString)
  {
    int breakpointAddress = 0;
    typedef std::string::size_type size_type;

    const size_type addressStart = 
      messageString.find(messageListDelimiter) + 1;
    if (addressStart < messageString.length())
    {
      const size_type addressEnd =
        messageString.find(messageListDelimiter, addressStart) + 1;
      if (addressEnd < messageString.length())
      {
        const size_type addressLength = addressEnd - addressStart;
        std::stringstream ss(messageString.substr(addressStart, addressLength));
        ss << std::hex;
        ss >> breakpointAddress;
      }
    }

    return breakpointAddress;
  }
  
  class GdbSetBreakpointMessage : public GdbMessage
  {
  public:
    GdbSetBreakpointMessage(const std::string& messageString)
      : GdbMessage(messageString), 
        m_breakpointAddress(ExtractBreakpointAddress(messageString))
    {
    }

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      Breakpoint bp(m_breakpointAddress);
      if (debugInterface.AddBreakpoint(bp))
        messageHandler.SendGdbMessage(GetOKMessage());
      else
        messageHandler.SendGdbMessage(GetErrorMessage(0));
    }

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, setBreakpointMessage);
    }

  private:
    int m_breakpointAddress;
  };
  
  class GdbRemoveBreakpointMessage : public GdbMessage
  {
  public:
    GdbRemoveBreakpointMessage(const std::string& messageString)
      : GdbMessage(messageString), 
        m_breakpointAddress(ExtractBreakpointAddress(messageString))
    {
    }

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      Breakpoint bp(m_breakpointAddress);
      if (debugInterface.RemoveBreakpoint(bp))
        messageHandler.SendGdbMessage(GetOKMessage());
      else
        messageHandler.SendGdbMessage(GetErrorMessage(0));
    }

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, removeBreakpointMessage);
    }

  private:
    int m_breakpointAddress;
  };
  
  //////////////////////////////////////////////////////////////////
  // vCont;s
  //////////////////////////////////////////////////////////////////
  class GdbStepMessage : public GdbMessage
  {
  public:
    GdbStepMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      debugInterface.SingleStep();
      targetContinue = true;
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, stepMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // m
  //////////////////////////////////////////////////////////////////
  
  void ExtractMemoryAddressAndLength(const std::string& messageString,
      long& address, long& length)
  {
    std::vector<std::string> params;
    boost::split(params, messageString, boost::is_any_of(messageListDelimiter));
    
    if (params.size() >= 1)
    {
      std::stringstream ss(params[0]);
      ss << std::hex;
      ss >> address;
    }

    if (params.size() >= 2)
    {
      std::stringstream ss(params[1]);
      ss << std::hex;
      ss >> length;
    }
  }
  
  class GdbReadMemoryMessage : public GdbMessage
  {
  public:
    GdbReadMemoryMessage(const std::string& messageString)
      : GdbMessage(messageString), m_address(0), m_length(0)
    {
      const std::string::size_type pos = readMemoryMessage.size();
      if (pos < messageString.size())
      {
        const std::string paramString =  messageString.substr(pos);
        ExtractMemoryAddressAndLength(paramString, m_address, m_length);
      }
    }

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      MemoryContent content = debugInterface.GetMemoryContent(m_address, m_length);
      if (content.size() > 0)
        messageHandler.SendGdbMessage(content);
      else
        messageHandler.SendGdbMessage(GetErrorMessage(0));
    }

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, readMemoryMessage);
    }

  private:
    long m_address;
    long m_length;
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) QStartNoAckMode
  //////////////////////////////////////////////////////////////////
  class GdbStartNoAckModeMessage : public GdbMessage
  {
  public:
    GdbStartNoAckModeMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetOKMessage());
      messageHandler.SetUseAck(false);
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, startNoAckModeMessage);
    };
  };
  
  //////////////////////////////////////////////////////////////////
  // (lldb extension) qHostInfo
  //////////////////////////////////////////////////////////////////
  class GdbHostInfoMessage : public GdbMessage
  {
  public:
    GdbHostInfoMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

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
        msg.AddKeyValue<std::string>("triple"     , info.triple);
      else
      {
        msg.AddKeyValue<std::string>("ostype"     , info.ostype);
        msg.AddKeyValue<std::string>("vendor"     , info.vendor);
      }

      msg.AddKeyValue<std::string>("endian"     , info.endian);
      msg.AddKeyValue<int>        ("ptrsize"    , info.ptrsize);
      messageHandler.SendGdbMessage(msg);
    };

    static bool CanHandle(const std::string& messageString)
    {
      return boost::starts_with(messageString, hostInfoMessage);
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
    GdbRegisterInfoMessage(const std::string& messageString)
      : GdbMessage(messageString), m_registerNumber(-1)
    {
      const std::string::size_type pos = registerInfoMessage.size();
      if (pos < messageString.size())
      {
        const std::string registerNumberString =
          messageString.substr(pos);
        std::stringstream ss(registerNumberString);
        ss << std::hex;
        ss >> m_registerNumber;
      }
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      GdbKeyValueResponseMessage msg;

      const RegisterInfoVec info = debugInterface.GetRegisterInfo();
      // Check if there is a register with that number.
      if (m_registerNumber < 0 || m_registerNumber >= info.size())
      {
        messageHandler.SendGdbMessage(GetEmptyMessage());
        return;
      }

      // Register exists - lets report it's parameters
      const RegisterInfo reg = info[m_registerNumber];
      
      msg.AddKeyValue<std::string>  ("name",     reg.name);
      msg.AddKeyValue<int>          ("bitsize",  reg.bitsize);
      msg.AddKeyValue<int>          ("offset",   reg.offset);
      msg.AddKeyValue<std::string>  ("encoding", registerEncodingStrings[reg.encoding]);
      msg.AddKeyValue<std::string>  ("format",   registerFormatStrings[reg.format]);
      msg.AddKeyValue<std::string>  ("set",      reg.setName);

      if (reg.dwarfNumber != noDwarf)
        msg.AddKeyValue<int>        ("dwarf",    reg.dwarfNumber);

      if (reg.type > 0)
      {
        // only add generic type if available (otherwise omit this value)
        msg.AddKeyValue<std::string>("generic", registerTypeStrings[reg.type]);
      }

      messageHandler.SendGdbMessage(msg);
    };

    static bool CanHandle(const std::string& messageString)
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
    GdbThreadStopInfoMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetStopReplyMessage());
    };

    static bool CanHandle(const std::string& messageString)
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
    GdbUnsupportedFeatureMessage(const std::string& messageString)
      : GdbMessage(messageString)
    { };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      messageHandler.SendGdbMessage(GetEmptyMessage());
    };

    static bool CanHandle(const std::string& messageString)
    {
      return (
        messageString == threadSuffixSupportedMessage ||
        messageString == listThreadsInStopReplyMessage ||
        messageString == attachOrWaitSupportedMessage ||
        messageString == processInfoMessage ||
        messageString == shlibInfoAddrMessage ||
        boost::starts_with(messageString, memoryRegionInfoMessage)
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
    GdbUnsupportedMessage(const std::string& messageString)
      : GdbMessage(messageString)
    {
    };

    virtual void Handle(GdbMessageHandler &messageHandler,
      DebugInterface &debugInterface,
      bool &targetContinue) const
    {
      throw GdbUnsupportedMessageException(GetMessageString());
    };
    static bool CanHandle(const std::string& messageString)
    {
      return true;
    };
  };


  //////////////////////////////////////////////////////////////////
  // Message Creator and Factory to register messages
  // use GdbMessageFactory::Register<GdbMessage>() to register messages
  // use GdbMessageFactory::Create(string) to create a message
  //////////////////////////////////////////////////////////////////
  class GdbMessageCreator
  {
  public:
    virtual bool CanCreate(const std::string& messageString) = 0;
    virtual GdbMessagePtr Create(const std::string& messageString) = 0;
  };

  template<class T>
  class GenericGdbMessageCreator : public GdbMessageCreator
  {
  public:
    virtual bool CanCreate(const std::string& messageString)
    {
      return T::CanHandle(messageString);
    };

    virtual GdbMessagePtr Create(const std::string& messageString)
    {
      return GdbMessagePtr(new T(messageString));
    };
  };
  
  typedef boost::shared_ptr<GdbMessageCreator>
    GdbMessageCreatorPtr;
  typedef std::vector<GdbMessageCreatorPtr>
    GdbMessageCreatorVec;

  class GdbMessageFactory
  {
  public:
    template<class T>
    static void Register()
    {
      m_creators.push_back(
          GdbMessageCreatorPtr(new GenericGdbMessageCreator<T>()));
    };
    
    static GdbMessagePtr Create(const std::string& messageString)
    {
      for (GdbMessageCreatorVec::const_iterator it = m_creators.begin();
          it != m_creators.end(); ++it)
      {
          if ((*it)->CanCreate(messageString))
          {
            return (*it)->Create(messageString);
          }
      }
      return GdbMessagePtr(new GdbUnsupportedMessage(messageString));
    };

  private:
    static GdbMessageCreatorVec m_creators;
  };

  GdbMessageCreatorVec GdbMessageFactory::m_creators;

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
    // standard GDB RSP messages
    GdbMessageFactory::Register<GdbSupportedMessage>();
    GdbMessageFactory::Register<GdbSetThreadMessage>();
    GdbMessageFactory::Register<GdbGetReasonMessage>();
    GdbMessageFactory::Register<GdbGetCurrentThreadMessage>();
    GdbMessageFactory::Register<GdbIsAttachedMessage>();
    GdbMessageFactory::Register<GdbContSupportedMessage>();
    GdbMessageFactory::Register<GdbAttachMessage>();
    GdbMessageFactory::Register<GdbThreadInfoStartMessage>();
    GdbMessageFactory::Register<GdbThreadInfoNextMessage>();
    GdbMessageFactory::Register<GdbKillMessage>();
    GdbMessageFactory::Register<GdbContinueMessage>();
    GdbMessageFactory::Register<GdbReadRegisterMessage>();
    GdbMessageFactory::Register<GdbSetBreakpointMessage>();
    GdbMessageFactory::Register<GdbRemoveBreakpointMessage>();
    GdbMessageFactory::Register<GdbStepMessage>();
    GdbMessageFactory::Register<GdbReadMemoryMessage>();

    // lldb extensions
    GdbMessageFactory::Register<GdbStartNoAckModeMessage>();
    GdbMessageFactory::Register<GdbHostInfoMessage>();
    GdbMessageFactory::Register<GdbRegisterInfoMessage>();
    GdbMessageFactory::Register<GdbThreadStopInfoMessage>();
    GdbMessageFactory::Register<GdbUnsupportedFeatureMessage>();
  }

  GdbMessagePtr GdbMessageHandler::ReadGdbMessage() const
  {
    GdbPacket packet = m_packetHandler.ReadGdbPacket();
    std::string packetContent = packet.GetContent();
    return GdbMessageFactory::Create(packetContent);
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

  GdbResponseMessage GetStopReplyMessage(std::string reason, int signalNumber,
      int threadId)
  {
    KeyValueStringBuilder params;
    params.AddKeyValue<int>         ("thread", threadId);
    params.AddKeyValue<std::string> ("reason", reason);

    std::stringstream ss; 
    ss << (boost::format(stopReplyMessage) % signalNumber).str();
    ss << params.GetString();
    return GdbResponseMessage(ss.str());
  }

}

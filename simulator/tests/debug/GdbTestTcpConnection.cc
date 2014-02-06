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
//  Test implementation of the gdb server.
//

#include <iostream>
#include <iomanip>
#include "debug/GdbServer.h"
#include "debug/TcpConnection.h"
#include "debug/DebugInterface.h"
#include "debug/PatmosRegisterInfo.h"

using namespace patmos;

namespace
{

  const std::string patmosTriple  = "patmos--";
  const std::string patmosEndian  = "big";
  const int         patmosPtrSize = 4;

  const RegisterContent emptyRegister("");

  class PatmosDebugInterface;

  class PatmosSimulator
  {
  public:
    PatmosSimulator()
    : m_programRuns(true), m_singleStep(false)
    {
    }

    void SetDebugClient(DebugClient *debugClient)
    {
      m_debugClient = debugClient;
    }

    void Run()
    {
      std::cerr << "waiting for client connection..." << std::endl;
      m_debugClient->Connect();

      int pc = 0;
      while (m_programRuns)
      {
        std::cerr << "pc=" << pc << std::endl;

        DoDebugActions(pc);
    
        // simulate the processor here...
        ++pc;
    
        // some terminate condition...
        if (pc > 4400)
          m_programRuns = false;
      }
    }

  private:
    void DoDebugActions(int pc)
    {
      if (WasSingleStep())
      {
        std::cerr << "single step" << std::endl;
        m_debugClient->SingleStepDone();
      }
      else if (IsDebugBreakpointHit(pc))
      {
        std::cerr << "breakpoint hit" << std::endl;
        m_debugClient->BreakpointHit(Breakpoint(pc));
        // need to check what to do next, maybe have to skip this interation?
      }

    }
    bool IsDebugBreakpointHit(int pc)
    {
      return (m_breakpoints.find(pc) != m_breakpoints.end());
    }
    bool WasSingleStep()
    {
      bool wasSingleStep = m_singleStep;
      m_singleStep = false;
      return wasSingleStep;
    }

    void PrepareSingleStep()
    {
      m_singleStep = true;
    }
    void AddDebugBreakpoint(const Breakpoint &bp)
    {
      m_breakpoints[bp.pc] = bp;
    }

    DebugClient *m_debugClient;
    bool m_programRuns;

    std::map<int, Breakpoint> m_breakpoints;
    bool m_singleStep;
    
    friend class PatmosDebugInterface;
  };

  class PatmosDebugInterface : public DebugInterface
  {
  public:
    PatmosDebugInterface(PatmosSimulator &simulator)
      : m_simulator(simulator), m_registerInfo(CreatePatmosRegisterInfo())
    {
    }

    virtual HostInfo GetHostInfo() const
    {
      HostInfo info;
      info.triple = patmosTriple;
      info.endian = patmosEndian;
      info.ptrsize = patmosPtrSize;
      return info;
    }
    virtual RegisterInfoVec GetRegisterInfo() const
    {
      return m_registerInfo;
    }

    virtual RegisterContent GetRegisterContent(int registerNumber) const
    {
      if (registerNumber < 0 || 
          registerNumber >= m_registerInfo.size())
        return emptyRegister;

      const RegisterInfo registerInfo = m_registerInfo[registerNumber];
      const int registerSize = registerInfo.bitsize;
      const int byteSize = registerSize / 8;

      unsigned int registerContent[4] = {
        0, 0, 0, 0
      };

      registerContent[0] = registerNumber;

      std::stringstream ss;
      ss << std::hex << std::setfill('0');
      for (int i = 0; i < byteSize; ++i)
      {
        ss << std::setw(2) << registerContent[i];
      }

      return RegisterContent(ss.str());
    }

    virtual MemoryContent GetMemoryContent(long address, long width) const
    {
      const int byteSize = 10;

      unsigned int memoryContent[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };

      std::stringstream ss;
      ss << std::hex << std::setfill('0');
      for (int i = 0; i < byteSize; ++i)
      {
        ss << std::setw(2) << memoryContent[i];
      }

      return MemoryContent(ss.str());
    }

    virtual bool AddBreakpoint(const Breakpoint &bp)
    {
      m_simulator.AddDebugBreakpoint(bp);
      return true;
    }
    
    virtual bool RemoveBreakpoint(Breakpoint bp)
    {
      return true;
    }

    virtual void SingleStep()
    {
      m_simulator.PrepareSingleStep();
    }

  private:
    PatmosSimulator &m_simulator;
    RegisterInfoVec m_registerInfo;
  };
  
}

int main(int argc, char **argv)
{
  const int listenPort = 1234;

  std::cerr << "using port " << listenPort << std::endl;

  std::cerr << "establishing connection...";
  TcpConnection con(listenPort);
  std::cerr << " done." << std::endl;

  PatmosSimulator simulator;
  PatmosDebugInterface patmosDebugInterface(simulator);
  DebugInterface &debugInterface = patmosDebugInterface;
  
  GdbServer gdbServer(debugInterface, con);
  DebugClient &debugClient = gdbServer;

  simulator.SetDebugClient(&debugClient);
  simulator.Run();

}

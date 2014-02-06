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
//  Implementation of the patmos debugging interface for the patmos simulator.
//

#include "debug/PatmosDebugInterface.h"
#include "debug/PatmosRegisterInfo.h"
#include "simulation-core.h"
#include "memory.h"

#include <sstream>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>

namespace
{
  const std::string patmosTriple  = "patmos--";
  const std::string patmosEndian  = "big";
  const int         patmosPtrSize = 4;
  
  const patmos::RegisterContent emptyRegister("");
}

namespace patmos
{

  PatmosDebugInterface::PatmosDebugInterface(simulator_t &simulator)
    : m_simulator(simulator), m_registerInfo(CreatePatmosRegisterInfo()),
      m_singleStep(false), m_debugActions(false)
  {
  }

  HostInfo PatmosDebugInterface::GetHostInfo() const
  {
    HostInfo info;
    info.triple = patmosTriple;
    info.endian = patmosEndian;
    info.ptrsize = patmosPtrSize;
    return info;
  }
  RegisterInfoVec PatmosDebugInterface::GetRegisterInfo() const
  {
    return m_registerInfo;
  }

  RegisterContent PatmosDebugInterface::GetRegisterContent(
      int registerNumber) const
  {
    if (m_debugActions)
      std::cerr << "PatmosDebugInterface::GetRegisterContent (" << registerNumber << ")" << std::endl;
    
    if (registerNumber < 0 || 
        registerNumber >= m_registerInfo.size())
      return emptyRegister;
    
    else if (registerNumber == 0)
      return GetPCContent();

    else if (registerNumber >= 1 && registerNumber <= 32)
      return GetGPRContent(registerNumber - 1);
    
    else if (registerNumber >= 33 && registerNumber <= 48)
      return GetSPRContent(registerNumber - 33);
    
    else if (registerNumber >= 49 && registerNumber <= 56)
      return GetPRRContent(registerNumber - 49);

    else
      return RegisterContent();
  }

  MemoryContent PatmosDebugInterface::GetMemoryContent(
      long address, long width) const
  {
    if (m_debugActions)
      std::cerr << "PatmosDebugInterface::GetMemoryContent (addr:" << address << ", width=" << width << ")" << std::endl;
    
    boost::scoped_array<byte_t> buf(new byte_t[width]);
    m_simulator.Memory.read_peek(address, buf.get(), width);
    
    std::stringstream ss;
    ss << std::hex;
    // big endian - start with the most significat byte
    for (int i = width - 1; i >= 0; --i)
    {
      ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
    }
    return MemoryContent(ss.str());
  }

  bool PatmosDebugInterface::AddBreakpoint(const Breakpoint &bp)
  {
    if (m_debugActions)
      std::cerr << "PatmosDebugInterface::AddBreakpoint - " << bp.pc << std::endl;
    
    if (m_breakpoints.find(bp.pc) == m_breakpoints.end())
      m_breakpoints[bp.pc] = bp;
    return true;
  }
  
  bool PatmosDebugInterface::RemoveBreakpoint(Breakpoint bp)
  {
    if (m_debugActions)
      std::cerr << "PatmosDebugInterface::RemoveBreakpoint - " << bp.pc << std::endl;
    
    return (m_breakpoints.erase(bp.pc) == 1);
  }

  void PatmosDebugInterface::SingleStep()
  {
    if (m_debugActions)
      std::cerr << "PatmosDebugInterface::SingleStep - Initiating single step" << std::endl;
    m_singleStep = true;
  }

  void PatmosDebugInterface::TakeControl(int pc)
  {
    if (WasSingleStep())
    {
      if (m_debugActions)
        std::cerr << "PatmosDebugInterface::TakeControl - Was single step" << std::endl;
      
      m_singleStep = false;
      m_simulator.debug_client->SingleStepDone();
      
      if (m_debugActions)
        std::cerr << "PatmosDebugInterface::TakeControl - Release Control (continue)" << std::endl;
    }
    else if (IsDebugBreakpointHit(pc))
    {
      if (m_debugActions)
        std::cerr << "PatmosDebugInterface::TakeControl - Breakpoint hit (" << pc << ")" << std::endl;
      
      m_simulator.debug_client->BreakpointHit(Breakpoint(pc));
      
      if (m_debugActions)
        std::cerr << "PatmosDebugInterface::TakeControl - Release Control (continue)" << std::endl;
    }
  }

  void PatmosDebugInterface::SetDebugActions(bool debugActions)
  {
    m_debugActions = debugActions;
  }

  bool PatmosDebugInterface::IsDebugBreakpointHit(int pc) const
  {
    return (m_breakpoints.find(pc) != m_breakpoints.end());
  }

  bool PatmosDebugInterface::WasSingleStep()
  {
    return m_singleStep;
  }

  RegisterContent PatmosDebugInterface::GetPCContent() const
  {
    std::stringstream ss;
    ss << boost::format("%1$08x") % m_simulator.PC;
    return RegisterContent(ss.str());
  }

  RegisterContent PatmosDebugInterface::GetGPRContent(int r) const
  {
    std::stringstream ss;                                                       
    ss << boost::format("%1$08x") % m_simulator.GPR.get((GPR_e)r).get();
    return RegisterContent(ss.str());
  }

  RegisterContent PatmosDebugInterface::GetSPRContent(int s) const
  {
    std::stringstream ss;
    ss << boost::format("%1$08x") % m_simulator.SPR.get((SPR_e)s).get();
    return RegisterContent(ss.str());
  }

  RegisterContent PatmosDebugInterface::GetPRRContent(int p) const
  {
    std::stringstream ss;
    ss << boost::format("%1$02x") % m_simulator.PRR.get((PRR_e)p).get();
    return RegisterContent(ss.str());
  }

}

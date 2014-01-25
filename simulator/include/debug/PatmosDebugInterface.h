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
//  Debugging interface of the patmos simulator.
//

#ifndef PATMOS_PATMOS_DEBUG_INTERFACE_H
#define PATMOS_PATMOS_DEBUG_INTERFACE_H

#include "debug/DebugInterface.h"

#include <map>

namespace patmos
{
  class simulator_t;

  class PatmosDebugInterface : public DebugInterface
  {
  public:
    PatmosDebugInterface(simulator_t &simulator);
    
    // implement DebugInterface
    virtual HostInfo GetHostInfo() const;
    // implement DebugInterface
    virtual RegisterInfo GetRegisterInfo() const;

    // implement DebugInterface
    virtual RegisterContent GetRegisterContent(int registerNumber) const;
    // implement DebugInterface
    virtual MemoryContent GetMemoryContent(long address, long width) const;

    // implement DebugInterface
    virtual bool AddBreakpoint(const Breakpoint &bp);
    // implement DebugInterface
    virtual bool RemoveBreakpoint(Breakpoint bp);

    // implement DebugInterface
    virtual void SingleStep();

    // Takes over control to check for actions (if a breakpoint is hit or
    // a single instruction step was performed). Might move control to the
    // debugging client.
    void TakeControl(int pc);

    void SetDebugActions(bool debugActions);

  private:
    bool IsDebugBreakpointHit(int pc) const;

    bool WasSingleStep();
    
    RegisterContent GetPCContent() const;
  
    RegisterContent GetGPRContent(int r) const;

    RegisterContent GetSPRContent(int s) const;

    RegisterContent GetPRRContent(int p) const;

    simulator_t &m_simulator;
    RegisterInfo m_registerInfo;
    
    // A map of breakpoints <PC, BREAKPOINT>
    std::map<int, Breakpoint> m_breakpoints;

    // True, if a single step was requested.
    bool m_singleStep;

    bool m_debugActions;
  };

}

#endif // PATMOS_PATMOS_DEBUG_INTERFACE_H

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
    
    /// implement DebugInterface
    virtual HostInfo GetHostInfo() const;
    /// implement DebugInterface
    virtual RegisterInfoVec GetRegisterInfo() const;

    /// implement DebugInterface
    virtual RegisterContent GetRegisterContent(int registerNumber) const;
    /// implement DebugInterface
    virtual MemoryContent GetMemoryContent(long address, long width) const;

    /// implement DebugInterface
    virtual bool AddBreakpoint(const Breakpoint &bp);
    /// implement DebugInterface
    virtual bool RemoveBreakpoint(Breakpoint bp);

    /// implement DebugInterface
    virtual void SingleStep();

    /// Takes over control to check for actions (if a breakpoint is hit or
    /// a single instruction step was performed). Might move control to the
    /// debugging client and block until the debugging client releases control
    /// again.
    /// @param pc current program counter in the pipeline.
    void TakeControl(int pc);

    /// Enables/Disables internal debugging. If enabled, all debugging relevant
    /// actions (like a breakpoint hit, reading/writing register contents by the
    /// debug client) are printed to stderr.
    /// @param debugActions if true, internal debugging will be enabled.
    void SetDebugActions(bool debugActions);

  private:
    /// Checks whether any breakpoints are hit by the current program counter.
    /// @param pc current program counter in the pipeline.
    /// @return true if a breakpoint exists for the given program counter.
    bool IsDebugBreakpointHit(int pc) const;

    /// Checks whether the last machine instruction was a single step. This is
    /// true if there was a previous call to SingleStep().
    /// @return true if there was a previous call to SingleStep()
    bool WasSingleStep();
    
    /// @return the content of the pc register.
    RegisterContent GetPCContent() const;
  
    /// @param r GPR register number
    /// @return the content of the general purpose register with the given
    ///   number.
    RegisterContent GetGPRContent(int r) const;

    /// @param s SPR register number
    /// @return the content of the special purpose register with the given
    ///   number.
    RegisterContent GetSPRContent(int s) const;

    /// @param p PRR register number
    /// @return the content of the predicate register with the given number.
    RegisterContent GetPRRContent(int p) const;

    /// A reference to the patmos simulator to debug.
    simulator_t &m_simulator;

    /// Contains all register information needed by the debug client.
    RegisterInfoVec m_registerInfo;
    
    /// A map of breakpoints <PC, BREAKPOINT>.
    std::map<int, Breakpoint> m_breakpoints;

    /// True, if a single step was requested.
    bool m_singleStep;

    /// True, if interal debugging is enabled.
    bool m_debugActions;
  };

}

#endif // PATMOS_PATMOS_DEBUG_INTERFACE_H

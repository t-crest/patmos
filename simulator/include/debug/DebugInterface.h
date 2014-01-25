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
//  debugging interface - used for gdb debugging
//

#ifndef PATMOS_DEBUG_INTERFACE_H
#define PATMOS_DEBUG_INTERFACE_H

#include "debug/RegisterInfo.h"

#include <string>

namespace patmos
{
  struct HostInfo
  {
    HostInfo()
      : cputype(0), cpusubtype(0), triple(""), ostype("unknown"), 
        vendor("unknown"), endian("little"), ptrsize(0)
    {};

    int cputype;        // is a number that is the mach-o CPU type that is 
                        // being debugged
    int cpusubtype;     // is a number that is the mach-o CPU subtype type 
                        // that is being debugged
    std::string triple; // target triple. Use this instead ostype and vendor.
    std::string ostype; // is a string the represents the OS being debugged 
                        // (darwin, linux, freebsd)
    std::string vendor; // is a string that represents the vendor (apple)
    std::string endian; // is one of "little", "big", or "pdp"
    int ptrsize;        // is a number that represents how big pointers are in 
                        // bytes on the debug target
  };

  typedef std::string RegisterContent;
  typedef std::string MemoryContent;

  struct Breakpoint
  {
    Breakpoint()
      : pc(0)
    {}
    Breakpoint(int p)
      : pc(p)
    {}

    int pc;
  };

  /*
   * Debugging Interface.
   */
  class DebugInterface
  {
  public:
    virtual ~DebugInterface() {}

    virtual HostInfo GetHostInfo() const = 0;
    virtual RegisterInfo GetRegisterInfo() const = 0;

    virtual RegisterContent GetRegisterContent(int registerNumber) const = 0;
    virtual MemoryContent GetMemoryContent(long address, long length) const = 0;

    virtual bool AddBreakpoint(const Breakpoint& bp) = 0;
    virtual bool RemoveBreakpoint(Breakpoint bp) = 0;

    virtual void SingleStep() = 0;
  };

}
#endif // PATMOS_DEBUG_INTERFACE_H

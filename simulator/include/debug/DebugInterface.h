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
  /// Holds information about the host (machine), on which the target 
  /// (=debuggee) is running.
  struct HostInfo
  {
    HostInfo()
      : cputype(0), cpusubtype(0), triple(""), ostype("unknown"), 
        vendor("unknown"), endian("little"), ptrsize(0)
    {};

    int cputype;        ///< is a number that is the mach-o CPU type that is 
                        ///< being debugged (if available)
    int cpusubtype;     ///< is a number that is the mach-o CPU subtype type 
                        ///< that is being debugged (if available)
    std::string triple; ///< target triple. Use this instead ostype and vendor.
    std::string ostype; ///< is a string the represents the OS being debugged 
                        ///< (darwin, linux, freebsd)
    std::string vendor; ///< is a string that represents the vendor (apple)
    std::string endian; ///< is one of "little", "big", or "pdp"
    int ptrsize;        ///< is a number that represents how big pointers are in 
                        ///< bytes on the debug target
  };

  typedef std::string RegisterContent;
  typedef std::string MemoryContent;

  /// Holds information about a single breakpoint.
  struct Breakpoint
  {
    Breakpoint()
      : pc(0)
    {}
    Breakpoint(int p)
      : pc(p)
    {}

    int pc; ///< program counter where the breakpoint is set
  };

  /// Debugging interface to access the target (=debuggee). Provides functions
  /// to query the type of the host, query register and memory contents, set
  /// breakpoints and initiate single steps.
  /// A debugging client (=debugger) will use this interface to query and change
  /// the state of the target.
  /// Note, that there is no function to transfer control to the target, and
  /// none of the functions will take control on their own. The control of the
  /// target is driven by the debug client alone. See DebugClient.
  class DebugInterface
  {
  public:
    virtual ~DebugInterface() {}

    /// @return (meta) information about the host
    virtual HostInfo GetHostInfo() const = 0;

    /// @return (meta) information about the registers of the host
    virtual RegisterInfoVec GetRegisterInfo() const = 0;

    /// Read register contents of the given register.
    /// @param registerNumber the 0-based number of the register to read content
    ///          from. register numbers are element-indizes of the register
    ///          information that can be retrieved via GetRegisterInfo().
    /// @return the content of the given register. RegisterContent() if the
    ///          register number is out of bounds.
    virtual RegisterContent GetRegisterContent(int registerNumber) const = 0;

    /// Read memory contents at the given address, using the given length.
    /// @param address memory address to start reading. Must not be aligned.
    /// @param length  number of bytes to read.
    /// @return memory content of the given length.
    virtual MemoryContent GetMemoryContent(long address, long length) const = 0;

    /// Add the given breakpoint. The breakpoint will be active right after
    /// adding it.
    /// @param bp breakpoint to be added.
    /// @return True if the breakpoint did not exist already and was added 
    ///         successfully, False otherwise.
    virtual bool AddBreakpoint(const Breakpoint& bp) = 0;

    /// Remove the given breakpoint.
    /// @param bp the breakpoint to be removed.
    /// @return True if the breakpoint did exist and was removed successfully,
    ///         False otherwise.
    virtual bool RemoveBreakpoint(Breakpoint bp) = 0;

    /// Prepare the target for a single step. This will not actually do a single
    /// machine instruction, instead it will prepare the target to do so, after
    /// the target gets control again. This happens usually right after a call
    /// to this function.
    virtual void SingleStep() = 0;
  };

}
#endif // PATMOS_DEBUG_INTERFACE_H

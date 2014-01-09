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

#include <string>

namespace patmos
{
  class DebugClient;

  class Breakpoint
  {
  public:
    Breakpoint(int pc);

  private:
    int m_pc;
  };

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

  enum RegisterEncoding
  {
    en_uint = 0, // unsigned integer
    en_sint,     // signed integer
    en_ieee754,  // IEEE 754 float
    en_vector,   // vector register

    en_default = en_uint
  };

  enum RegisterFormat
  {
    fo_binary = 0,
    fo_decimal,
    fo_hex,
    fo_float,
    fo_vector_sint8,
    fo_vector_uint8,
    fo_vector_sint16,
    fo_vector_uint16,
    fo_vector_sint32,
    fo_vector_uint32,
    fo_vector_float32,
    fo_vector_uint128,

    fo_default = fo_binary
  };

  enum RegisterGenericType
  {
    ge_other = 0, // not a generic register
    ge_pc,        // a program counter register
    ge_sp,        // a stack pointer register
    ge_fp,        // a frame pointer register
    ge_ra,        // a return address register
    ge_flags,     // a CPU flags register
    ge_arg1,      // specified for registers that contain function
    ge_arg2,      //    arguments when the argument fits into a register
    ge_arg3,
    ge_arg4,
    ge_arg5,
    ge_arg6,
    ge_arg7,
    ge_arg8,

    ge_default = ge_other
  };

  struct RegisterInfoEntry
  {
    RegisterInfoEntry()
      : name(""), bitsize(0), encoding(en_default), format(fo_default),
        setName(""), dwarfNumber(0), type(ge_default)
    {};

    RegisterInfoEntry(std::string n, int b, RegisterEncoding e, RegisterFormat f,
        std::string s, int d, RegisterGenericType t)
      : name(n), bitsize(b), encoding(e), format(f), setName(s),
        dwarfNumber(d), type(t)
    {};

    std::string name;           // The primary register name as a string 
                                // ("rbp" for example).
    int bitsize;                // Size in bits of a register (32, 64, etc).
    RegisterEncoding encoding;  // The encoding type of the register.
    RegisterFormat format;      // The preferred format for display of this 
                                // register.
    std::string setName;        // The register set name as a string that this
                                // register belongs to.
    int dwarfNumber;            // The DWARF register number for this register
                                // that is used for this register in the debug
                                // information.
    RegisterGenericType type;   // If the register is a generic register that 
                                // most CPUs have, classify it correctly so the
                                // debugger knows. Omit if it is not a generic 
                                // register.
  };

  struct RegisterInfo
  {
    RegisterInfo()
      : regCount(0), registers()
    {};

    int regCount;                   // Number of registers in the registers 
                                    // array.
    RegisterInfoEntry *registers;   // Array of register infos.
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

    virtual void SetDebugClient(DebugClient &debugClient) = 0;
    virtual void AddBreakpoint(Breakpoint bp) = 0;
    virtual void RemoveBreakpoint(Breakpoint bp) = 0;
  };

}
#endif // PATMOS_DEBUG_INTERFACE_H

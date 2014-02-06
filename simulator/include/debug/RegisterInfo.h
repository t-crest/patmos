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
//  register information for debugging purposes
//

#ifndef PATMOS_REGISTER_INFO_H
#define PATMOS_REGISTER_INFO_H

#include <string>
#include <vector>

namespace patmos
{
  /// The encoding of a register
  enum RegisterEncoding
  {
    en_uint = 0, ///< unsigned integer
    en_sint,     ///< signed integer
    en_ieee754,  ///< IEEE 754 float
    en_vector,   ///< vector register

    en_default = en_uint
  };

  /// The format of a register
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

  /// The generic type of a register
  enum RegisterGenericType
  {
    ge_other = 0, ///< not a generic register
    ge_pc,        ///< a program counter register
    ge_sp,        ///< a stack pointer register
    ge_fp,        ///< a frame pointer register
    ge_ra,        ///< a return address register
    ge_flags,     ///< a CPU flags register
    ge_arg1,      ///< specified for registers that contain function
    ge_arg2,      ///<    arguments when the argument fits into a register
    ge_arg3,
    ge_arg4,
    ge_arg5,
    ge_arg6,
    ge_arg7,
    ge_arg8,

    ge_default = ge_other
  };

  const int noDwarf = -1; ///< used if no dwarf number is known

  /// Holds information about a single register.
  struct RegisterInfo
  {
    RegisterInfo()
      : name(""), bitsize(0), offset(0), encoding(en_default), format(fo_default),
        setName(""), dwarfNumber(noDwarf), type(ge_default)
    {};

    RegisterInfo(std::string n, int b, RegisterEncoding e, RegisterFormat f,
        std::string s, int d, RegisterGenericType t)
      : name(n), bitsize(b), offset(0), encoding(e), format(f), setName(s),
        dwarfNumber(d), type(t)
    {};

    std::string name;           ///< The primary register name as a string 
                                ///< ("rbp" for example).
    int bitsize;                ///< Size in bits of a register (32, 64, etc).
    int offset;                 ///< The offset within the "g" and "G" packet of 
                                ///< the register data for this register. This
                                ///< is the byte offset once the data has been
                                ///< transformed into binary, not the character 
                                ///< offset into the g/G packet.
    RegisterEncoding encoding;  ///< The encoding type of the register.
    RegisterFormat format;      ///< The preferred format for display of this 
                                ///< register.
    std::string setName;        ///< The register set name as a string that this
                                ///< register belongs to.
    int dwarfNumber;            ///< The DWARF register number for this register
                                ///< that is used for this register in the debug
                                ///< information.
    RegisterGenericType type;   ///< If the register is a generic register that 
                                ///< most CPUs have, classify it correctly so the
                                ///< debugger knows. Omit if it is not a generic 
                                ///< register.
  };

  typedef std::vector<RegisterInfo> RegisterInfoVec;
}

#endif // PATMOS_REGISTER_INFO_H

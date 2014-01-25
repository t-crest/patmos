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
//  Implements patmos register information for debugging purposes.
//
#include "debug/PatmosRegisterInfo.h"

#include <boost/assign/list_of.hpp>

namespace
{
  using namespace patmos;

  std::vector<RegisterInfoEntry> patmosRegisterInfo = boost::assign::list_of
//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfoEntry("pc" , 32     , en_default , fo_default , "Program Counter"           , noDwarf   , ge_pc))
(RegisterInfoEntry("r0" , 32     , en_default , fo_default , "General Purpose Registers" , 0         , ge_default))
(RegisterInfoEntry("r1" , 32     , en_default , fo_default , "General Purpose Registers" , 1         , ge_default))
(RegisterInfoEntry("r2" , 32     , en_default , fo_default , "General Purpose Registers" , 2         , ge_default))
(RegisterInfoEntry("r3" , 32     , en_default , fo_default , "General Purpose Registers" , 3         , ge_arg1))
(RegisterInfoEntry("r4" , 32     , en_default , fo_default , "General Purpose Registers" , 4         , ge_arg2))
(RegisterInfoEntry("r5" , 32     , en_default , fo_default , "General Purpose Registers" , 5         , ge_arg3))
(RegisterInfoEntry("r6" , 32     , en_default , fo_default , "General Purpose Registers" , 6         , ge_arg4))
(RegisterInfoEntry("r7" , 32     , en_default , fo_default , "General Purpose Registers" , 7         , ge_arg5))
(RegisterInfoEntry("r8" , 32     , en_default , fo_default , "General Purpose Registers" , 8         , ge_arg6))
(RegisterInfoEntry("r9" , 32     , en_default , fo_default , "General Purpose Registers" , 9         , ge_default))
(RegisterInfoEntry("r10", 32     , en_default , fo_default , "General Purpose Registers" , 10        , ge_default))
(RegisterInfoEntry("r11", 32     , en_default , fo_default , "General Purpose Registers" , 11        , ge_default))
(RegisterInfoEntry("r12", 32     , en_default , fo_default , "General Purpose Registers" , 12        , ge_default))
(RegisterInfoEntry("r13", 32     , en_default , fo_default , "General Purpose Registers" , 13        , ge_default))
(RegisterInfoEntry("r14", 32     , en_default , fo_default , "General Purpose Registers" , 14        , ge_default))
(RegisterInfoEntry("r15", 32     , en_default , fo_default , "General Purpose Registers" , 15        , ge_default))
(RegisterInfoEntry("r16", 32     , en_default , fo_default , "General Purpose Registers" , 16        , ge_default))
(RegisterInfoEntry("r17", 32     , en_default , fo_default , "General Purpose Registers" , 17        , ge_default))
(RegisterInfoEntry("r18", 32     , en_default , fo_default , "General Purpose Registers" , 18        , ge_default))
(RegisterInfoEntry("r19", 32     , en_default , fo_default , "General Purpose Registers" , 19        , ge_default))
(RegisterInfoEntry("r20", 32     , en_default , fo_default , "General Purpose Registers" , 20        , ge_default))
(RegisterInfoEntry("r21", 32     , en_default , fo_default , "General Purpose Registers" , 21        , ge_default))
(RegisterInfoEntry("r22", 32     , en_default , fo_default , "General Purpose Registers" , 22        , ge_default))
(RegisterInfoEntry("r23", 32     , en_default , fo_default , "General Purpose Registers" , 23        , ge_default))
(RegisterInfoEntry("r24", 32     , en_default , fo_default , "General Purpose Registers" , 24        , ge_default))
(RegisterInfoEntry("r25", 32     , en_default , fo_default , "General Purpose Registers" , 25        , ge_default))
(RegisterInfoEntry("r26", 32     , en_default , fo_default , "General Purpose Registers" , 26        , ge_default))
(RegisterInfoEntry("r27", 32     , en_default , fo_default , "General Purpose Registers" , 27        , ge_default))
(RegisterInfoEntry("r28", 32     , en_default , fo_default , "General Purpose Registers" , 28        , ge_fp))
(RegisterInfoEntry("r29", 32     , en_default , fo_default , "General Purpose Registers" , 29        , ge_sp))
(RegisterInfoEntry("r30", 32     , en_default , fo_default , "General Purpose Registers" , 30        , ge_default))
(RegisterInfoEntry("r31", 32     , en_default , fo_default , "General Purpose Registers" , 31        , ge_default))

//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfoEntry("s0" , 32     , en_default , fo_default , "Special Purpose Registers" , 0         , ge_default))
(RegisterInfoEntry("s1" , 32     , en_default , fo_default , "Special Purpose Registers" , 32        , ge_default))
(RegisterInfoEntry("s2" , 32     , en_default , fo_default , "Special Purpose Registers" , 33        , ge_default))
(RegisterInfoEntry("s3" , 32     , en_default , fo_default , "Special Purpose Registers" , 34        , ge_default))
(RegisterInfoEntry("s4" , 32     , en_default , fo_default , "Special Purpose Registers" , 35        , ge_default))
(RegisterInfoEntry("s5" , 32     , en_default , fo_default , "Special Purpose Registers" , 36        , ge_default))
(RegisterInfoEntry("s6" , 32     , en_default , fo_default , "Special Purpose Registers" , 37        , ge_default))
(RegisterInfoEntry("s7" , 32     , en_default , fo_default , "Special Purpose Registers" , 38        , ge_default))
(RegisterInfoEntry("s8" , 32     , en_default , fo_default , "Special Purpose Registers" , 39        , ge_default))
(RegisterInfoEntry("s9" , 32     , en_default , fo_default , "Special Purpose Registers" , 40        , ge_default))
(RegisterInfoEntry("s10", 32     , en_default , fo_default , "Special Purpose Registers" , 41        , ge_default))
(RegisterInfoEntry("s11", 32     , en_default , fo_default , "Special Purpose Registers" , 42        , ge_default))
(RegisterInfoEntry("s12", 32     , en_default , fo_default , "Special Purpose Registers" , 43        , ge_default))
(RegisterInfoEntry("s13", 32     , en_default , fo_default , "Special Purpose Registers" , 44        , ge_default))
(RegisterInfoEntry("s14", 32     , en_default , fo_default , "Special Purpose Registers" , 45        , ge_default))
(RegisterInfoEntry("s15", 32     , en_default , fo_default , "Special Purpose Registers" , 46        , ge_default))

//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfoEntry("p0" , 8      , en_vector  , fo_default , "Predicate Registers"       , 50        , ge_default))
(RegisterInfoEntry("p1" , 8      , en_vector  , fo_default , "Predicate Registers"       , 51        , ge_default))
(RegisterInfoEntry("p2" , 8      , en_vector  , fo_default , "Predicate Registers"       , 52        , ge_default))
(RegisterInfoEntry("p3" , 8      , en_vector  , fo_default , "Predicate Registers"       , 53        , ge_default))
(RegisterInfoEntry("p4" , 8      , en_vector  , fo_default , "Predicate Registers"       , 54        , ge_default))
(RegisterInfoEntry("p5" , 8      , en_vector  , fo_default , "Predicate Registers"       , 55        , ge_default))
(RegisterInfoEntry("p6" , 8      , en_vector  , fo_default , "Predicate Registers"       , 56        , ge_default))
(RegisterInfoEntry("p7" , 8      , en_vector  , fo_default , "Predicate Registers"       , 57        , ge_default))
; // end of boost::assign

  void SetRegisterInfoOffsets(RegisterInfo &info)
  {
    int offset = 0;
    for (RegisterInfo::iterator it = info.begin();
        it != info.end(); ++it)
    {
      it->offset = offset;
      offset += it->bitsize;
    }
  }

}

namespace patmos
{

  RegisterInfo CreatePatmosRegisterInfo()
  {
    RegisterInfo newInfo = patmosRegisterInfo; // get a fresh copy
    SetRegisterInfoOffsets(newInfo);
    return newInfo;
  }

}

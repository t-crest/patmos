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

  RegisterInfoVec patmosRegisterInfo = boost::assign::list_of
//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfo("pc" , 32     , en_default , fo_default , "Program Counter"           , noDwarf   , ge_pc))
(RegisterInfo("r0" , 32     , en_default , fo_default , "General Purpose Registers" , 0         , ge_default))
(RegisterInfo("r1" , 32     , en_default , fo_default , "General Purpose Registers" , 1         , ge_default))
(RegisterInfo("r2" , 32     , en_default , fo_default , "General Purpose Registers" , 2         , ge_default))
(RegisterInfo("r3" , 32     , en_default , fo_default , "General Purpose Registers" , 3         , ge_arg1))
(RegisterInfo("r4" , 32     , en_default , fo_default , "General Purpose Registers" , 4         , ge_arg2))
(RegisterInfo("r5" , 32     , en_default , fo_default , "General Purpose Registers" , 5         , ge_arg3))
(RegisterInfo("r6" , 32     , en_default , fo_default , "General Purpose Registers" , 6         , ge_arg4))
(RegisterInfo("r7" , 32     , en_default , fo_default , "General Purpose Registers" , 7         , ge_arg5))
(RegisterInfo("r8" , 32     , en_default , fo_default , "General Purpose Registers" , 8         , ge_arg6))
(RegisterInfo("r9" , 32     , en_default , fo_default , "General Purpose Registers" , 9         , ge_default))
(RegisterInfo("r10", 32     , en_default , fo_default , "General Purpose Registers" , 10        , ge_default))
(RegisterInfo("r11", 32     , en_default , fo_default , "General Purpose Registers" , 11        , ge_default))
(RegisterInfo("r12", 32     , en_default , fo_default , "General Purpose Registers" , 12        , ge_default))
(RegisterInfo("r13", 32     , en_default , fo_default , "General Purpose Registers" , 13        , ge_default))
(RegisterInfo("r14", 32     , en_default , fo_default , "General Purpose Registers" , 14        , ge_default))
(RegisterInfo("r15", 32     , en_default , fo_default , "General Purpose Registers" , 15        , ge_default))
(RegisterInfo("r16", 32     , en_default , fo_default , "General Purpose Registers" , 16        , ge_default))
(RegisterInfo("r17", 32     , en_default , fo_default , "General Purpose Registers" , 17        , ge_default))
(RegisterInfo("r18", 32     , en_default , fo_default , "General Purpose Registers" , 18        , ge_default))
(RegisterInfo("r19", 32     , en_default , fo_default , "General Purpose Registers" , 19        , ge_default))
(RegisterInfo("r20", 32     , en_default , fo_default , "General Purpose Registers" , 20        , ge_default))
(RegisterInfo("r21", 32     , en_default , fo_default , "General Purpose Registers" , 21        , ge_default))
(RegisterInfo("r22", 32     , en_default , fo_default , "General Purpose Registers" , 22        , ge_default))
(RegisterInfo("r23", 32     , en_default , fo_default , "General Purpose Registers" , 23        , ge_default))
(RegisterInfo("r24", 32     , en_default , fo_default , "General Purpose Registers" , 24        , ge_default))
(RegisterInfo("r25", 32     , en_default , fo_default , "General Purpose Registers" , 25        , ge_default))
(RegisterInfo("r26", 32     , en_default , fo_default , "General Purpose Registers" , 26        , ge_default))
(RegisterInfo("r27", 32     , en_default , fo_default , "General Purpose Registers" , 27        , ge_default))
(RegisterInfo("r28", 32     , en_default , fo_default , "General Purpose Registers" , 28        , ge_fp))
(RegisterInfo("r29", 32     , en_default , fo_default , "General Purpose Registers" , 29        , ge_sp))
(RegisterInfo("r30", 32     , en_default , fo_default , "General Purpose Registers" , 30        , ge_default))
(RegisterInfo("r31", 32     , en_default , fo_default , "General Purpose Registers" , 31        , ge_default))

//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfo("s0" , 32     , en_default , fo_default , "Special Purpose Registers" , 0         , ge_default))
(RegisterInfo("s1" , 32     , en_default , fo_default , "Special Purpose Registers" , 32        , ge_default))
(RegisterInfo("s2" , 32     , en_default , fo_default , "Special Purpose Registers" , 33        , ge_default))
(RegisterInfo("s3" , 32     , en_default , fo_default , "Special Purpose Registers" , 34        , ge_default))
(RegisterInfo("s4" , 32     , en_default , fo_default , "Special Purpose Registers" , 35        , ge_default))
(RegisterInfo("s5" , 32     , en_default , fo_default , "Special Purpose Registers" , 36        , ge_default))
(RegisterInfo("s6" , 32     , en_default , fo_default , "Special Purpose Registers" , 37        , ge_default))
(RegisterInfo("s7" , 32     , en_default , fo_default , "Special Purpose Registers" , 38        , ge_default))
(RegisterInfo("s8" , 32     , en_default , fo_default , "Special Purpose Registers" , 39        , ge_default))
(RegisterInfo("s9" , 32     , en_default , fo_default , "Special Purpose Registers" , 40        , ge_default))
(RegisterInfo("s10", 32     , en_default , fo_default , "Special Purpose Registers" , 41        , ge_default))
(RegisterInfo("s11", 32     , en_default , fo_default , "Special Purpose Registers" , 42        , ge_default))
(RegisterInfo("s12", 32     , en_default , fo_default , "Special Purpose Registers" , 43        , ge_default))
(RegisterInfo("s13", 32     , en_default , fo_default , "Special Purpose Registers" , 44        , ge_default))
(RegisterInfo("s14", 32     , en_default , fo_default , "Special Purpose Registers" , 45        , ge_default))
(RegisterInfo("s15", 32     , en_default , fo_default , "Special Purpose Registers" , 46        , ge_default))

//                 name , regsize, encoding   , format     , setName                     , dwarfNumber, RegisterType
(RegisterInfo("p0" , 8      , en_vector  , fo_default , "Predicate Registers"       , 50        , ge_default))
(RegisterInfo("p1" , 8      , en_vector  , fo_default , "Predicate Registers"       , 51        , ge_default))
(RegisterInfo("p2" , 8      , en_vector  , fo_default , "Predicate Registers"       , 52        , ge_default))
(RegisterInfo("p3" , 8      , en_vector  , fo_default , "Predicate Registers"       , 53        , ge_default))
(RegisterInfo("p4" , 8      , en_vector  , fo_default , "Predicate Registers"       , 54        , ge_default))
(RegisterInfo("p5" , 8      , en_vector  , fo_default , "Predicate Registers"       , 55        , ge_default))
(RegisterInfo("p6" , 8      , en_vector  , fo_default , "Predicate Registers"       , 56        , ge_default))
(RegisterInfo("p7" , 8      , en_vector  , fo_default , "Predicate Registers"       , 57        , ge_default))
; // end of boost::assign

  void SetRegisterInfoOffsets(RegisterInfoVec &info)
  {
    int offset = 0;
    for (RegisterInfoVec::iterator it = info.begin();
        it != info.end(); ++it)
    {
      it->offset = offset;
      offset += it->bitsize;
    }
  }

}

namespace patmos
{

  RegisterInfoVec CreatePatmosRegisterInfo()
  {
    RegisterInfoVec newInfo = patmosRegisterInfo; // get a fresh copy
    SetRegisterInfoOffsets(newInfo);
    return newInfo;
  }

}

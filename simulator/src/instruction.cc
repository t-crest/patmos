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
// This is a tiny assembler embedded into C++ using operator overloading that
// allows to write small test programs.
//

#include "instruction.h"

namespace patmos
{
  instruction_data_t::instruction_data_t() : I(NULL), Bundle_end(true)
  {
  }

  instruction_data_t::instruction_data_t(const instruction_t &i,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end)
  {
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         GPR_e rd, GPR_e rs1, GPR_e rs2,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.RRR.Rd = rd;
    OPS.RRR.Rs1 = rs1;
    OPS.RRR.Rs2 = rs2;
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         GPR_e rd, GPR_e rs1, word_t imm2,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.RRI.Rd = rd;
    OPS.RRI.Rs1 = rs1;
    OPS.RRI.Imm2 = imm2;
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         PRR_e pd, GPR_e rs1, GPR_e rs2,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.CMP.Pd = pd;
    OPS.CMP.Rs1 = rs1;
    OPS.CMP.Rs2 = rs2;
  }
  
  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         word_t imm, bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.I.Imm = imm;
  }
  
  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         GPR_e ra, bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.JSR.Ra = ra;
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred,
                                         GPR_e ra, word_t imm, GPR_e rs,
                                         bool bundle_end) :
      I(&i), Bundle_end(bundle_end), Pred(pred)
  {
    OPS.ST.Ra = ra;
    OPS.ST.Imm = imm;
    OPS.ST.Rs = rs;
  }
}


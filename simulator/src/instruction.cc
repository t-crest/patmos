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
// Basic definitions of interfaces to simulation functions for Patmos
// instructions.
//

#include "instruction.h"

namespace patmos
{
  instruction_data_t::instruction_data_t() : I(NULL)
  {
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred) :
      I(&i), Pred(pred)
  {
  }

  instruction_data_t instruction_data_t::mk_ALUil(const instruction_t &i,
                                                  PRR_e pred, GPR_e rd,
                                                  GPR_e rs1, word_t imm2)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUil.Rd = rd;
    result.OPS.ALUil.Rs1 = rs1;
    result.OPS.ALUil.Imm2 = imm2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_ALUr(const instruction_t &i,
                                                 PRR_e pred, GPR_e rd,
                                                 GPR_e rs1, GPR_e rs2)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUr.Rd = rd;
    result.OPS.ALUr.Rs1 = rs1;
    result.OPS.ALUr.Rs2 = rs2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_ALUu(const instruction_t &i,
                                                 PRR_e pred, GPR_e rd,
                                                 GPR_e rs1)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUu.Rd = rd;
    result.OPS.ALUu.Rs1 = rs1;
    return result;
  }

  instruction_data_t instruction_data_t::mk_ALUm(const instruction_t &i,
                                                 PRR_e pred, GPR_e rs1,
                                                 GPR_e rs2)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUm.Rs1 = rs1;
    result.OPS.ALUm.Rs2 = rs2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_ALUc(const instruction_t &i,
                                                 PRR_e pred, PRR_e pd,
                                                 GPR_e rs1, GPR_e rs2)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUc.Pd = pd;
    result.OPS.ALUc.Rs1 = rs1;
    result.OPS.ALUc.Rs2 = rs2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_ALUp(const instruction_t &i,
                                                 PRR_e pred, PRR_e pd,
                                                 PRR_e ps1, PRR_e ps2)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUp.Pd = pd;
    result.OPS.ALUp.Ps1 = ps1;
    result.OPS.ALUp.Ps2 = ps2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_SPCn(const instruction_t &i,
                                                 PRR_e pred, word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.SPCn.Imm = imm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_SPCw(const instruction_t &i,
                                                 PRR_e pred)
  {
    return instruction_data_t(i, pred);
  }

  instruction_data_t instruction_data_t::mk_SPCt(const instruction_t &i,
                                                 PRR_e pred, SPR_e sd,
                                                 GPR_e rs1)
  {
    instruction_data_t result(i, pred);
    result.OPS.SPCt.Sd = sd;
    result.OPS.SPCt.Rs1 = rs1;
    return result;
  }

  instruction_data_t instruction_data_t::mk_SPCf(const instruction_t &i,
                                                 PRR_e pred, GPR_e rd, SPR_e ss)
  {
    instruction_data_t result(i, pred);
    result.OPS.SPCf.Rd = rd;
    result.OPS.SPCf.Ss = ss;
    return result;
  }

  instruction_data_t instruction_data_t::mk_LDT(const instruction_t &i,
                                                PRR_e pred, GPR_e rd, GPR_e ra,
                                                word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.LDT.Rd = rd;
    result.OPS.LDT.Ra = ra;
    result.OPS.LDT.Imm = imm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_STT(const instruction_t &i,
                                                PRR_e pred, GPR_e ra, GPR_e rs1,
                                                word_t imm2)
  {
    instruction_data_t result(i, pred);
    result.OPS.STT.Ra = ra;
    result.OPS.STT.Rs1 = rs1;
    result.OPS.STT.Imm2 = imm2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_STC(const instruction_t &i,
                                                PRR_e pred, word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.STC.Imm = imm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_PFLb(const instruction_t &i,
                                                 PRR_e pred, word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.PFLb.Imm = imm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_PFLi(const instruction_t &i,
                                                 PRR_e pred, GPR_e rs)
  {
    instruction_data_t result(i, pred);
    result.OPS.PFLi.Rs = rs;
    return result;
  }

  instruction_data_t instruction_data_t::mk_PFLr(const instruction_t &i,
                                                 PRR_e pred)
  {
    return instruction_data_t(i, pred);
  }

  instruction_data_t instruction_data_t::mk_HLT(const instruction_t &i)
  {
    return instruction_data_t(i, p0);
  }
}


/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

//
// Basic definitions of interfaces to simulation functions for Patmos
// instructions.
//

#include "instruction.h"

namespace patmos
{
  instruction_data_t::instruction_data_t() : I(NULL), Pred(pn0), Address(0)
  {
  }

  instruction_data_t::instruction_data_t(const instruction_t &i, PRR_e pred) :
    I(&i), Pred(pred), Address(0)
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

  instruction_data_t instruction_data_t::mk_ALUci(const instruction_t &i,
                                                 PRR_e pred, PRR_e pd,
                                                 GPR_e rs1, uword_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUci.Pd = pd;
    result.OPS.ALUci.Rs1 = rs1;
    result.OPS.ALUci.Imm = imm;
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

  instruction_data_t instruction_data_t::mk_ALUb(const instruction_t &i,
                                                 PRR_e pred, GPR_e rd,
                                                 GPR_e rs1, uword_t imm, PRR_e ps)
  {
    instruction_data_t result(i, pred);
    result.OPS.ALUb.Rd = rd;
    result.OPS.ALUb.Rs1 = rs1;
    result.OPS.ALUb.Imm = imm;
    result.OPS.ALUb.Ps = ps;
    return result;
  }

  instruction_data_t instruction_data_t::mk_SPCn(const instruction_t &i,
                                                 PRR_e pred, word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.SPCn.Imm = imm;
    return result;
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

  instruction_data_t instruction_data_t::mk_STCi(const instruction_t &i,
                                                PRR_e pred, word_t imm)
  {
    instruction_data_t result(i, pred);
    result.OPS.STCi.Imm = imm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_STCr(const instruction_t &i,
                                                PRR_e pred, GPR_e rs)
  {
    instruction_data_t result(i, pred);
    result.OPS.STCr.Rs = rs;
    return result;
  }

  instruction_data_t instruction_data_t::mk_CFLi(const instruction_t &i,
                                                 PRR_e pred, word_t flag,
                                                 word_t imm, uword_t uimm)
  {
    instruction_data_t result(i, pred);
    result.OPS.CFLi.D = flag;
    result.OPS.CFLi.Imm = imm;
    result.OPS.CFLi.UImm = uimm;
    return result;
  }

  instruction_data_t instruction_data_t::mk_CFLri(const instruction_t &i,
                                                  PRR_e pred, word_t flag)
  {
    instruction_data_t result(i, pred);
    result.OPS.CFLri.D = flag;
    return result;
  }

  instruction_data_t instruction_data_t::mk_CFLrs(const instruction_t &i,
                                                  PRR_e pred, word_t flag,
                                                  GPR_e rs)
  {
    instruction_data_t result(i, pred);
    result.OPS.CFLrs.D = flag;
    result.OPS.CFLrs.Rs = rs;
    return result;
  }

  instruction_data_t instruction_data_t::mk_CFLrt(const instruction_t &i,
                                                  PRR_e pred, word_t flag,
                                                  GPR_e rs1, GPR_e rs2)
  {
    instruction_data_t result(i, pred);
    result.OPS.CFLrt.D = flag;
    result.OPS.CFLrt.Rs1 = rs1;
    result.OPS.CFLrt.Rs2 = rs2;
    return result;
  }

  instruction_data_t instruction_data_t::mk_HLT(const instruction_t &i)
  {
    return instruction_data_t(i, p0);
  }
}


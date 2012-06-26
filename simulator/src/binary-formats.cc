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
// Concrete definition of instruction formats, defining the binary encoding of
// concrete instructions.
//


#include "binary-formats.h"
#include "instructions.h"
#include "util.h"

#include "boost/utility/binary.hpp"

namespace patmos
{
  typedef std::vector<const binary_format_t *> formats_t;

  /// Extract a given number of bits form a given start position in a data
  /// word -- unsigned.
  /// @param from The data word.
  /// @param start The start offset.
  /// @param width The number of bits to extract.
  static uword_t extract(uword_t from, unsigned int start, unsigned int width)
  {
    return (from >> start) & ((1 << width) - 1);
  }

  /// Extract a given number of bits form a given start position in a data
  /// word -- signed.
  /// @param from The data word.
  /// @param start The start offset.
  /// @param width The number of bits to extract.
  static word_t extractS(word_t from, unsigned int start, unsigned int width)
  {
    word_t shift = sizeof(word_t)*8 - width;
    return ((from >> start) << shift) >> shift;
  }

  static GPR_e extractG(uword_t from, unsigned int start)
  {
    return (GPR_e)extract(from, start, 5);
  }

  static PRR_e extractPN(uword_t from, unsigned int start)
  {
    return (PRR_e)extract(from, start, 4);
  }

  static PRR_e extractP(uword_t from, unsigned int start)
  {
    return (PRR_e)extract(from, start, 3);
  }

  static SPR_e extractS(uword_t from, unsigned int start)
  {
    return (SPR_e)extract(from, start, 4);
  }

  static uword_t insert(uword_t in, unsigned int start, unsigned int width,
                        uword_t pattern)
  {
    assert(fitu(pattern, width));
    return in | (pattern << start);
  }

  static void insertV(word_t &iw, unsigned int start, unsigned int width,
                      word_t pattern)
  {
    assert(fitu(pattern, width));
    iw = iw | (pattern << start);
  }

  static void insertVs(word_t &iw, unsigned int start, unsigned int width,
                       word_t pattern)
  {
    assert(fits(pattern, width));
    iw = iw | ( (pattern & ((1 << width) - 1)) << start);
  }

  static void insertG(word_t &iw, unsigned int start, word_t pattern)
  {
    iw = insert(iw, start, 5, pattern);
  }

  static void insertPN(word_t &iw, unsigned int start, word_t pattern)
  {
    iw = insert(iw, start, 4, pattern);
  }

  static void insertP(word_t &iw, unsigned int start, word_t pattern)
  {
    iw = insert(iw, start, 3, pattern);
  }

  static void insertS(word_t &iw, unsigned int start, word_t pattern)
  {
    iw = insert(iw, start, 4, pattern);
  }

  alui_format_t::alui_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00000, insert(0, 22, 3, opcode), 3)
  {
  }

  instruction_data_t alui_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    word_t imm = extract(iw, 0, 12);
    GPR_e rs1 = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUil(Instruction, pred, rd, rs1, imm);
  }

  word_t alui_format_t::encode(word_t pred, word_t opcode, word_t rd,
                               word_t rs1, word_t imm)
  {
    word_t iw = 0;

    assert(fitu(opcode, 3) && isGPR(rd) && isGPR(rs1) && fitu(imm, 12));

    insertV(iw, 0, 12, imm);
    insertG(iw, 12, rs1);
    insertG(iw, 17, rd);
    insertV(iw, 22, 3, opcode);
    insertPN(iw, 27, pred);

    return iw;
  }

  alul_format_t::alul_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x87C0007F, insert(0x87C00000, 0, 4, opcode),
                      1, true)
  {
  }

  instruction_data_t alul_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs1 = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUil(Instruction, pred, rd, rs1, longimm);
  }

  dword_t alul_format_t::encode(word_t pred, word_t opcode, word_t rd,
                               word_t rs1, word_t imm)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isGPR(rd) && isGPR(rs1));

    insertV(iw, 0, 4, opcode);
    insertG(iw, 12, rs1);
    insertG(iw, 17, rd);
    insertV(iw, 22, 5, BOOST_BINARY(11111));
    insertPN(iw, 27, pred);
    insertV(iw, 31, 1, 1);

    return ((dword_t)iw) << 32 | imm;
  }

  alur_format_t::alur_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2000000, 0, 4, opcode),
                      3)
  {
  }

  instruction_data_t alur_format_t::decode_operands(word_t iw,
                                                           word_t longimm) const
  {
    GPR_e rs2 = extractG(iw, 7);
    GPR_e rs1 = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUr(Instruction, pred, rd, rs1, rs2);
  }

  word_t alur_format_t::encode(word_t pred, word_t opcode, word_t rd,
                               word_t rs1, word_t rs2)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isGPR(rd) && isGPR(rs1) && isGPR(rs2));

    insertV(iw, 0, 4, opcode);
    insertG(iw, 7, rs2);
    insertG(iw, 12, rs1);
    insertG(iw, 17, rd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, pred);

    return iw;
  }

  aluu_format_t::aluu_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2000010, 0, 4, opcode),
                      3)
  {
  }

  instruction_data_t aluu_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs1 = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUu(Instruction, pred, rd, rs1);
  }

  word_t aluu_format_t::encode(word_t pred, word_t opcode, word_t rd,
                               word_t rs1)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isGPR(rd) && isGPR(rs1));

    insertV(iw, 0, 4, opcode);
    insertV(iw, 4, 3, BOOST_BINARY(001));
    insertG(iw, 12, rs1);
    insertG(iw, 17, rd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, pred);

    return iw;
  }

  alum_format_t::alum_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2000020, 0, 4, opcode),
                      3)
  {
  }

  instruction_data_t alum_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs2 = extractG(iw, 7);
    GPR_e rs1 = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUm(Instruction, pred, rs1, rs2);
  }

  word_t alum_format_t::encode(word_t pred, word_t opcode, word_t rs1,
                               word_t rs2)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isGPR(rs1) && isGPR(rs2));

    insertV(iw, 0, 4, opcode);
    insertV(iw, 4, 3, BOOST_BINARY(010));
    insertG(iw, 7, rs2);
    insertG(iw, 12, rs1);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, pred);

    return iw;
  }

  aluc_format_t::aluc_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2000030, 0, 4, opcode),
                      3)
  {
  }

  instruction_data_t aluc_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs2 = extractG(iw, 7);
    GPR_e rs1 = extractG(iw, 12);
    PRR_e pd = extractP(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUc(Instruction, pred, pd, rs1, rs2);
  }

  word_t aluc_format_t::encode(word_t pred, word_t opcode, word_t pd,
                               word_t rs1, word_t rs2)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isPRR(pd) && isGPR(rs1) && isGPR(rs2));

    insertV(iw, 0, 4, opcode);
    insertV(iw, 4, 3, BOOST_BINARY(011));
    insertG(iw, 7, rs2);
    insertG(iw, 12, rs1);
    insertP(iw, 17, pd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, pred);

    return iw;
  }

  alup_format_t::alup_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2000040, 0, 4, opcode),
                      3)
  {
  }

  instruction_data_t alup_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    PRR_e ps2 = extractPN(iw, 7);
    PRR_e ps1 = extractPN(iw, 12);
    PRR_e pd = extractP(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_ALUp(Instruction, pred, pd, ps1, ps2);
  }

  word_t alup_format_t::encode(word_t pred, word_t opcode, word_t pd,
                               word_t ps1, word_t ps2)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isPRR(pd) && isPRRn(ps1) && isPRRn(ps2));

    insertV(iw, 0, 4, opcode);
    insertV(iw, 4, 3, BOOST_BINARY(100));
    insertPN(iw, 7, ps2);
    insertPN(iw, 12, ps1);
    insertP(iw, 17, pd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, pred);

    return iw;
  }

  spcn_format_t::spcn_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00070, 0x2400000, 1)
  {
  }

  instruction_data_t spcn_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    word_t imm = extract(iw, 0, 4);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_SPCn(Instruction, pred, imm);
  }

  word_t spcn_format_t::encode(word_t pred, word_t imm)
  {
    word_t iw = 0;

    assert(fitu(imm, 4));

    insertV(iw, 0, 4, imm);
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, pred);

    return iw;
  }

  spcw_format_t::spcw_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0007F, insert(0x2400010, 0, 4, opcode),
                      1)
  {
  }

  instruction_data_t spcw_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_SPCw(Instruction, pred);
  }

  word_t spcw_format_t::encode(word_t pred, word_t opcode)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && opcode == 0);

    insertV(iw, 0, 4, opcode);
    insertV(iw, 4, 3, BOOST_BINARY(001));
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, pred);

    return iw;
  }

  spct_format_t::spct_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00070, 0x2400020, 3)
  {
  }

  instruction_data_t spct_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    SPR_e sd = extractS(iw, 0);
    GPR_e rs = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_SPCt(Instruction, pred, sd, rs);
  }

  word_t spct_format_t::encode(word_t pred, word_t sd, word_t rs1)
  {
    word_t iw = 0;

    assert(isSPR(sd) && isGPR(rs1));

    insertS(iw, 0, sd);
    insertV(iw, 4, 3, BOOST_BINARY(010));
    insertG(iw, 12, rs1);
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, pred);

    return iw;
  }

  spcf_format_t::spcf_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00070, 0x2400030, 3)
  {
  }

  instruction_data_t spcf_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    SPR_e ss = extractS(iw, 0);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_SPCf(Instruction, pred, rd, ss);
  }

  word_t spcf_format_t::encode(word_t pred, word_t rd, word_t ss)
  {
    word_t iw = 0;

    assert(isGPR(rd) && isSPR(ss));

    insertS(iw, 0, ss);
    insertV(iw, 4, 3, BOOST_BINARY(011));
    insertG(iw, 17, rd);
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, pred);

    return iw;
  }

  ldt_format_t::ldt_format_t(const instruction_t &instruction, word_t opcode,
                             bool is_stack) :
      binary_format_t(instruction, 0x7C00F80, insert(0x2800000, 7, 5, opcode),
                      is_stack ? 3 : 1)
  {
  }

  instruction_data_t ldt_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    word_t imm = extractS(iw, 0, 7);
    GPR_e ra = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_LDT(Instruction, pred, rd, ra, imm);
  }

  word_t ldt_format_t::encode(word_t pred, word_t opcode, word_t rd, word_t ra,
                              word_t imm)
  {
    word_t iw = 0;

    assert(fitu(opcode, 5) && isGPR(rd) && isGPR(ra) && fits(imm, 7));

    insertVs(iw, 0, 7, imm);
    insertV(iw, 7, 5, opcode);
    insertG(iw, 12, ra);
    insertG(iw, 17, rd);
    insertV(iw, 22, 5, BOOST_BINARY(01010));
    insertPN(iw, 27, pred);

    return iw;
  }

  word_t ldt_format_t::encode(word_t pred, word_t opcode, word_t ra, word_t imm)
  {
    return encode(pred, opcode, 0, ra, imm);
  }

  stt_format_t::stt_format_t(const instruction_t &instruction, word_t opcode,
                             bool is_stack) :
      binary_format_t(instruction, 0x7FE0000, insert(0x2C00000, 17, 5, opcode),
                      is_stack ? 3 : 1)
  {
  }

  instruction_data_t stt_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    word_t imm = extractS(iw, 0, 7);
    GPR_e rs = extractG(iw, 7);
    GPR_e ra = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_STT(Instruction, pred, ra, rs, imm);
  }

  word_t stt_format_t::encode(word_t pred, word_t opcode, word_t ra, word_t imm,
                              word_t rs)
  {
    word_t iw = 0;

    assert(fitu(opcode, 5) && isGPR(ra) && fits(imm, 7) && isGPR(rs));

    insertVs(iw, 0, 7, imm);
    insertG(iw, 7, rs);
    insertG(iw, 12, ra);
    insertV(iw, 17, 5, opcode);
    insertV(iw, 22, 5, BOOST_BINARY(01011));
    insertPN(iw, 27, pred);

    return iw;
  }

  stc_format_t::stc_format_t(const instruction_t &instruction, word_t opcode) :
      binary_format_t(instruction, 0x7C00000, insert(0x3000000, 22, 2, opcode),
                      1)
  {
  }

  instruction_data_t stc_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    word_t imm = extract(iw, 0, 22);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_STC(Instruction, pred, imm);
  }

  word_t stc_format_t::encode(word_t pred, word_t opcode, word_t imm)
  {
    word_t iw = 0;

    assert(fitu(opcode, 2) && fitu(imm, 22));

    insertV(iw, 0, 22, imm);
    insertV(iw, 22, 2, opcode);
    insertV(iw, 24, 3, BOOST_BINARY(011));
    insertPN(iw, 27, pred);

    return iw;
  }

  pflb_format_t::pflb_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00000, insert(0x6000000, 22, 2, opcode),
                      1)
  {
  }

  instruction_data_t pflb_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    word_t imm = extract(iw, 0, 22);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_PFLb(Instruction, pred, imm);
  }

  word_t pflb_format_t::encode(word_t pred, word_t opcode, word_t imm)
  {
    word_t iw = 0;

    assert(fitu(opcode, 2) && fitu(imm, 22));

    insertV(iw, 0, 22, imm);
    insertV(iw, 22, 2, opcode);
    insertV(iw, 24, 3, BOOST_BINARY(110));
    insertPN(iw, 27, pred);

    return iw;
  }

  pfli_format_t::pfli_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0000F, insert(0x7000000, 0, 4, opcode),
                      1)
  {
  }

  instruction_data_t pfli_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_PFLi(Instruction, pred, rs);
  }

  word_t pfli_format_t::encode(word_t pred, word_t opcode, word_t rs1)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4) && isGPR(rs1));

    insertV(iw, 0, 4, opcode);
    insertG(iw, 12, rs1);
    insertV(iw, 22, 5, BOOST_BINARY(11100));
    insertPN(iw, 27, pred);

    return iw;
  }

  pflr_format_t::pflr_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0000F, insert(0x7800000, 0, 4, opcode),
                      1)
  {
  }

  instruction_data_t pflr_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_PFLr(Instruction, pred);
  }

  word_t pflr_format_t::encode(word_t pred, word_t opcode)
  {
    word_t iw = 0;

    assert(fitu(opcode, 4));

    insertV(iw, 0, 4, opcode);
    insertV(iw, 22, 5, BOOST_BINARY(11110));
    insertPN(iw, 27, pred);

    return iw;
  }

  bne_format_t::bne_format_t(const instruction_t &instruction, word_t opcode) :
      binary_format_t(instruction, 0x87C00000, 0x7C00000, 1)
  {
  }

  instruction_data_t bne_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    word_t imm = extractS(iw, 0, 7);
    GPR_e rs2 = extractG(iw, 7);
    GPR_e rs1 = extractG(iw, 12);
    return instruction_data_t::mk_BNE(Instruction, rs1, rs2, imm);
 }

  word_t bne_format_t::encode(word_t rs1, word_t rs2, word_t imm)
  {
    word_t iw = 0;

    assert(isGPR(rs1) && isGPR(rs2) && fits(imm, 7));

    insertVs(iw, 0, 7, imm);
    insertG(iw, 7, rs2);
    insertG(iw, 12, rs1);
    insertV(iw, 22, 5, BOOST_BINARY(11111));

    return iw;
  }

  hlt_format_t::hlt_format_t(const instruction_t &instruction, word_t opcode) :
      binary_format_t(instruction, 0xffffffff, 0xffffffff, 1, true)
  {
  }

  instruction_data_t hlt_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    return instruction_data_t::mk_HLT(Instruction);
  }

  word_t hlt_format_t::encode()
  {
    return 0xffffffff;
  }
}

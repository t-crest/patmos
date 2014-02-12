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
#include "symbol.h"
#include "assembler.h"

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

  static void insertV(uword_t &iw, unsigned int start, unsigned int width,
                      uword_t pattern)
  {
    assert(fitu(pattern, width));
    iw = iw | (pattern << start);
  }

  static void insertVs(uword_t &iw, unsigned int start, unsigned int width,
                       uword_t pattern)
  {
    assert(fits(pattern, width));
    iw = iw | ( (pattern & ((1 << width) - 1)) << start);
  }

  static void insertG(uword_t &iw, unsigned int start, uword_t pattern)
  {
    iw = insert(iw, start, 5, pattern);
  }

  static void insertPN(uword_t &iw, unsigned int start, uword_t pattern)
  {
    iw = insert(iw, start, 4, pattern);
  }

  static void insertP(uword_t &iw, unsigned int start, uword_t pattern)
  {
    iw = insert(iw, start, 3, pattern);
  }

  static void insertS(uword_t &iw, unsigned int start, uword_t pattern)
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

  bool alui_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.ALUil.Rd)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUil.Rs1)) return false;
    if (!parser.match_token(",")) return false;
    
    // ensure that we are not parsing a register name as immediate symbol
    GPR_e tmp;
    if (parser.parse_GPR(tmp)) {
      return false;
    }
    
    reloc.set_format(12);
    
    if (!parser.parse_expression(instr.OPS.ALUil.Imm2, reloc, true)) return false;
    
    if (!fitu(instr.OPS.ALUil.Imm2, 12)) {
      parser.set_error("immediate value too large.");
      return false;
    }
    
    return true;
  }
                                              
  udword_t alui_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    assert(fitu(instr.OPS.ALUil.Imm2, 12));

    insertV(iw, 0, 12, instr.OPS.ALUil.Imm2);
    insertG(iw, 12, instr.OPS.ALUil.Rs1);
    insertG(iw, 17, instr.OPS.ALUil.Rd);
    insertPN(iw, 27, instr.Pred);

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

  bool alul_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.ALUil.Rd)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUil.Rs1)) return false;
    if (!parser.match_token(",")) return false;

    // ensure that we are not parsing a register name as immediate symbol
    GPR_e tmp;
    if (parser.parse_GPR(tmp)) {
      return false;
    }
    
    reloc.set_format(32);
    
    if (!parser.parse_expression(instr.OPS.ALUil.Imm2, reloc, true)) return false;
    return true;
  }
                                              
  udword_t alul_format_t::encode(std::string mnemonic, 
                                const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertG(iw, 12, instr.OPS.ALUil.Rs1);
    insertG(iw, 17, instr.OPS.ALUil.Rd);
    insertV(iw, 22, 5, BOOST_BINARY(11111));
    insertPN(iw, 27, instr.Pred);
    insertV(iw, 31, 1, 1);

    return ((udword_t)iw << 32) | ((udword_t)instr.OPS.ALUil.Imm2 & ((1ULL<<32)-1));
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

  bool alur_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.ALUr.Rd)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUr.Rs1)) return false;
    if (!parser.match_token(",")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUr.Rs2)) return false;
    return true;
  }

  udword_t alur_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertG(iw, 7, instr.OPS.ALUr.Rs2);
    insertG(iw, 12, instr.OPS.ALUr.Rs1);
    insertG(iw, 17, instr.OPS.ALUr.Rd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, instr.Pred);

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
  
  bool alum_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.ALUm.Rs1)) return false;
    if (!parser.match_token(",")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUm.Rs2)) return false;
    return true;
  }
                                              
  udword_t alum_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertV(iw, 4, 3, BOOST_BINARY(010));
    insertG(iw, 7, instr.OPS.ALUm.Rs2);
    insertG(iw, 12, instr.OPS.ALUm.Rs1);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, instr.Pred);

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

  bool aluc_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_PRR(instr.OPS.ALUc.Pd, false)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUc.Rs1)) return false;
    if (!parser.match_token(",")) return false;
    if (!parser.parse_GPR(instr.OPS.ALUc.Rs2)) return false;
    return true;
  }
                                              
  udword_t aluc_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertV(iw, 4, 3, BOOST_BINARY(011));
    insertG(iw, 7, instr.OPS.ALUc.Rs2);
    insertG(iw, 12, instr.OPS.ALUc.Rs1);
    insertP(iw, 17, instr.OPS.ALUc.Pd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, instr.Pred);

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

  bool alup_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_PRR(instr.OPS.ALUp.Pd, false)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_PRR(instr.OPS.ALUp.Ps1, true)) return false;
    if (!parser.match_token(",")) return false;
    if (!parser.parse_PRR(instr.OPS.ALUp.Ps2, true)) return false;
    return true;
  }
                                              
  udword_t alup_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertV(iw, 4, 3, BOOST_BINARY(100));
    insertPN(iw, 7, instr.OPS.ALUp.Ps2);
    insertPN(iw, 12, instr.OPS.ALUp.Ps1);
    insertP(iw, 17, instr.OPS.ALUp.Pd);
    insertV(iw, 22, 5, BOOST_BINARY(01000));
    insertPN(iw, 27, instr.Pred);

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

  bool spcw_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {    
    return true;
  }
                                              
  udword_t spcw_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertV(iw, 4, 3, BOOST_BINARY(001));
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, instr.Pred);

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

  bool spct_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_SPR(instr.OPS.SPCt.Sd)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_GPR(instr.OPS.SPCt.Rs1)) return false;
    return true;
  }
                                              
  udword_t spct_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertS(iw, 0, instr.OPS.SPCt.Sd);
    insertV(iw, 4, 3, BOOST_BINARY(010));
    insertG(iw, 12, instr.OPS.SPCt.Rs1);
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, instr.Pred);

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

  bool spcf_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.SPCf.Rd)) return false;
    if (!parser.match_token("=")) return false;
    if (!parser.parse_SPR(instr.OPS.SPCf.Ss)) return false;    
    return true;
  }
                                              
  udword_t spcf_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertS(iw, 0, instr.OPS.SPCf.Ss);
    insertV(iw, 4, 3, BOOST_BINARY(011));
    insertG(iw, 17, instr.OPS.SPCf.Rd);
    insertV(iw, 22, 5, BOOST_BINARY(01001));
    insertPN(iw, 27, instr.Pred);

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
    word_t imm = extract(iw, 0, 7);
    GPR_e ra = extractG(iw, 12);
    GPR_e rd = extractG(iw, 17);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_LDT(Instruction, pred, rd, ra, imm);
  }

  bool ldt_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                    instruction_data_t &instr, 
                                    reloc_info_t &reloc) const
  {
    
    bool decoupled = mnemonic[0] == 'd';
    
    if (decoupled) {
      SPR_e tmp;
      if (!parser.parse_SPR(tmp)) return false;
      if (tmp != sm) {
        parser.set_error("expected special register sm.");
        return false;
      }
      
      instr.OPS.LDT.Rd = r0;
    } else {
      if (!parser.parse_GPR(instr.OPS.LDT.Rd)) return false;
    }
    
    if (!parser.match_token("=")) return false;
    
    if (!parser.match_token("[")) return false;
    
    bool has_imm = false;
    bool negate = false;
    
    if (parser.get_lexer().is_name()) {
      if (!parser.parse_GPR(instr.OPS.LDT.Ra)) return false;      
    } else {
      instr.OPS.LDT.Ra = r0;
      has_imm = true;
    }
    
    if (parser.match_token("+")) {
      has_imm = true;
    }
    if (parser.match_token("-")) {
      has_imm = true;
      negate = true;
    }

    unsigned type_idx = decoupled ? 2 : 1;
    if (mnemonic[type_idx] == 'w') {
      reloc.set_format(7, 0, 2);
    } else if (mnemonic[type_idx] == 'h') {
      reloc.set_format(7, 0, 1);
    } else {
      reloc.set_format(7);
    }
    
    if (has_imm) {
      if (!parser.parse_expression(instr.OPS.LDT.Imm, reloc, true)) return false;
    } else {
      instr.OPS.LDT.Imm = 0;
    }
    
    if (!parser.match_token("]")) return false;
    
    if (!fits(instr.OPS.LDT.Imm, 7)) {
      parser.set_error("immediate value too large.");
      return false;
    }
    
    return true;
  }
                                              
  udword_t ldt_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    assert(fits(instr.OPS.LDT.Imm, 7));

    insertV(iw, 0, 7, instr.OPS.LDT.Imm);
    insertG(iw, 12, instr.OPS.LDT.Ra);
    insertG(iw, 17, instr.OPS.LDT.Rd);
    insertV(iw, 22, 5, BOOST_BINARY(01010));
    insertPN(iw, 27, instr.Pred);

    return iw;
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
    word_t imm = extract(iw, 0, 7);
    GPR_e rs = extractG(iw, 7);
    GPR_e ra = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_STT(Instruction, pred, ra, rs, imm);
  }

  bool stt_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                    instruction_data_t &instr, 
                                    reloc_info_t &reloc) const
  {
    
    if (!parser.match_token("[")) return false;
    
    bool has_imm = false;
    bool negate = false;
    
    if (parser.get_lexer().is_name()) {
      if (!parser.parse_GPR(instr.OPS.STT.Ra)) return false;      
    } else {
      instr.OPS.STT.Ra = r0;
      has_imm = true;
    }
    
    if (parser.match_token("+")) {
      has_imm = true;
    }
    if (parser.match_token("-")) {
      has_imm = true;
      negate = true;
    }
    
    if (mnemonic[1] == 'w') {
      reloc.set_format(7, 0, 2);
    } else if (mnemonic[1] == 'h') {
      reloc.set_format(7, 0, 1);
    } else {
      reloc.set_format(7);
    }
    
    if (has_imm) {
      if (!parser.parse_expression(instr.OPS.STT.Imm2, reloc, true)) return false;
    } else {
      instr.OPS.STT.Imm2 = 0;
    }
    
    if (!parser.match_token("]")) return false;
    
    if (!parser.match_token("=")) return false;

    if (!parser.parse_GPR(instr.OPS.STT.Rs1)) return false;
    
    if (!fits(instr.OPS.STT.Imm2, 7)) {
      parser.set_error("immediate value too large.");
      return false;
    }
    
    return true;
  }
                                              
  udword_t stt_format_t::encode(std::string mnemonic, 
                              const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    assert(fits(instr.OPS.STT.Imm2, 7));

    insertV(iw, 0, 7, instr.OPS.STT.Imm2);
    insertG(iw, 7, instr.OPS.STT.Rs1);
    insertG(iw, 12, instr.OPS.STT.Ra);
    insertV(iw, 22, 5, BOOST_BINARY(01011));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

  stci_format_t::stci_format_t(const instruction_t &instruction, word_t opcode) :
      binary_format_t(instruction, 0x7FC0000, insert(0x3000000, 20, 2, opcode),
                      1)
  {
  }

  instruction_data_t stci_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    word_t imm = extract(iw, 0, 18);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_STCi(Instruction, pred, imm);
  }

  bool stci_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    reloc.set_word_format(22);
    
    if (!parser.parse_expression(instr.OPS.STCi.Imm, reloc, true)) return false;    
    
    if (!fitu(instr.OPS.STCi.Imm, 22)) {
      parser.set_error("immediate value too large.");
      return false;
    }

    return true;
  }
                                              
  udword_t stci_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    assert(fitu(instr.OPS.STCi.Imm, 22));

    insertV(iw, 0, 18, instr.OPS.STCi.Imm);
    insertV(iw, 18, 2, BOOST_BINARY(00));
    insertV(iw, 22, 5, BOOST_BINARY(01100));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

  stcr_format_t::stcr_format_t(const instruction_t &instruction, word_t opcode) :
      binary_format_t(instruction, 0x7FC0000, insert(0x3040000, 20, 2, opcode),
                      1)
  {
  }

  instruction_data_t stcr_format_t::decode_operands(word_t iw,
                                                   word_t longimm) const
  {
    GPR_e rs = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_STCr(Instruction, pred, rs);
  }

  bool stcr_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.STCr.Rs)) return false;    
    return true;
  }
                                              
  udword_t stcr_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertG(iw, 12, instr.OPS.STCr.Rs);
    insertV(iw, 18, 2, BOOST_BINARY(01));
    insertV(iw, 22, 5, BOOST_BINARY(01100));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

  cflb_format_t::cflb_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C00000, insert(0x6000000, 22, 2, opcode),
                      1)
  {
  }

  instruction_data_t cflb_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    word_t imm = extractS(iw, 0, 22);
    uword_t uimm = extract(iw, 0, 22);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_CFLb(Instruction, pred, imm, uimm);
  }

  bool cflb_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (mnemonic == "br") {
      reloc.set_PCRel_format(22);
    } else {
      reloc.set_word_format(22);
    }
    
    if (!parser.parse_expression(instr.OPS.CFLb.Imm, reloc, true)) return false;
    
    if (!fits(instr.OPS.CFLb.Imm, 22)) {
      parser.set_error("immediate value too large.");
      return false;
    }
    
    return true;
  }
                                              
  udword_t cflb_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    assert(fits(instr.OPS.CFLb.Imm, 22));

    insertVs(iw, 0, 22, instr.OPS.CFLb.Imm);
    insertV(iw, 24, 3, BOOST_BINARY(110));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

  cfli_format_t::cfli_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0000F, insert(0x7000000, 0, 4, opcode),
                      1)
  {
  }

  instruction_data_t cfli_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e rs = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_CFLi(Instruction, pred, rs);
  }

  bool cfli_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.CFLi.Rs)) return false;
    return true;
  }
                                              
  udword_t cfli_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertG(iw, 12, instr.OPS.CFLi.Rs);
    insertV(iw, 22, 5, BOOST_BINARY(11100));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

  cflr_format_t::cflr_format_t(const instruction_t &instruction,
                               word_t opcode) :
      binary_format_t(instruction, 0x7C0000F, insert(0x7800000, 0, 4, opcode),
                      1)
  {
  }

  instruction_data_t cflr_format_t::decode_operands(word_t iw,
                                                    word_t longimm) const
  {
    GPR_e ro = extractG(iw,  7);
    GPR_e rb = extractG(iw, 12);
    PRR_e pred = extractPN(iw, 27);
    return instruction_data_t::mk_CFLr(Instruction, pred, rb, ro);
  }

  bool cflr_format_t::parse_operands(line_parser_t &parser, std::string mnemonic, 
                                     instruction_data_t &instr, 
                                     reloc_info_t &reloc) const
  {
    if (!parser.parse_GPR(instr.OPS.CFLr.Rb)) return false;
    if (!parser.match_token(",")) return false;
    if (!parser.parse_GPR(instr.OPS.CFLr.Ro)) return false;
    return true;
  }
                                              
  udword_t cflr_format_t::encode(std::string mnemonic, 
                               const instruction_data_t &instr) const
  {
    uword_t iw = Opcode;

    insertG(iw,  7, instr.OPS.CFLr.Ro);
    insertG(iw, 12, instr.OPS.CFLr.Rb);
    insertV(iw, 22, 5, BOOST_BINARY(11110));
    insertPN(iw, 27, instr.Pred);

    return iw;
  }

}

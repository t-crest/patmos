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
// Core simulation loop of the Patmos Simulator.
//

#include "decoder.h"
#include "binary-formats.h"
#include "endian-conversion.h"
#include "instructions.h"

namespace patmos
{
  decoder_t::instructions_t decoder_t::Instructions;

  decoder_t::decoder_t()
  {
    // initialize the known instructions and binary formats.
    initialize_instructions();
  }

  unsigned int decoder_t::decode(word_t iw, word_t imm, unsigned int slot,
                                 instruction_data_t &result)
  {
    bool matched = false;
    bool is_long = false;

    // check all known instructions
    for(instructions_t::const_iterator i(Instructions.begin()),
        ie(Instructions.end()); i != ie; i++)
    {
      const binary_format_t &fmt = *i->get<1>();
      if (fmt.matches(iw, slot))
      {
        // ensure that only one instruction type matches.
        assert(!matched);

        // decode the instruction
        result = fmt.decode_operands(iw, imm);

        // check slot assignment
        assert(!fmt.is_long() || slot == 0);

        matched = true;
        is_long = fmt.is_long();
      }
    }

    return matched ?
            (is_long ? 2 : 1) : 0;
  }

  unsigned int decoder_t::decode(word_t *iwp, instruction_data_t *result)
  {
    word_t iw = from_big_endian<big_word_t>(iwp[0]);
    word_t imm = from_big_endian<big_word_t>(iwp[1]);

    // decode first instruction of bundle
    unsigned int size = decode(iw, imm, 0, result[0]);
    if (size == 0)
    {
      // unknown instruction -- report error
      return 0;
    }
    else if (size == 2)
    {
      assert(iw < 0);

      // long instruction (ALUl) -- inject NOP in second pipeline
      result[1] = instruction_data_t();

      return 2;
    }
    else if (iw >= 0)
    {
      assert(size == 1);

      // short bundle -- inject NOP in second pipeline
      result[1] = instruction_data_t();

      return 1;
    }
    // decode second instruction of bundle
    else if (decode(imm, 0, 1, result[1]) == 1)
    {
      // two instructinos of the bundle decoded
      return 2;
    }
    else
    {
      // unknown instruction or invalid encoding? -- report error
      return 0;
    }

    // we should never get here
    assert(false);
    abort();
  }

  void decoder_t::initialize_instructions()
  {
    // initialization already done?
    if (!Instructions.empty())
      return;

#define MK_INSTR(name, format, opcode)                                         \
  {                                                                            \
    instruction_t *itmp = new i_ ## name ## _t();                              \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode);            \
    Instructions.push_back(boost::make_tuple(itmp, ftmp));                     \
  }

#define MK_NINSTR(classname, name, format, opcode)                             \
  {                                                                            \
    instruction_t *itmp = new i_ ## classname ## _t();                         \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode);            \
    Instructions.push_back(boost::make_tuple(itmp, ftmp));                     \
  }

#define MK_SINSTR(name, format, opcode)                                        \
  {                                                                            \
    instruction_t *itmp = new i_ ## name ## _t();                              \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode, true);      \
    Instructions.push_back(boost::make_tuple(itmp, ftmp));                     \
  }

    // ALUi:
    MK_NINSTR(addil , addi , alui, 0)
    MK_NINSTR(subil , subi , alui, 1)
    MK_NINSTR(rsubil, rsubi, alui, 2)
    MK_NINSTR(slil  , sli  , alui, 3)
    MK_NINSTR(sril  , sri  , alui, 4)
    MK_NINSTR(srail , srai , alui, 5)
    MK_NINSTR(oril  , ori  , alui, 6)
    MK_NINSTR(andil , andi , alui, 7)

    // ALUl:
    MK_NINSTR(addil  , addl  , alul,  0)
    MK_NINSTR(subil  , subl  , alul,  1)
    MK_NINSTR(rsubil , rsubl , alul,  2)
    MK_NINSTR(slil   , sll   , alul,  3)
    MK_NINSTR(sril   , srl   , alul,  4)
    MK_NINSTR(srail  , sral  , alul,  5)
    MK_NINSTR(oril   , orl   , alul,  6)
    MK_NINSTR(andil  , andl  , alul,  7)
    MK_INSTR (rll            , alul,  8)
    MK_INSTR (rrl            , alul,  9)
    MK_INSTR (xorl           , alul, 10)
    MK_INSTR (norl           , alul, 11)
    MK_INSTR (shaddl         , alul, 12)
    MK_INSTR (shadd2l        , alul, 13)

    // ALUr:
    MK_INSTR(add   , alur,  0)
    MK_INSTR(sub   , alur,  1)
    MK_INSTR(rsub  , alur,  2)
    MK_INSTR(sl    , alur,  3)
    MK_INSTR(sr    , alur,  4)
    MK_INSTR(sra   , alur,  5)
    MK_INSTR(or    , alur,  6)
    MK_INSTR(and   , alur,  7)
    MK_INSTR(rl    , alur,  8)
    MK_INSTR(rr    , alur,  9)
    MK_INSTR(xor   , alur, 10)
    MK_INSTR(nor   , alur, 11)
    MK_INSTR(shadd , alur, 12)
    MK_INSTR(shadd2, alur, 13)

    // ALUu
    MK_INSTR(sext8 , aluu, 0)
    MK_INSTR(sext16, aluu, 1)
    MK_INSTR(zext16, aluu, 2)
    MK_INSTR(abs   , aluu, 3)

    // ALUm
    MK_INSTR(mul , alum, 0)
    MK_INSTR(mulu, alum, 1)

    // ALUc
    MK_INSTR(cmpeq  , aluc, 0)
    MK_INSTR(cmpneq , aluc, 1)
    MK_INSTR(cmplt  , aluc, 2)
    MK_INSTR(cmple  , aluc, 3)
    MK_INSTR(cmpult , aluc, 4)
    MK_INSTR(cmpule , aluc, 5)
    MK_INSTR(btest  , aluc, 6)

    // ALUp
    MK_INSTR(por , alup,  6)
    MK_INSTR(pand, alup,  7)
    MK_INSTR(pxor, alup, 10)
    MK_INSTR(pnor, alup, 11)

    // SPC
    MK_NINSTR(spcn, nop , spcn, 0)
    MK_NINSTR(spcw, wait, spcw, 0)
    MK_NINSTR(spct, mts , spct, 0)
    MK_NINSTR(spcf, mfs , spcf, 0)

    // LDT
    MK_SINSTR(lws , ldt,  0)
    MK_INSTR(lwl  , ldt,  1)
    MK_INSTR(lwc  , ldt,  2)
    MK_INSTR(lwm  , ldt,  3)
    MK_SINSTR(lhs , ldt,  4)
    MK_INSTR(lhl  , ldt,  5)
    MK_INSTR(lhc  , ldt,  6)
    MK_INSTR(lhm  , ldt,  7)
    MK_SINSTR(lbs , ldt,  8)
    MK_INSTR(lbl  , ldt,  9)
    MK_INSTR(lbc  , ldt, 10)
    MK_INSTR(lbm  , ldt, 11)
    MK_SINSTR(lhus, ldt, 12)
    MK_INSTR(lhul , ldt, 13)
    MK_INSTR(lhuc , ldt, 14)
    MK_INSTR(lhum , ldt, 15)
    MK_SINSTR(lbus, ldt, 16)
    MK_INSTR(lbul , ldt, 17)
    MK_INSTR(lbuc , ldt, 18)
    MK_INSTR(lbum , ldt, 19)

    MK_INSTR(dlwc , ldt, 20)
    MK_INSTR(dlwm , ldt, 21)
    MK_INSTR(dlhc , ldt, 22)
    MK_INSTR(dlhm , ldt, 23)
    MK_INSTR(dlbc , ldt, 24)
    MK_INSTR(dlbm , ldt, 25)
    MK_INSTR(dlhuc, ldt, 26)
    MK_INSTR(dlhum, ldt, 27)
    MK_INSTR(dlbuc, ldt, 28)
    MK_INSTR(dlbum, ldt, 29)

    // STT
    MK_SINSTR(sws, stt,  0)
    MK_INSTR(swl , stt,  1)
    MK_INSTR(swc , stt,  2)
    MK_INSTR(swm , stt,  3)
    MK_SINSTR(shs, stt,  4)
    MK_INSTR(shl , stt,  5)
    MK_INSTR(shc , stt,  6)
    MK_INSTR(shm , stt,  7)
    MK_SINSTR(sbs, stt,  8)
    MK_INSTR(sbl , stt,  9)
    MK_INSTR(sbc , stt, 10)
    MK_INSTR(sbm , stt, 11)

    // STC
    MK_INSTR(sres , stc, 0)
    MK_INSTR(sens , stc, 1)
    MK_INSTR(sfree, stc, 2)

    // PFLb
    MK_INSTR(bs, pflb, 0)
    MK_INSTR(bc, pflb, 1)
    MK_INSTR(b , pflb, 2)

    // PFLi
    MK_INSTR(bsr, pfli, 0)
    MK_INSTR(bcr, pfli, 1)
    MK_INSTR(br , pfli, 2)

    // PFLr
    MK_INSTR(ret, pflr, 0)

    // BNE
    MK_INSTR(bne, bne, 0)
  }

  const instruction_t &decoder_t::get_instruction(unsigned int ID)
  {
    const instruction_t *result = Instructions[ID].get<0>();
    assert(result && result->ID == ID);
    return *result;
  }
}


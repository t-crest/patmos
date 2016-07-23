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
// Core simulation loop of the Patmos Simulator.
//

#include "decoder.h"
#include "binary-formats.h"
#include "endian-conversion.h"
#include "instructions.h"
#include "loader.h"
#include "symbol.h"
#include "simulation-core.h"

#include <algorithm> 

namespace patmos
{
  decoder_t::instructions_t decoder_t::Instructions;
  
  int decoder_t::NOP_ID;

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
  
  int decoder_t::decode(loader_t &loader, section_info_t &section,
                        symbol_map_t &sym, decoder_callback_t &cb)
  {
    patmos::word_t bundle[NUM_SLOTS];
    patmos::instruction_data_t id[NUM_SLOTS];
    
    unsigned fetch = NUM_SLOTS;

    uword_t offset = section.offset;
    uword_t end = offset + section.size;
    uword_t addr = section.addr;
    
    int ret = 0;
    
    while (offset < end + 4) {
        
      // read next bundle
      while (fetch)
      {
        for (unsigned i = 0; i < NUM_SLOTS - 1; i++) {
          bundle[i] = bundle[i+1];
        }
        
        bundle[NUM_SLOTS-1] = offset < end ? loader.read_word(offset) : 0;
        offset += 4;
        fetch--;
      }
      
      // decode bundle        
      int slots = decode(bundle, id);
      
      if (slots == 0) {
        std::cerr << boost::format("Unknown instruction in bundle: "
                          "0x%1$08x: 0x%2$08x\n")
                          % addr % bundle[0];
      }
      
      int rs = cb.process_bundle(addr, id, slots, sym);
      if (rs != 0) ret = rs;
      
      fetch = std::max(1, slots);
      
      addr += fetch * 4;      
    }
    
    return ret;
  }

  bool decoder_t::is_NOP(instruction_data_t *data) const
  {
    return data && data->I->ID == NOP_ID && data->OPS.ALUil.Rd == patmos::r0;
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
#define MK_NINSTR_ALIAS(classname, name, format, opcode)

#define MK_FINSTR(classname, name, format, opcode, flag)                       \
  {                                                                            \
    instruction_t *itmp = new i_ ## classname ## _t();                         \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode, flag);      \
    Instructions.push_back(boost::make_tuple(itmp, ftmp));                     \
  }

#include "instructions.inc"
  }

  instruction_t &decoder_t::get_instruction(int ID)
  {
    instruction_t *result = Instructions[ID].get<0>();
    assert(result && result->ID == ID);
    return *result;
  }
}


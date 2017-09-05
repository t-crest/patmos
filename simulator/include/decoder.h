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
// Decode instructions given by a pointer.
//

#ifndef PATMOS_DECODER_H
#define PATMOS_DECODER_H

#include "basic-types.h"

#include <boost/tuple/tuple.hpp>

#include <vector>

namespace patmos
{
  // forward declaration
  class instruction_t;
  class binary_format_t;
  class instruction_data_t;
  class loader_t;
  class symbol_map_t;
  class section_info_t;

  class decoder_callback_t
  {
  public:

    /// Called when a bundle has been decoded.
    /// @param addr the current address of the bundle
    /// @param bundle the decoded bundle
    /// @param size the size of the bundle in operations, 0 if decoding failed.
    /// @param sym the symbol table
    /// @return 0 on success, anything else on error.
    virtual int process_bundle(uword_t addr, instruction_data_t *bundle,
                               unsigned slots, symbol_map_t &sym) = 0;
  };

  /// Interface to decoder Patmos instructions.
  class decoder_t
  {
  private:
    /// A vector of Patmos instructions and binary formats available to the
    /// simulator.
    typedef std::vector<boost::tuples::tuple<instruction_t*,
                                             const binary_format_t*> >
                                                                 instructions_t;

    /// A vector of Patmos instructions and binary formats available to the
    /// simulator.
    static instructions_t Instructions;

    /// ID of the instruction used to encode NOPs
    static int NOP_ID;

    /// Initialize the simulation functions and binary formats.
    static void initialize_instructions();

    /// Decode a binary encoded instruction.
    /// @param iw The instruction word to decode.
    /// @param imm The optional long immediate operand.
    /// @param slot Index of the slot of the instruction within the bundle.
    /// @param result A reference to store the data of the decoded instructions.
    /// @return The number of words occupied by the decoded instructions, i.e.,
    /// 1 or 2 if the instructions were decoded successfully, 0 in case of an
    /// error.
    unsigned int decode(word_t iw, word_t imm, unsigned int slot,
                        instruction_data_t &result);
  public:
    /// Construct a new instance of a Patmos decoder.
    decoder_t();

    /// Decode a binary encoded instruction bundle.
    /// @param iwp Pointer to binary data of the instruction bundle, the pointer
    /// has to point to an array of at least two elements.
    /// @param result A pointer to an array to store the data of the decoded
    /// instructions.
    /// @return The number of words occupied by the decoded instructions, i.e.,
    /// 1 or 2 if the instructions were decoded successfully, 0 in case of an
    /// error.
    unsigned int decode(word_t *iwp, instruction_data_t *result);

    /// Decode a stream of instructions provided by a binary loader.
    /// @return 0 on success, or any error code returned by the callback handler
    int decode(loader_t &loader, section_info_t &section,
               symbol_map_t &sym, decoder_callback_t &cb);

    /// Return the number of instructions known to the decoder.
    /// @return The number of instructions known to the decoder
    static unsigned int get_num_instructions()
    {
      return Instructions.size();
    }

    /// Test if an instruction is a NOP instruction.
    bool is_NOP(instruction_data_t *data) const;

    /// @return the ID of the instruction used for NOPs, i.e., SUBi.
    static unsigned int get_noop_id() { return NOP_ID; }

    /// Return instruction by ID.
    /// @return The instruction having the given ID.
    static instruction_t &get_instruction(int ID);
  };
}

#endif // PATMOS_DECODER_H


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

  /// Interface to decoder Patmos instructions.
  class decoder_t
  {
  private:
    /// A vector of Patmos instructions and binary formats available to the
    /// simulator.
    typedef std::vector<boost::tuples::tuple<instruction_t*, binary_format_t*> >
                                                                 instructions_t;

    /// A vector of Patmos instructions and binary formats available to the
    /// simulator.
    static instructions_t Instructions;

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
  };
}

#endif // PATMOS_DECODER_H


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
// This is a tiny assembler based on the boost::spirit parser, which can parse
// single lines of Patmos assembly code and return the corresponding instruction
// words.
//

#ifndef PATMOS_ASSEMBLER_H
#define PATMOS_ASSEMBLER_H

#include "basic-types.h"

#include <string>

namespace patmos
{
  // forward declarations
  class assembly_line_grammar_t;

  /// Interface to parse a individual lines of Patmos assembly code and
  /// returning the corresponding instruction words.
  class line_assembler_t
  {
  private:
    /// The spirit parser
    assembly_line_grammar_t &Line_parser;
  public:
    /// Construct a line assembler.
    line_assembler_t();

    /// Parse a line of assembly code and return the corresponding instruction
    /// word.
    /// @param line The input assembly code line.
    /// @param iw The output instruction word.
    /// @return True, in case the assembly line was parsed successfully; false
    /// otherwise.
    bool parse_line(const std::string &line, dword_t &iw) const;

    /// Free memory.
    ~line_assembler_t();
  };
}

#endif // PATMOS_ASSEMBLER_H



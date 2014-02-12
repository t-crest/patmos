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
#include "binary-format.h"
#include "registers.h"
#include "symbol.h"

#include <string>
#include <cctype>
#include <vector>
#include <map>
#include <ostream>

namespace patmos
{
  class lexer_t {
  private:
    std::string Line;
    
    std::vector<size_t> Positions;
        
    std::string Token;
    
    unsigned skip_space(unsigned pos);
    
    bool is_name_start(char c) const {
      return isalpha(c) || c == '.' || c == '_';
    }
    
  public:
    lexer_t(std::string& line);
    
    void reset();
    
    /// @return the number of tokens read so far
    unsigned tokens() { return Positions.size() - 1; }
    
    /// @return true when the current token is the last before EOL
    bool last() { return Positions.back() == Line.size(); }
    
    /// @return true when the current token is EOL
    bool end() { return Positions.back() == -1; }
    
    std::string tok() const { return Token; }
    
    size_t pos() const;
    
    bool get_value(word_t &value, bool negate) const;
    
    bool is_name() const;
    
    bool is_digit() const;
    
    /// Push the given number of tokens back, reset to the n'th token before
    /// the current one.
    bool push_back(unsigned count = 1);
    
    /// @return false when the next token is EOL.
    bool next();
  };
  
  class line_assembler_t;
  
  class line_parser_t {
  private:
    lexer_t Lexer;
    
    // For error messages
    line_assembler_t &Assembler;

    /// Parse the register number of a register name, ignoring the first character.
    /// @param name the register name
    /// @param maxregs the maximum number of registers (at most 100)
    /// @param regno the result
    /// @result true on success.
    bool parse_register_number(const std::string &name, unsigned maxregs, 
                               unsigned &regno) const;
    
  public:
    line_parser_t(line_assembler_t &assembler, std::string line);
    
    lexer_t &get_lexer() { return Lexer; }
    
    void set_error(const std::string &msg);
    
        
    bool parse_expression(word_t &value, reloc_info_t &reloc, 
                          bool require_paren);
    
    bool parse_GPR(GPR_e &reg);
    
    bool parse_SPR(SPR_e &reg);
    
    bool parse_PRR(PRR_e &pred, bool may_negate);
    
    bool match_token(const std::string &tok);
    
    bool match_stmt_end();
  };
  
  /// Interface to parse a individual lines of Patmos assembly code and
  /// returning the corresponding instruction words.
  class line_assembler_t
  {
  private:
    typedef std::multimap<std::string, binary_format_t *> instructions_t;
    instructions_t Instructions;
    
    unsigned NOP_ID;
    
    typedef std::vector<std::pair<unsigned,reloc_info_t> > relocations_t;
    relocations_t Relocations;
    
    symbol_map_t SymTab;
    
    std::vector<word_t> Code;
    
    int Error_pos;
    
    std::string Error_msg;
    
    void initialize();
    
    void add_relocation(unsigned index, reloc_info_t &reloc);
    
    bool parse_instruction(line_parser_t &parser, udword_t &encoded, 
                           reloc_info_t &reloc, bool &is_long);
    
    void set_error(line_parser_t &parser);
    
  public:
    /// Construct a line assembler.
    line_assembler_t();
    
    
    void set_error(size_t pos, const std::string &msg);
    
    bool has_error() const { return Error_pos >= 0; }
    
    int get_error_pos() const { return Error_pos; }
    
    std::string get_error_msg() const { return Error_msg; }
    
    void print_error(std::ostream &out, unsigned offset);
    

    /// Parse a line of assembly code and return the corresponding instruction
    /// word.
    /// @param line The input assembly code line.
    /// @param iw The output instruction word.
    /// @return True, in case the assembly line was parsed successfully; false
    /// otherwise.
    bool parse_line(const std::string &line, dword_t &iw);

    /// Write the parsed program to the given output stream.
    /// \param out The output stream.
    /// \param size Return the number of words emitted.
    /// \return False in case of an error.
    bool write_program(std::ostream &out, unsigned int &size);

    /// Free memory.
    ~line_assembler_t();
  };
}

#endif // PATMOS_ASSEMBLER_H

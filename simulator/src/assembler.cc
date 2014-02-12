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
// This is a tiny assembler embedded into C++ using operator overloading that
// allows to write small test programs.
//

// #define BOOST_SPIRIT_DEBUG 1

#include "assembler.h"
#include "binary-formats.h"
#include "instruction.h"
#include "instructions.h"
#include "util.h"

#include <cstring>
#include <string>
#include <iterator>
#include <iostream>
#include <map>
#include <boost/concept_check.hpp>

namespace patmos
{
  lexer_t::lexer_t(std::string& line) : Line(line)
  {
    reset();
  }
  
  unsigned lexer_t::skip_space(unsigned position) {
    unsigned pos = position;
    while (pos < Line.size() && isspace(Line[pos])) {
      pos++;
    }
    return pos;
  }
  
  void lexer_t::reset() {
    Positions.clear();
    Positions.push_back(skip_space(0));
    Token = "";
    
  }
      
  size_t lexer_t::pos() const {
    if (Positions.size() < 2) {
      return 0;
    } else {
      return Positions[Positions.size() - 2];
    }
  }
  
  bool lexer_t::get_value(word_t &value, bool negate) const {
    if (!is_digit()) return false;

    char *nptr;
    value = strtol(Token.c_str(), &nptr, 0);
      
    if (*nptr != '\0') {
      return false;
    }
      
    if (negate) {
      value = -value;
    }

    return true;
  }
  
  bool lexer_t::is_name() const {
    return !Token.empty() && is_name_start(Token[0]);
  }
    
  bool lexer_t::is_digit() const {
    return !Token.empty() && isdigit(Token[0]);
  }
    
  bool lexer_t::push_back(unsigned count) {    
    for (unsigned i = 0; i < count; i++) { 
      Positions.pop_back();
    }
    Positions.pop_back();
    if (Positions.empty()) {
      reset();
    } else {
      next();
    }
    return true;
  }
    
  bool lexer_t::next() 
  {
    if (end()) {
      return false;
    } else if (last()) {
      // Eat an EOL token..
      Token = "";
      Positions.push_back(-1);
      return false;
    }
    
    unsigned pos = Positions.back();
    
    size_t first = pos;
    size_t last = pos;
    
    // Eat a full word
    while (last < Line.size()) {
      char c = Line[last];
      
      // TODO we could check here if we are parsing a name or a digit and 
      // throw an error on malformed numbers
      if (isalnum(c) || c == '.' || c == '_') {
        last++;
      } else {
        break;
      }
    }
    
    // include at least one (special) character in the next token
    if (last == first) last++;
    // special handling of || separator as one token
    if (Line[first] == '|' && last < Line.size() && Line[last] == '|') last++;
    
    // go to the beginning of the next token
    Positions.push_back(skip_space(last));
    
    Token = Line.substr(first, last - first);
    
    return true;
  }
 

  line_parser_t::line_parser_t(line_assembler_t &assembler, std::string line)
  : Lexer(line), Assembler(assembler)
  {}
 
  void line_parser_t::set_error(const std::string &msg) {
    Assembler.set_error(Lexer.pos(), msg);
  }
  
  bool line_parser_t::parse_expression(word_t &value, reloc_info_t &reloc, 
                                       bool require_paren)
  {
    bool has_paren = false;
    if (Lexer.tok() == "(") {
      has_paren = true;
      if (!Lexer.next()) {
        set_error("missing expression.");
        return false;
      }
    }
    
    if (Lexer.is_name()) {      
      reloc.SymA = Lexer.tok();
      value = 0;
    } else {
      bool negate = match_token("-");
      
      if (!Lexer.get_value(value, false)) {
        set_error("Error parsing number.");
        return false;
      }
    }

    if (!Lexer.next()) {
      if (has_paren) {
        set_error("missing ')'.");
      }
      return !has_paren;
    }
    
    if (!reloc.SymA.empty()) {
      // TODO allow for 'SymA - SymB +|- Addend', 'SymA +|- Addend'
      
      if (Lexer.tok() == "-") {
        if (require_paren && !has_paren) {
          set_error("parenthesis required for expressions.");
          return false;
        }
        
        if (!Lexer.next()) {
          set_error("missing symbol or value.");
          return false;
        }
        if (Lexer.is_name()) {
          reloc.SymB = Lexer.tok();
        } else if (!Lexer.get_value(reloc.Addend, true)) {
          set_error("Error parsing number.");
          return false;
        }
        if (!Lexer.next()) {
          // missing closing parenthesis
          if (has_paren) {
            set_error("missing ')'.");
          }
          return !has_paren;
        }
      }
    }
    
    if (has_paren) {
      if (Lexer.tok() == ")") {
        Lexer.next();
      } else {
        if (has_paren) {
          set_error("missing ')'.");
        }
        return false;
      }
    }
    
    return true;
  }
   
  bool line_parser_t::parse_GPR(GPR_e &reg)
  {
    std::string name = Lexer.tok();
    if (name == "r0") {
      reg = r0;
    } else if (name == "r1") {
      reg = r1;
    } else if (name == "r2") {
      reg = r2;
    } else if (name == "r3") {
      reg = r3;
    } else if (name == "r4") {
      reg = r4;
    } else if (name == "r5") {
      reg = r5;
    } else if (name == "r6") {
      reg = r6;
    } else if (name == "r7") {
      reg = r7;
    } else if (name == "r8") {
      reg = r8;
    } else if (name == "r9") {
      reg = r9;
    } else if (name == "r10") {
      reg = r10;
    } else if (name == "r11") {
      reg = r11;
    } else if (name == "r12") {
      reg = r12;
    } else if (name == "r13") {
      reg = r13;
    } else if (name == "r14") {
      reg = r14;
    } else if (name == "r15") {
      reg = r15;
    } else if (name == "r16") {
      reg = r16;
    } else if (name == "r17") {
      reg = r17;
    } else if (name == "r18") {
      reg = r18;
    } else if (name == "r19") {
      reg = r19;
    } else if (name == "r20") {
      reg = r20;
    } else if (name == "r21") {
      reg = r21;
    } else if (name == "r22") {
      reg = r22;
    } else if (name == "r23") {
      reg = r23;
    } else if (name == "r24") {
      reg = r24;
    } else if (name == "r25") {
      reg = r25;
    } else if (name == "r26") {
      reg = r26;
    } else if (name == "r27") {
      reg = r27;
    } else if (name == "r28") {
      reg = r28;
    } else if (name == "r29") {
      reg = r29;
    } else if (name == "r30") {
      reg = r30;
    } else if (name == "r31") {
      reg = r31;
    } else {
      set_error("invalid GPR register name.");
      return false;
    }
    Lexer.next();
    return true;
  }
  
  bool line_parser_t::parse_SPR(SPR_e &reg)
  {
    std::string name = Lexer.tok();
    if (name == "s0") {
      reg = s0;
    } else if (name == "s1" || name == "sm") {
      reg = s1;
    } else if (name == "s2" || name == "sl") {
      reg = s2;
    } else if (name == "s3" || name == "sh") {
      reg = s3;
    } else if (name == "s4") {
      reg = s4;
    } else if (name == "s5" || name == "ss") {
      reg = s5;
    } else if (name == "s6" || name == "st") {
      reg = s6;
    } else if (name == "s7") {
      reg = s7;
    } else if (name == "s8") {
      reg = s8;
    } else if (name == "s9") {
      reg = s9;
    } else if (name == "s10") {
      reg = s10;
    } else if (name == "s11") {
      reg = s11;
    } else if (name == "s12") {
      reg = s12;
    } else if (name == "s13") {
      reg = s13;
    } else if (name == "s14") {
      reg = s14;
    } else if (name == "s15") {
      reg = s15;
    } else {
      set_error("invalid SPR register name.");
      return false;
    }
    Lexer.next();
    return true;
  }
  
  bool line_parser_t::parse_PRR(PRR_e &pred, bool may_negate)
  {
    bool negated = false;
    if (may_negate && Lexer.tok() == "!") {
      negated = true;
      if (!Lexer.next()) {
        set_error("predicate name expected.");
        return false;
      }
    }
    std::string reg = Lexer.tok();
    if (reg == "p0") {
      pred = negated ? pn0 : p0;
    } else if (reg == "p1") {
      pred = negated ? pn1 : p1;
    } else if (reg == "p2") {
      pred = negated ? pn2 : p2;
    } else if (reg == "p3") {
      pred = negated ? pn3 : p3;
    } else if (reg == "p4") {
      pred = negated ? pn4 : p4;
    } else if (reg == "p5") {
      pred = negated ? pn5 : p5;
    } else if (reg == "p6") {
      pred = negated ? pn6 : p6;
    } else if (reg == "p7") {
      pred = negated ? pn7 : p7;
    } else {
      if (Lexer.tok() == "!") {
        set_error("predicate can not be negated here.");
      } else {
        set_error("Invalid predicate name.");
      }
      return false;
    }
    Lexer.next();
    return true;
  }
  
  bool line_parser_t::match_token(const std::string &tok)
  {
    if (Lexer.tok() == tok) {
      Lexer.next();
      return true;
    }
    set_error(std::string("expected token '") + tok + "'");
    return false;
  }
  
  bool line_parser_t::match_stmt_end()
  {
    // skip semicolon
    if (Lexer.tok() == ";") {
      Lexer.next();
      return true;
    } else {
      // stmt must have parsed past the last token if there is no semicolon
      if (Lexer.end()) return true;
    }
    set_error("end of statement expected.");
    return false;
  }

   
   
  line_assembler_t::line_assembler_t() : NOP_ID(0), Error_pos(-1)
  {
      initialize();
  }
  
  line_assembler_t::~line_assembler_t() 
  {
  }

  void line_assembler_t::set_error(size_t pos, const std::string &msg) {
    Error_pos = pos;
    Error_msg = msg;
  }
  
  void line_assembler_t::print_error(std::ostream &out, unsigned offset) 
  {
    if (Error_pos < 0) return;
    
    for (unsigned i = 0; i < offset + Error_pos; i++) {
      out << " ";
    }
    out << "^ " << Error_msg << "\n";
  }
  
  void line_assembler_t::initialize() 
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
    Instructions.insert(std::pair<std::string,binary_format_t*>(#name , ftmp));\
  }

#define MK_NINSTR(classname, name, format, opcode)                             \
  {                                                                            \
    instruction_t *itmp = new i_ ## classname ## _t();                         \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode);            \
    Instructions.insert(std::pair<std::string,binary_format_t*>(#name , ftmp));\
  }

#define MK_SINSTR(name, format, opcode)                                        \
  {                                                                            \
    instruction_t *itmp = new i_ ## name ## _t();                              \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode, true);      \
    Instructions.insert(std::pair<std::string,binary_format_t*>(#name , ftmp));\
  }
#define MK_NINSTR_ALIAS(classname, name, format, opcode)                       \
    MK_NINSTR(classname, name, format, opcode)
  
#include "instructions.inc"
  }
  
  void line_assembler_t::add_relocation(unsigned index, reloc_info_t &reloc)
  {
    if (reloc.SymA.empty()) {
      return;
    }
    Relocations.push_back(std::pair<unsigned,reloc_info_t>(index, reloc));
  }
  
  bool line_assembler_t::parse_instruction(line_parser_t &parser, 
                                           udword_t &encoded, 
                                           reloc_info_t &reloc, bool &is_long) 
  {
    lexer_t &lexer = parser.get_lexer();
    
    instruction_data_t instr;
    instr.Address = Code.size() * 4;
    instr.Pred = p0;
    
    // parse guard
    // [ '(' ['!'] <pred> ')' ]
    if (lexer.tok() == "(") {
      if (!lexer.next()) {
        parser.set_error("missing guard name.");
        return false;
      }
      if (!parser.parse_PRR(instr.Pred, true)) {        
        return false;
      }
      if (lexer.tok() != ")") {
        parser.set_error("missing ')'.");
        return false;
      }
      if (!lexer.next()) {
        parser.set_error("instruction expected.");
        return false;
      }
    }
    
    // parse instruction
    if (!lexer.is_name()) {
      parser.set_error("expect instruction opcode.");
      return false;
    }
    std::string opcode = lexer.tok();

    // Match pseudo instructions
    bool skip_operands = false;
    if (opcode == "nop") {
      skip_operands = true;
      opcode = "subi";
      instr.OPS.ALUil.Rd = r0;
      instr.OPS.ALUil.Rs1 = r0;
      instr.OPS.ALUil.Imm2 = 0;
    } 
    else if (opcode == "halt") {
      skip_operands = true;
      opcode = "ret";
      instr.OPS.CFLr.Ro = r0;
      instr.OPS.CFLr.Rb = r0;
    }
    
    // get the instruction format, parse operands.
    instructions_t::iterator it = Instructions.find(opcode);
    if (it == Instructions.end()) {
      parser.set_error("unknown opcode.");
      return false;
    }
    
    lexer.next();
    
    unsigned tokens = lexer.tokens();
    
    // iterate over all instruction formats for the same mnemonic
    while (it != Instructions.end() && it->first == opcode) {
      binary_format_t *bfmt = it->second;
          
      if (!skip_operands) {
        if (!bfmt->parse_operands(parser, opcode, instr, reloc)) {
          // reset the lexer
          lexer.push_back(lexer.tokens() - tokens);
          // try the next binary format
          it++;
          continue;
        }
      }
    
      is_long = bfmt->is_long();
      
      encoded = bfmt->encode(opcode, instr);
      
      return true;
    }
    return false;
  }
  
  bool line_assembler_t::parse_line(const std::string &line, dword_t &iw)
  {
    // [<label>:] [ .word <expr> | '('[!]<pred>')' <opcode> <operands> ]
    
    // Tokenize
    line_parser_t parser(*this, line);
    lexer_t &lexer = parser.get_lexer();
    if (lexer.end()) return true;

    lexer.next();
    
    // [ <label> ':' ]
    std::string label = lexer.tok();
    
    if (lexer.is_name() && !lexer.last()) {
      lexer.next();
      if (parser.match_token(":")) {
        
        // we got a label, emit it
        symbol_info_t symbol(Code.size() * 4, 0, false, label);
        
        SymTab.add(symbol);
        
        // continue with the next token or return if no more tokens
        if (lexer.end()) {
          return true;
        }
      } else {
        // go back to the first token, start over 
        lexer.push_back();
      }
    }
    
    // Check directives
    // .word <symbol> [ '-' <symbol> ]
    if (lexer.tok() == ".word") {
      if (!lexer.next()) {
        parser.set_error("missing value.");
        return false;
      }
      
      word_t value; 
      reloc_info_t reloc;
      reloc.set_format(32);
      if (!parser.parse_expression(value, reloc, false)) {
        return false;
      }
      
      // Emit a word with a relocation
      add_relocation(Code.size(), reloc);
      Code.push_back(value);
      
      if (!parser.match_stmt_end()) {
        return false;
      }
    }

    while (!lexer.end()) {
      
      reloc_info_t reloc;
      bool is_long = false;
      udword_t encoded;

      if (!parse_instruction(parser, encoded, reloc, is_long)) {
        return false;
      }
      
      // handle bundles
      if (lexer.tok() == "||") {
        
        // TODO check if the last instruction had the bundle bit set as well
        if (is_long) {
          parser.set_error("ALUl instruction cannot be bundled.");
          return false;
        }
        
        // set bundle bit
        encoded |= ((udword_t)1<<31);
        
        lexer.next();
      } else if (!parser.match_stmt_end()) {
        return false;
      }
   
      // add the instruction and the relocation
      if (is_long) {
        Code.push_back( (uword_t)(encoded >> 32) );
      }
      add_relocation(Code.size(), reloc);
      Code.push_back((uword_t)encoded);
    }
    
    return true;
  }

  bool line_assembler_t::write_program(std::ostream &out, unsigned int &size)
  {
    // Patch all relocations.
    for (relocations_t::iterator it=Relocations.begin(),
          ie=Relocations.end(); it != ie; it++) 
    {
      unsigned index = it->first;
      reloc_info_t &reloc = it->second;
      
      word_t value;
      if (reloc.get_value(SymTab, value, index*4)) {        
        Code[index] = Code[index] | value;
      } 
      else {
        if (!reloc.SymA.empty() && !SymTab.contains(reloc.SymA)) {
          std::cout << "Undefined Label: " << reloc.SymA << "\n";
        }
        if (!reloc.SymB.empty() && !SymTab.contains(reloc.SymB)) {
          std::cout << "Undefined Label: " << reloc.SymB << "\n";
        }
      }
    }

    // Write program.
    for (int i = 0; i < Code.size(); i++) {
        uword_t instr = Code[i];
        
        out << (char)(instr >> 24);
        out << (char)(instr >> 16);
        out << (char)(instr >>  8);
        out << (char)(instr);
    }
    
    size = Code.size();
    
    return true;
  }

}

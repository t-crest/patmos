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
 
  bool line_parser_t::parse_register_number(const std::string &name, 
                                            unsigned maxregs, 
                                            unsigned &regno) const
  {
    unsigned length = name.size();
    if (length < 2 || length > 3) return false;
    if (!isdigit(name[1]) || (length == 3 && !isdigit(name[2]))) return false;
    
    // using scanf or atoi or strtol for a 2 digit number has quite some 
    // overhead and the error handling code to reject stuff like 'r+1z' 
    // as valid name would be larger than a simple switch over all names.
    unsigned value = name[1] - '0';
    if (length == 3) {
      value = value * 10 + (name[2] - '0');
    }
    
    regno = value;
    
    return value < maxregs;
  }

 
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
    if (name.size() < 2 || name[0] != 'r') {
      set_error("invalid GPR register name.");
      return false;
    }
    if (name == "rtr") {
      reg = r29;
    } else if (name == "rfp") {
      reg = r30;
    } else if (name == "rsp") {
      reg = r31;
    } else {
      unsigned regno;
      if (!parse_register_number(name, 32, regno)) {
        set_error("invalid GPR register name.");
        return false;
      }
      reg = (GPR_e)regno;
    }
    Lexer.next();
    return true;
  }
  
  bool line_parser_t::parse_SPR(SPR_e &reg)
  {
    std::string name = Lexer.tok();
    if (name.size() < 2 || name[0] != 's') {
      set_error("invalid SPR register name.");
      return false;
    }
    if (name == "sl") {
      reg = s2;
    } else if (name == "sh") {
      reg = s3;
    } else if (name == "ss") {
      reg = s5;
    } else if (name == "st") {
      reg = s6;
    } else if (name == "srb") {
      reg = s7;
    } else if (name == "sro") {
      reg = s8;
    } else if (name == "sxb") {
      reg = s9;
    } else if (name == "sxo") {
      reg = s10;
    } else {
      unsigned regno;
      if (!parse_register_number(name, 16, regno)) {
        set_error("invalid SPR register name.");
        return false;
      }
      reg = (SPR_e)regno;
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

#define MK_FINSTR(classname, name, format, opcode, flag)                       \
  {                                                                            \
    instruction_t *itmp = new i_ ## classname ## _t();                         \
    itmp->ID = Instructions.size();                                            \
    itmp->Name = #name;                                                        \
    binary_format_t *ftmp = new format ## _format_t(*itmp, opcode, flag);      \
    Instructions.insert(std::pair<std::string,binary_format_t*>(#name , ftmp));\
  }

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
      opcode = "brcf";
      instr.OPS.CFLrt.D = 1;
      instr.OPS.CFLrt.Rs1 = r0;
      instr.OPS.CFLrt.Rs2 = r0;
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
    for (unsigned int i = 0; i < Code.size(); i++) {
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

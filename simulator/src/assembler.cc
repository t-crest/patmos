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
#include "util.h"

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_binary.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>

#include <iterator>
#include <iostream>
#include <map>

namespace patmos
{
  /// Instructions of a program (host encoding)
  typedef std::vector<word_t> program_t;

  /// Mapping of names to name IDs
  typedef std::map<std::string, unsigned int> names_t;

  /// Mapping of symbols (name IDs to addresses)
  typedef std::map<unsigned, word_t> symbols_t;

  /// Relocation kinds
  enum relocation_kind_e
  {
    /// relocation of absolute addresses for ALUl instructions
    ALUL_ABS,
    /// relocation of absolute addresses for ALUi instructions
    ALUI_ABS,
    /// relocation of absolute addresses for PFL instructions
    PFLB_ABS,
    /// relocation of relative addresses for PFL instructions
    PFLB_FREL
  };

  /// Relocation information
  struct relocation_t
  {
    /// Kind of the relocation.
    relocation_kind_e Kind;

    /// Symbol whose address has to be patched.
    unsigned int Symbol;

    /// Address of the last .fstart directive seen in the program.
    unsigned int Last_fstart;
  };

  /// Map program addresses to relocations.
  typedef std::map<word_t, relocation_t> relocations_t;

  /// Base class for parsing assembly code lines.
  class assembly_line_grammar_t : public boost::spirit::qi::grammar<
                                               std::string::const_iterator,
                                               void(),
                                               boost::spirit::ascii::space_type>
  {
    friend class line_assembler_t;
    private:
      typedef boost::spirit::qi::rule<std::string::const_iterator, word_t(),
                                      boost::spirit::ascii::space_type> rule_t;

      typedef boost::spirit::qi::rule<std::string::const_iterator, dword_t(),
                                      boost::spirit::ascii::space_type> drule_t;

      typedef boost::spirit::qi::rule<std::string::const_iterator, void(),
                                      boost::spirit::ascii::space_type> nrule_t;

      /// The currently parsed program.
      program_t P;

      /// Names appearing the program.
      names_t Names;

      /// Symbol definitions appearing the program (labels).
      symbols_t Symbols;

      /// Relocations appearing in the program.
      relocations_t Relocations;

      /// Keep track of last .fstart.
      unsigned int Last_fstart;

      /// Main rule.
      nrule_t Line;

      /// Parse a symbol name - return name ID.
      rule_t Name;

      /// Parse a label - return symbol ID.
      nrule_t Label;

      /// Parse a bundle.
      nrule_t Bundle;

      /// Parse any 32-bit instruction for either slot 1 and 2.
      rule_t Instruction1, Instruction2;

      /// Parse general purpose registers.
      rule_t GPR;

      /// Parse general purpose registers.
      rule_t SPR;

      /// Parse predicate registers.
      rule_t PRR;

      /// Parse predicate registers with negate.
      rule_t NegPRR;

      /// Parse immediate.
      rule_t Imm, Imm4u, Imm12u, Imm22u, Imm22s, Imm7s;

      /// Parse an immediate or label for ALUl and ALUi instructions.
      rule_t ImmALUL, ImmALUI;

      /// Parse an immediate or label for PFL instructions.
      rule_t ImmPFL_ABS, ImmPFL_FREL;

      /// Parse an arbitrary data word.
      rule_t Data_Word;

      /// Parse an .fstart directive
      rule_t Fstart;

      /// Parse an instruction predicate.
      rule_t Pred;

      /// Parse an addressing mode.
      rule_t Addresse;

      /// Parse ALU opcodes.
      rule_t ALUiopc, ALUlropc, ALUuopc, ALUmopc, ALUcopc, ALUpopc;

      /// Parse long ALU instructions.
      drule_t ALUl;

      /// Parse ALU instructions.
      rule_t ALUi, ALUr, ALUu, ALUm, ALUc, ALUp;

      /// Parse SPC opcodes.
      rule_t SPCwopc;

      /// Parse SPC instructions.
      rule_t SPCn, SPCw, SPCt, SPCf;

      /// Parse LDT opcodes.
      rule_t LDTopc, LDTsopc, dLDTopc;

      /// Parse LDT instructions.
      rule_t LDT, LDTs, dLDT;

      /// Parse STT opcodes.
      rule_t STTopc, STTsopc;

      /// Parse STT instructions.
      rule_t STT, STTs;

      /// Parse STC opcodes.
      rule_t STCopc;

      /// Parse STC instructions.
      rule_t STC;

      /// Parse PFL opcodes.
      rule_t PFLbopc, PFLiopc, PFLropc;

      /// Parse PFL instructions.
      rule_t PFLb, PFLi, PFLr;

      /// Parse BNE instructions.
      rule_t BNE;

      /// Emit an instruction word to the program.
      /// \param iw the instruction word.
      inline void emit(word_t iw)
      {
        P.push_back(iw);
      }

      /// Emit the second instruction of a bundle to the program, updating the
      /// stop-bit of the previous instruction word.
      /// \param iw2 the second instruction word.
      inline void emit2(word_t iw2)
      {
        P.back() |= (1ul << (sizeof(word_t)*8 - 1));
        P.push_back(iw2);
      }

      /// Emit a bundle.
      /// \param diw A bundle consisting of two instruction words.
      inline void demit(dword_t diw)
      {
        word_t iw1 = diw >> (sizeof(word_t) * 8);
        word_t iw2 = diw;
        P.push_back(iw1);
        P.push_back(iw2);
      }

      /// Append a name to the name list.
      /// \param c The first character of the name.
      /// \param t The other characters of the name.
      /// \return A unique ID for the name.
      inline unsigned int make_name(char h, const std::vector<char> &t)
      {
        std::string name(1, h);
        name.append(t.begin(), t.end());
        if (Names.find(name) == Names.end())
          Names[name] = Names.size() + 1;

        return Names[name];
      }

      /// Append a symbol to the symbol list, assign the symbol the address of 
      /// the current program location.
      /// \param symbol The name ID of the symbol to be created.
      /// \return False in case the label was redefined (causes parsing error).
      inline bool make_label(unsigned int symbol)
      {
        if (Symbols.find(symbol) == Symbols.end())
        {
          Symbols[symbol] = P.size() * sizeof(word_t);
          return true;
        }
        else
          return false;
      }

      /// Create a new relocation at the current program location.
      /// \param kind The relocation kind (instruction type, etc.)
      /// \param symbol The symbol whose address should be patched.
      /// \return Zero to supply a valid value for the parser semantic action.
      inline unsigned int make_relocation(relocation_kind_e kind, unsigned int symbol)
      {
        relocation_t tmp;
        tmp.Kind = kind;
        tmp.Symbol = symbol;
        tmp.Last_fstart = Last_fstart;
        Relocations[P.size()] = tmp;

        return 0;
      }

      /// Keep track of methods for the method cache.
      /// \param size
      inline word_t make_fstart(unsigned int size)
      {
        // align for method cache block size (32 for now)
        while((P.size() % 8) != 7)
          P.push_back(0);

        Last_fstart = (P.size() + 1) * 4;

        return size;
      }
    public:
      /// Construct a parser for instruction path patterns.
      explicit assembly_line_grammar_t() :
          assembly_line_grammar_t::base_type(Line), Last_fstart(0)
      {
        Line = Label >> ((Bundle >> ';') | boost::spirit::eol);

        // parse a name, e.g. for labels
        Name = boost::spirit::lexeme[(boost::spirit::ascii::char_("_a-zA-Z") >>
                *boost::spirit::ascii::char_("_0-9a-zA-Z"))
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                            &assembly_line_grammar_t::make_name,
                                            this, boost::spirit::qi::_1,
                                            boost::spirit::qi::_2)]];

        // parse an optional label:
        Label = (Name >> ":")
                [boost::spirit::qi::_pass = boost::phoenix::bind(
                                           &assembly_line_grammar_t::make_label,
                                           this, boost::spirit::qi::_1)] |
                boost::spirit::eps;

        // parse a bundle, i.e., 1 or 2 instructions -- special care is taken
        // for ALUl instructions and the special HLT instruction of the
        // simulator.
        Bundle = ALUl
                 [boost::phoenix::bind(&assembly_line_grammar_t::demit, this,
                                       boost::spirit::qi::_1)]                 |
                 boost::spirit::lit("halt")
                 [boost::phoenix::bind(&assembly_line_grammar_t::demit, this,
                                       0x0780000002400024)]                    |
                 (Instruction1
                  [boost::phoenix::bind(&assembly_line_grammar_t::emit, this,
                                        boost::spirit::qi::_1)] >>
                   -("||" >> Instruction2
                          [boost::phoenix::bind(&assembly_line_grammar_t::emit2,
                                                this, boost::spirit::qi::_1)]))|
                 Data_Word
                 [boost::phoenix::bind(&assembly_line_grammar_t::emit, this,
                                       boost::spirit::qi::_1)]                 |
                 Fstart
                 [boost::phoenix::bind(&assembly_line_grammar_t::emit, this,
                                       boost::spirit::qi::_1)]                 ;

        // All instructions except ALUl
        Instruction1 %= ( ALUi | ALUr | ALUu | ALUm | ALUc | ALUp |
                          SPCn | SPCw | SPCt | SPCf |
                          LDT  | dLDT |
                          STT  |
                          STC  |
                          BNE  |
                          PFLi | PFLb | PFLr);

        // All instructions except SPCn, SPNw, PFL, LDT, STT, and STC.
        Instruction2 %= (ALUi | ALUr | ALUu | ALUm | ALUc | ALUp |
                         SPCt | SPCf |
                         LDTs | STTs);

        // Parse general purpose registers.
        GPR = 'r' >> boost::spirit::uint_
              [boost::spirit::qi::_pass = boost::phoenix::bind(isGPR,
                                                         boost::spirit::qi::_1)]
              [boost::spirit::qi::_val = boost::spirit::qi::_1];

        // Parse general purpose registers.
        SPR = 's' >> boost::spirit::uint_
              [boost::spirit::qi::_pass = boost::phoenix::bind(isSPR,
                                                         boost::spirit::qi::_1)]
              [boost::spirit::qi::_val = boost::spirit::qi::_1];

        // Parse predicate registers.
        PRR = 'p' >> boost::spirit::uint_
              [boost::spirit::qi::_pass = boost::phoenix::bind(isPRR,
                                                         boost::spirit::qi::_1)]
              [boost::spirit::qi::_val = boost::spirit::qi::_1];

        // Parse predicate registers with negate.
        NegPRR = (boost::spirit::lit('!') >> PRR [
                    boost::spirit::qi::_val = boost::spirit::qi::_1 + (unsigned int)NUM_PRR]) |
                 PRR[boost::spirit::qi::_val = boost::spirit::qi::_1];


        // Parse immediate.
        Imm %= ("0x" >> boost::spirit::hex) |
               ("0b" >> boost::spirit::bin) |
               ('0' >> boost::spirit::oct)  |
               (boost::spirit::int_)        ;

        Imm4u = Imm[boost::spirit::_pass = boost::phoenix::bind(fitu,
                                                              boost::spirit::_1,
                                                              4)]
                   [boost::spirit::_val = boost::spirit::_1];

        Imm12u = Imm[boost::spirit::_pass = boost::phoenix::bind(fitu,
                                                              boost::spirit::_1,
                                                              12)]
                   [boost::spirit::_val = boost::spirit::_1];

        Imm22u = Imm[boost::spirit::_pass = boost::phoenix::bind(fitu,
                                                              boost::spirit::_1,
                                                              22)]
                   [boost::spirit::_val = boost::spirit::_1];

        Imm22s = Imm[boost::spirit::_pass = boost::phoenix::bind(fits,
                                                              boost::spirit::_1,
                                                              22)]
                   [boost::spirit::_val = boost::spirit::_1];

        Imm7s = Imm[boost::spirit::_pass = boost::phoenix::bind(fits,
                                                              boost::spirit::_1,
                                                              7)]
                   [boost::spirit::_val = boost::spirit::_1];

        ImmALUL = Imm [boost::spirit::_val = boost::spirit::_1]                |
                  (Name - GPR) [boost::spirit::_val = boost::phoenix::bind(
                                      &assembly_line_grammar_t::make_relocation,
                                      this, ALUL_ABS, boost::spirit::_1)];

        ImmALUI = Imm12u [boost::spirit::_val = boost::spirit::_1]                |
                  Name [boost::spirit::_val = boost::phoenix::bind(
                                      &assembly_line_grammar_t::make_relocation,
                                      this, ALUI_ABS, boost::spirit::_1)];

        ImmPFL_ABS = Imm22s [boost::spirit::_val = boost::spirit::_1]         |
                     Name [boost::spirit::_val = boost::phoenix::bind(
                                      &assembly_line_grammar_t::make_relocation,
                                      this, PFLB_ABS, boost::spirit::_1)];

        ImmPFL_FREL = Imm22s [boost::spirit::_val = boost::spirit::_1]        |
                      Name [boost::spirit::_val = boost::phoenix::bind(
                                      &assembly_line_grammar_t::make_relocation,
                                      this, PFLB_FREL, boost::spirit::_1)];

        // parse some arbitrary word-sized data
        Data_Word = (".word" >> Imm)
                    [boost::spirit::_pass = boost::phoenix::bind(fits,
                                                              boost::spirit::_1,
                                                              32)]
                    [boost::spirit::_val = boost::spirit::_1];

        Fstart = (".fstart" >> Imm)
                    [boost::spirit::_pass = boost::phoenix::bind(fitu,
                                                              boost::spirit::_1,
                                                              32)]
                    [boost::spirit::_val = boost::phoenix::bind(
                                          &assembly_line_grammar_t::make_fstart,
                                          this, boost::spirit::_1)];

        // Parse an (optional) instruction predicate.
        Pred = ('(' >> NegPRR >> ')')
               [boost::spirit::qi::_val = boost::spirit::qi::_1] |
               boost::spirit::eps
               [boost::spirit::qi::_val = (unsigned int) p0];

        // Parse an addressing mode.
        Addresse = ('+' >> Imm7s)
                   [boost::spirit::qi::_val = boost::spirit::qi::_1] |
                   ('-' >> Imm7s)
                   [boost::spirit::qi::_val = -boost::spirit::qi::_1];

        // Parse ALUi instructions
        ALUiopc = boost::spirit::lit("addi")  [boost::spirit::qi::_val = 0] |
                  boost::spirit::lit("subi")  [boost::spirit::qi::_val = 1] |
                  boost::spirit::lit("rsubi") [boost::spirit::qi::_val = 2] |
                  boost::spirit::lit("sli")   [boost::spirit::qi::_val = 3] |
                  boost::spirit::lit("sri")   [boost::spirit::qi::_val = 4] |
                  boost::spirit::lit("srai")  [boost::spirit::qi::_val = 5] |
                  boost::spirit::lit("ori")   [boost::spirit::qi::_val = 6] |
                  boost::spirit::lit("andi")  [boost::spirit::qi::_val = 7] ;

        ALUi = (Pred >> ALUiopc >> GPR >> '=' >> GPR >> ',' >> ImmALUI)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 alui_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse ALUl and ALUr opcodes
        ALUlropc = boost::spirit::lit("add")    [boost::spirit::qi::_val =  0] |
                   boost::spirit::lit("sub")    [boost::spirit::qi::_val =  1] |
                   boost::spirit::lit("rsub")   [boost::spirit::qi::_val =  2] |
                   boost::spirit::lit("sl")     [boost::spirit::qi::_val =  3] |
                   boost::spirit::lit("sra")    [boost::spirit::qi::_val =  5] |
                   boost::spirit::lit("sr")     [boost::spirit::qi::_val =  4] |
                   boost::spirit::lit("or")     [boost::spirit::qi::_val =  6] |
                   boost::spirit::lit("and")    [boost::spirit::qi::_val =  7] |
                   boost::spirit::lit("rl")     [boost::spirit::qi::_val =  8] |
                   boost::spirit::lit("rr")     [boost::spirit::qi::_val =  9] |
                   boost::spirit::lit("xor")    [boost::spirit::qi::_val = 10] |
                   boost::spirit::lit("nor")    [boost::spirit::qi::_val = 11] |
                   boost::spirit::lit("shadd2") [boost::spirit::qi::_val = 13] |
                   boost::spirit::lit("shadd")  [boost::spirit::qi::_val = 12] ;

        // Parse ALUl instructions
        ALUl = (Pred >> ALUlropc >> GPR >> '=' >> GPR >> ',' >> ImmALUL)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 alul_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse ALUr instructions
        ALUr = (Pred >> ALUlropc >> GPR >> '=' >> GPR >> ',' >> GPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 alur_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse ALUu instructions
        ALUuopc = boost::spirit::lit("sext8")  [boost::spirit::qi::_val = 0] |
                  boost::spirit::lit("sext16") [boost::spirit::qi::_val = 1] |
                  boost::spirit::lit("zext16") [boost::spirit::qi::_val = 2] |
                  boost::spirit::lit("abs")    [boost::spirit::qi::_val = 3] ;

        ALUu = (Pred >> ALUuopc >> GPR >> '=' >> GPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                   aluu_format_t::encode, boost::spirit::qi::_1,
                                   boost::spirit::qi::_2, boost::spirit::qi::_3,
                                   boost::spirit::qi::_4)];

        // Parse ALUm instructions
        ALUmopc = boost::spirit::lit("mulu")  [boost::spirit::qi::_val = 1] |
                  boost::spirit::lit("mul")   [boost::spirit::qi::_val = 0] ;

        ALUm = (Pred >> ALUmopc >> GPR >> ',' >> GPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                   alum_format_t::encode, boost::spirit::qi::_1,
                                   boost::spirit::qi::_2, boost::spirit::qi::_3,
                                   boost::spirit::qi::_4)];

        // Parse ALUc instructions
        ALUcopc = boost::spirit::lit("cmpeq")  [boost::spirit::qi::_val = 0] |
                  boost::spirit::lit("cmpneq") [boost::spirit::qi::_val = 1] |
                  boost::spirit::lit("cmplt")  [boost::spirit::qi::_val = 2] |
                  boost::spirit::lit("cmple")  [boost::spirit::qi::_val = 3] |
                  boost::spirit::lit("cmpult") [boost::spirit::qi::_val = 4] |
                  boost::spirit::lit("cmpule") [boost::spirit::qi::_val = 5] |
                  boost::spirit::lit("btest")  [boost::spirit::qi::_val = 6] ;

        ALUc = (Pred >> ALUcopc >> PRR >> '=' >> GPR >> ',' >> GPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 aluc_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse ALUp instructions
        ALUpopc = boost::spirit::lit("por")   [boost::spirit::qi::_val =  6] |
                  boost::spirit::lit("pand")  [boost::spirit::qi::_val =  7] |
                  boost::spirit::lit("pxor")  [boost::spirit::qi::_val = 10] |
                  boost::spirit::lit("pnor")  [boost::spirit::qi::_val = 11] ;

        ALUp = (Pred >> ALUpopc >> PRR >> '=' >> NegPRR >> ',' >> NegPRR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 alup_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse SPCn instructions
        SPCn = (Pred >> "nop" >> Imm4u)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                   spcn_format_t::encode, boost::spirit::qi::_1,
                                   boost::spirit::qi::_2)];

        // Parse SPCw instructions
        SPCwopc = boost::spirit::lit("waitm")  [boost::spirit::qi::_val = 0];

        SPCw = (Pred >> SPCwopc)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                   spcw_format_t::encode, boost::spirit::qi::_1,
                                   boost::spirit::qi::_2)];

        // Parse SPCt instructions
        SPCt = (Pred >> "mts" >> SPR >> '=' >> GPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 spct_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3)];

        // Parse SPCf instructions
        SPCf = (Pred >> "mfs" >> GPR >> '=' >> SPR)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 spcf_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3)];

        // Parse LDT instructions
        LDTopc = boost::spirit::lit("lws")    [boost::spirit::qi::_val =  0] |
                 boost::spirit::lit("lwl")    [boost::spirit::qi::_val =  1] |
                 boost::spirit::lit("lwc")    [boost::spirit::qi::_val =  2] |
                 boost::spirit::lit("lwm")    [boost::spirit::qi::_val =  3] |
                 boost::spirit::lit("lhs")    [boost::spirit::qi::_val =  4] |
                 boost::spirit::lit("lhl")    [boost::spirit::qi::_val =  5] |
                 boost::spirit::lit("lhc")    [boost::spirit::qi::_val =  6] |
                 boost::spirit::lit("lhm")    [boost::spirit::qi::_val =  7] |
                 boost::spirit::lit("lbs")    [boost::spirit::qi::_val =  8] |
                 boost::spirit::lit("lbl")    [boost::spirit::qi::_val =  9] |
                 boost::spirit::lit("lbc")    [boost::spirit::qi::_val = 10] |
                 boost::spirit::lit("lbm")    [boost::spirit::qi::_val = 11] |
                 boost::spirit::lit("lhus")   [boost::spirit::qi::_val = 12] |
                 boost::spirit::lit("lhul")   [boost::spirit::qi::_val = 13] |
                 boost::spirit::lit("lhuc")   [boost::spirit::qi::_val = 14] |
                 boost::spirit::lit("lhum")   [boost::spirit::qi::_val = 15] |
                 boost::spirit::lit("lbus")   [boost::spirit::qi::_val = 16] |
                 boost::spirit::lit("lbul")   [boost::spirit::qi::_val = 17] |
                 boost::spirit::lit("lbuc")   [boost::spirit::qi::_val = 18] |
                 boost::spirit::lit("lbum")   [boost::spirit::qi::_val = 19] ;

        // Parse LDTs instructions
        LDTsopc = boost::spirit::lit("lws")    [boost::spirit::qi::_val =  0] |
                  boost::spirit::lit("lhs")    [boost::spirit::qi::_val =  4] |
                  boost::spirit::lit("lbs")    [boost::spirit::qi::_val =  8] |
                  boost::spirit::lit("lhus")   [boost::spirit::qi::_val = 12] |
                  boost::spirit::lit("lbus")   [boost::spirit::qi::_val = 16] ;

        dLDTopc = boost::spirit::lit("dlwc")  [boost::spirit::qi::_val = 20] |
                  boost::spirit::lit("dlwm")  [boost::spirit::qi::_val = 21] |
                  boost::spirit::lit("dlhc")  [boost::spirit::qi::_val = 22] |
                  boost::spirit::lit("dlhm")  [boost::spirit::qi::_val = 23] |
                  boost::spirit::lit("dlbc")  [boost::spirit::qi::_val = 24] |
                  boost::spirit::lit("dlbm")  [boost::spirit::qi::_val = 25] |
                  boost::spirit::lit("dlhuc") [boost::spirit::qi::_val = 26] |
                  boost::spirit::lit("dlhum") [boost::spirit::qi::_val = 27] |
                  boost::spirit::lit("dlbuc") [boost::spirit::qi::_val = 28] |
                  boost::spirit::lit("dlbum") [boost::spirit::qi::_val = 29] ;

        LDT = (Pred >> LDTopc >> GPR >> '='
                >> '[' >> GPR >> Addresse >> ']')
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 ldt_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        LDTs = (Pred >> LDTsopc >> GPR >> '='
                >> '[' >> GPR >> Addresse >> ']')
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 ldt_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        dLDT = (Pred >> dLDTopc >> "sm" >> '='
                >> '[' >> GPR >> Addresse >> ']')
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 ldt_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4)];

        // Parse STT instructions
        STTopc = boost::spirit::lit("sws")   [boost::spirit::qi::_val =  0] |
                 boost::spirit::lit("swl")   [boost::spirit::qi::_val =  1] |
                 boost::spirit::lit("swc")   [boost::spirit::qi::_val =  2] |
                 boost::spirit::lit("swm")   [boost::spirit::qi::_val =  3] |
                 boost::spirit::lit("shs")   [boost::spirit::qi::_val =  4] |
                 boost::spirit::lit("shl")   [boost::spirit::qi::_val =  5] |
                 boost::spirit::lit("shc")   [boost::spirit::qi::_val =  6] |
                 boost::spirit::lit("shm")   [boost::spirit::qi::_val =  7] |
                 boost::spirit::lit("sbs")   [boost::spirit::qi::_val =  8] |
                 boost::spirit::lit("sbl")   [boost::spirit::qi::_val =  9] |
                 boost::spirit::lit("sbc")   [boost::spirit::qi::_val = 10] |
                 boost::spirit::lit("sbm")   [boost::spirit::qi::_val = 11] ;

        STTsopc = boost::spirit::lit("sws")   [boost::spirit::qi::_val =  0] |
                  boost::spirit::lit("shs")   [boost::spirit::qi::_val =  4] |
                  boost::spirit::lit("sbs")   [boost::spirit::qi::_val =  8] ;

        STT = (Pred >> STTopc >> '[' >> GPR >> Addresse >> ']'
                >> '=' >> GPR)
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 stt_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        STTs = (Pred >> STTsopc >> '[' >> GPR >> Addresse >> ']'
                >> '=' >> GPR)
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 stt_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3,
                                 boost::spirit::qi::_4, boost::spirit::qi::_5)];

        // Parse STCt instructions
        STCopc = boost::spirit::lit("sres")    [boost::spirit::qi::_val = 0] |
                 boost::spirit::lit("sens")    [boost::spirit::qi::_val = 1] |
                 boost::spirit::lit("sfree")   [boost::spirit::qi::_val = 2] ;

        STC = (Pred >> STCopc >> Imm22u)
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 stc_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3)];

        // Parse PFLb instructions
        PFLbopc = boost::spirit::lit("bs")    [boost::spirit::qi::_val = 0] |
                  boost::spirit::lit("b")     [boost::spirit::qi::_val = 2] ;

        PFLb = ((Pred >> "bc" >> ImmPFL_FREL)
                [boost::spirit::qi::_val = boost::phoenix::bind(
                                   pflb_format_t::encode, boost::spirit::qi::_1,
                                   1, boost::spirit::qi::_2)])                 |
               ((Pred >> PFLbopc >> ImmPFL_ABS)
                [boost::spirit::qi::_val = boost::phoenix::bind(
                               pflb_format_t::encode, boost::spirit::qi::_1,
                               boost::spirit::qi::_2, boost::spirit::qi::_3)]);

        // Parse PFLi instructions
        PFLiopc = boost::spirit::lit("bsr")    [boost::spirit::qi::_val = 0] |
                  boost::spirit::lit("bcr")    [boost::spirit::qi::_val = 1] |
                  boost::spirit::lit("br")     [boost::spirit::qi::_val = 2] ;

        PFLi = (Pred >> PFLiopc >> GPR)
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                 pfli_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3)];

        // Parse PFLr instructions
        PFLropc = boost::spirit::lit("ret")[boost::spirit::qi::_val = 0];
        PFLr = (Pred >> PFLropc)
              [boost::spirit::qi::_val = boost::phoenix::bind(
                                   pflr_format_t::encode, boost::spirit::qi::_1,
                                   boost::spirit::qi::_2)];

        // Parse BNE instructions
        BNE = ("bne" >> GPR >> "!=" >> GPR >> ',' >> Imm7s)
               [boost::spirit::qi::_val = boost::phoenix::bind(
                                 bne_format_t::encode, boost::spirit::qi::_1,
                                 boost::spirit::qi::_2, boost::spirit::qi::_3)];

        // enable debugging of rules -- if BOOST_SPIRIT_DEBUG is defined
        BOOST_SPIRIT_DEBUG_NODE(Line);
        BOOST_SPIRIT_DEBUG_NODE(Bundle);
        BOOST_SPIRIT_DEBUG_NODE(Name);        BOOST_SPIRIT_DEBUG_NODE(Label);
        BOOST_SPIRIT_DEBUG_NODE(Instruction1);
        BOOST_SPIRIT_DEBUG_NODE(Instruction2);
        BOOST_SPIRIT_DEBUG_NODE(GPR);         BOOST_SPIRIT_DEBUG_NODE(SPR);
        BOOST_SPIRIT_DEBUG_NODE(PRR);         BOOST_SPIRIT_DEBUG_NODE(NegPRR);
        BOOST_SPIRIT_DEBUG_NODE(Imm);
        BOOST_SPIRIT_DEBUG_NODE(Imm4u);       BOOST_SPIRIT_DEBUG_NODE(Imm12u);
        BOOST_SPIRIT_DEBUG_NODE(Imm22s);      BOOST_SPIRIT_DEBUG_NODE(Imm7s);
        BOOST_SPIRIT_DEBUG_NODE(Imm22u);      BOOST_SPIRIT_DEBUG_NODE(ImmALUL);
        BOOST_SPIRIT_DEBUG_NODE(ImmPFL_ABS);  BOOST_SPIRIT_DEBUG_NODE(ImmALUI);
        BOOST_SPIRIT_DEBUG_NODE(ImmPFL_FREL);
        BOOST_SPIRIT_DEBUG_NODE(Pred);
        BOOST_SPIRIT_DEBUG_NODE(ALUiopc);     BOOST_SPIRIT_DEBUG_NODE(ALUlropc);
        BOOST_SPIRIT_DEBUG_NODE(ALUuopc);     BOOST_SPIRIT_DEBUG_NODE(ALUmopc);
        BOOST_SPIRIT_DEBUG_NODE(ALUcopc);     BOOST_SPIRIT_DEBUG_NODE(ALUpopc);
        BOOST_SPIRIT_DEBUG_NODE(SPCwopc);
        BOOST_SPIRIT_DEBUG_NODE(LDTopc);      BOOST_SPIRIT_DEBUG_NODE(dLDTopc);
        BOOST_SPIRIT_DEBUG_NODE(STTopc);
        BOOST_SPIRIT_DEBUG_NODE(STCopc);
        BOOST_SPIRIT_DEBUG_NODE(PFLbopc);     BOOST_SPIRIT_DEBUG_NODE(PFLiopc);
        BOOST_SPIRIT_DEBUG_NODE(PFLropc);
        BOOST_SPIRIT_DEBUG_NODE(ALUi);        BOOST_SPIRIT_DEBUG_NODE(ALUl);
        BOOST_SPIRIT_DEBUG_NODE(ALUr);        BOOST_SPIRIT_DEBUG_NODE(ALUu);
        BOOST_SPIRIT_DEBUG_NODE(ALUm);        BOOST_SPIRIT_DEBUG_NODE(ALUc);
        BOOST_SPIRIT_DEBUG_NODE(ALUp);
        BOOST_SPIRIT_DEBUG_NODE(SPCn);        BOOST_SPIRIT_DEBUG_NODE(SPCw);
        BOOST_SPIRIT_DEBUG_NODE(SPCt);        BOOST_SPIRIT_DEBUG_NODE(SPCf);
        BOOST_SPIRIT_DEBUG_NODE(LDT);         BOOST_SPIRIT_DEBUG_NODE(dLDT);
        BOOST_SPIRIT_DEBUG_NODE(STT);
        BOOST_SPIRIT_DEBUG_NODE(STC);
        BOOST_SPIRIT_DEBUG_NODE(PFLb);        BOOST_SPIRIT_DEBUG_NODE(PFLi);
        BOOST_SPIRIT_DEBUG_NODE(PFLr);
        BOOST_SPIRIT_DEBUG_NODE(BNE);
      }
  };


  line_assembler_t::line_assembler_t() :
      Line_parser(* new assembly_line_grammar_t())
  {
  }

  line_assembler_t::~line_assembler_t()
  {
    delete &Line_parser;
  }

  bool line_assembler_t::parse_line(const std::string &line, dword_t &iw) const
  {
    return boost::spirit::qi::phrase_parse(line.begin(), line.end(),
                                  Line_parser, boost::spirit::ascii::space, iw);
  }

  bool line_assembler_t::write_program(std::ostream &out,
                                       unsigned int &size) const
  {
    std::ostream_iterator<char> out_iterator(out);

    // emit program
    for(unsigned int i = 0; i < Line_parser.P.size(); i++)
    {
      // get instruction word
      word_t iw = Line_parser.P[i];

      // apply relocation, if needed
      relocations_t::const_iterator r(Line_parser.Relocations.find(i));
      if (r != Line_parser.Relocations.end())
      {
        // see if label was defined somewhere
        if (Line_parser.Symbols.find(r->second.Symbol) ==
            Line_parser.Symbols.end())
        {
          for(names_t::const_iterator j(Line_parser.Names.begin()),
              je(Line_parser.Names.end()); j != je; j++)
          {
            if (j->second == r->second.Symbol)
            {
              std::cerr << "Undefined Label: " << j->first << "\n";
              return false;
            }
          }

          std::cerr << "Undefined Label.\n";
          return false;
        }

        word_t address = Line_parser.Symbols[r->second.Symbol];
        switch(r->second.Kind)
        {
          case ALUL_ABS:
            // fix-up next word.
            Line_parser.P[i + 1] = address;
            break;
          case ALUI_ABS:
            // patch ALUi instruction -- byte addressing
            iw = (iw & ~0xFFF) | (address & 0xFFF);
            break;
          case PFLB_ABS:
            // patch branch -- word addressing
            iw = (iw & ~0x3FFFFF) | ((address >> 2) & 0x3FFFFF);
            break;
          case PFLB_FREL:
            // patch branch -- word addressing
            iw = (iw & ~0x3FFFFF) | (((address - r->second.Last_fstart) >> 2) &
                                     0x3FFFFF);
            break;
        }
      }

      // write as big endian
      boost::spirit::karma::generate(out_iterator,
                                     boost::spirit::big_dword(iw));
    }

    size = Line_parser.P.size();
    return true;
  }
}


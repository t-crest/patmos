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
// Concrete definition of instruction formats, defining the binary encoding of
// concrete instructions.
//


#ifndef PATMOS_BINARY_FORMATS_H
#define PATMOS_BINARY_FORMATS_H

#include "binary-format.h"

namespace patmos
{
  /// The ALUi instruction format (see Patmos TR).
  class alui_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alui_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUl instruction format (see Patmos TR).
  class alul_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alul_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUr instruction format (see Patmos TR).
  class alur_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alur_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUm instruction format (see Patmos TR).
  class alum_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alum_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUc instruction format (see Patmos TR).
  class aluc_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    aluc_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUci instruction format (see Patmos TR).
  class aluci_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    aluci_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUp instruction format (see Patmos TR).
  class alup_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alup_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The ALUb instruction format (see Patmos TR).
  class alub_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alub_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The SPCt instruction format (see Patmos TR).
  class spct_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    spct_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The SPCf instruction format (see Patmos TR).
  class spcf_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    spcf_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The LDT instruction format (see Patmos TR).
  class ldt_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    /// @param is_stack Flag indicating whether the instruction accesses the
    /// stack.
    ldt_format_t(const instruction_t &instruction, word_t opcode,
                 bool is_stack = false);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The STT instruction format (see Patmos TR).
  class stt_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    /// @param is_stack Flag indicating whether the instruction accesses the
    /// stack.
    stt_format_t(const instruction_t &instruction, word_t opcode,
                 bool is_stack = false);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The STCi instruction format (see Patmos TR).
  class stci_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    stci_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The STCr instruction format (see Patmos TR).
  class stcr_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    stcr_format_t(const instruction_t &instruction, word_t opcode);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The CFLi instruction format (see Patmos TR).
  class cfli_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cfli_format_t(const instruction_t &instruction, word_t opcode, word_t flag);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The CFLri instruction format (see Patmos TR).
  class cflri_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cflri_format_t(const instruction_t &instruction, word_t opcode, word_t flag);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The CFLrs instruction format (see Patmos TR).
  class cflrs_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cflrs_format_t(const instruction_t &instruction, word_t opcode, word_t flag);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

  /// The CFLrt instruction format (see Patmos TR).
  class cflrt_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cflrt_format_t(const instruction_t &instruction, word_t opcode, word_t flag);

    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    virtual bool parse_operands(line_parser_t &parser, std::string mnemonic,
                                instruction_data_t &instr,
                                reloc_info_t &reloc) const;

    virtual udword_t encode(std::string mnemonic,
                          const instruction_data_t &instr) const;
  };

}

#endif // PATMOS_BINARY_FORMATS_H

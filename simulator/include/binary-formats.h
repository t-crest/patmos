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

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rd The destination register.
    /// @param rs1 The source register operand.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rd, word_t rs1,
                         word_t imm);
  };

  /// The ALUl instruction format (see Patmos TR).
  class alul_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alul_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rd The destination register.
    /// @param rs1 The source register operand.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static dword_t encode(word_t pred, word_t opcode, word_t rd, word_t rs1,
                          word_t imm);
  };

  /// The ALUr instruction format (see Patmos TR).
  class alur_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alur_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rd The destination register.
    /// @param rs1 The first source register operand.
    /// @param rs2 The second source register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rd, word_t rs1,
                         word_t rs2);
  };

  /// The ALUm instruction format (see Patmos TR).
  class alum_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alum_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rs1 The first source register operand.
    /// @param rs2 The second source register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rs1, word_t rs2);
  };

  /// The ALUc instruction format (see Patmos TR).
  class aluc_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    aluc_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param pd The destination predicate register.
    /// @param rs1 The first source register operand.
    /// @param rs2 The second source register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t pd, word_t rs1,
                         word_t rs2);
  };

  /// The ALUp instruction format (see Patmos TR).
  class alup_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    alup_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param pd The destination predicate register.
    /// @param ps1 The first predicate register operand.
    /// @param ps2 The second predicate register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t pd, word_t ps1,
                         word_t ps2);
  };

  /// The SPCw instruction format (see Patmos TR).
  class spcw_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    spcw_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode);
  };

  /// The SPCt instruction format (see Patmos TR).
  class spct_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    spct_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param sd The destination special register.
    /// @param rs1 The source register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t sd, word_t rs1);
  };

  /// The SPCf instruction format (see Patmos TR).
  class spcf_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    spcf_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param rd The destination register.
    /// @param ss The source special register.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t rd, word_t ss);
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

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rd The destination register.
    /// @param ra The address register operand.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rd, word_t ra,
                         word_t imm);

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param ra The address register operand.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t ra, word_t imm);
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

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param ra The address register operand.
    /// @param imm The immediate operand.
    /// @param rs The source register.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t ra, word_t imm,
                         word_t rs);
  };

  /// The STCi instruction format (see Patmos TR).
  class stci_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    stci_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t imm);
  };

  /// The STCr instruction format (see Patmos TR).
  class stcr_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    stcr_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rs The register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rs);
  };

  /// The CFLb instruction format (see Patmos TR).
  class cflb_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cflb_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param imm The immediate operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t imm);
  };

  /// The CFLi instruction format (see Patmos TR).
  class cfli_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cfli_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rs1 The register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rs1);
  };

  /// The CFLr instruction format (see Patmos TR).
  class cflr_format_t : public binary_format_t
  {
  public:
    /// Construct a new binary format for the instruction using a given opcode.
    /// @param instruction The instruction.
    /// @param opcode The instruction's opcode.
    cflr_format_t(const instruction_t &instruction, word_t opcode);

    /// Decode the operands of the instruction and return a corresponding
    /// instruction data instance.
    /// @param iw The instruction word.
    /// @param longimm A long immediate (exclusively for the ALUl format),
    /// @return The resulting instruction data instance representing the
    /// instruction and its operands.
    virtual instruction_data_t decode_operands(word_t iw, word_t longimm) const;

    /// Encode an instruction.
    /// @param pred The instruction's predicate.
    /// @param opcode The instruction opcode.
    /// @param rb The function base register operand.
    /// @param ro The offset register operand.
    /// @return An encoded instruction word.
    static word_t encode(word_t pred, word_t opcode, word_t rb, word_t ro);
  };

}

#endif // PATMOS_BINARY_FORMATS_H

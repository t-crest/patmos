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
// Basic definitions of interfaces to simulation functions for Patmos
// instructions.
//

#ifndef PATMOS_INSTRUCTION_H
#define PATMOS_INSTRUCTION_H

#include "registers.h"

#include <ostream>
#include <vector>

namespace patmos
{
  // forward declaration
  class simulator_t;
  class symbol_map_t;
  class instruction_t;
  class instruction_data_t;

  /// Base class for all Patmos instructions.
  class instruction_t
  {
  public:
    /// ID of the instruction. Negative for pseudo instructions.
    int ID;

    /// Instruction name.
    const char *Name;

    virtual ~instruction_t() {}

    // -------------------------- UTILITY --------------------------------------

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os,
                       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const = 0;

    /// Print the instruction's operands to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os,
                       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const = 0;

    /// Returns true if the instruction is a flow control instruction
    virtual bool is_flow_control() const = 0;

    virtual bool is_call() const { return false; }
    
    /// Returns the number of delay slot cycles of this instruction
    virtual unsigned get_delay_slots() const = 0;
    
    /// Reset all statistic counters.
    virtual void reset_stats() { }

    /// Collect statistic for instruction instance and add to stats counters.
    virtual void collect_stats(const simulator_t &s,
                               const instruction_data_t &ops) { }

    /// Print statistics for this instruction type.
    /// This should print statistics as one line without newlines, and
    /// always format it as comma-separated list of '<name>: <value>' pairs.
    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const symbol_map_t &symbols) const { }

    // -------------------------- SIMULATION -----------------------------------

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of a decoupled load
    /// instruction running in parallel to the pipeline.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void dMW(simulator_t &s, instruction_data_t &ops) const = 0;
  };

  /// Data structure to keep data of instructions while executing.
  class instruction_data_t
  {
  public:
    /// The instruction class that implements the behavior.
    const instruction_t *I;

    /// Address the instruction was fetched from.
    uword_t Address;

    /// The predicate under which the instruction is executed.
    PRR_e Pred;

    /// Union to keep operand information depending on instruction classes.
    union
    {
      /// Operands for an ALUi or ALUl instruction.
      struct
      {
        GPR_e Rd;
        GPR_e Rs1;
        word_t Imm2;
      } ALUil;
      /// Operands for an ALUr instruction.
      struct
      {
        GPR_e Rd;
        GPR_e Rs1;
        GPR_e Rs2;
      } ALUr;
      /// Operands for an ALUm instruction.
      struct
      {
        GPR_e Rs1;
        GPR_e Rs2;
      } ALUm;
      /// Operands for an ALUc instruction.
      struct
      {
        PRR_e Pd;
        GPR_e Rs1;
        GPR_e Rs2;
      } ALUc;
      /// Operands for an ALUp instruction.
      struct
      {
        PRR_e Pd;
        PRR_e Ps1;
        PRR_e Ps2;
      } ALUp;
      /// Operands for an SPCn instruction.
      struct
      {
        word_t Imm;
      } SPCn;
      /// Operands for an SPCt instruction.
      struct
      {
        SPR_e Sd;
        GPR_e Rs1;
      } SPCt;
      /// Operands for an SPCf instruction.
      struct
      {
        GPR_e Rd;
        SPR_e Ss;
      } SPCf;
      /// Operands for an LDT instruction.
      struct
      {
        GPR_e Rd;
        GPR_e Ra;
        word_t Imm;
      } LDT;
      /// Operands for an STT instruction.
      struct
      {
        GPR_e Ra;
        GPR_e Rs1;
        word_t Imm2;
      } STT;
      /// Operands for an STCi instruction.
      struct
      {
        word_t Imm;
      } STCi;
      /// Operands for an STCr instruction.
      struct
      {
        GPR_e Rs;
      } STCr;
      /// Operands for an CFLi instruction.
      struct
      {
        word_t Imm;
        uword_t UImm;
      } CFLi;
      /// Operands for an CFLri instruction.
      struct
      {
      } CFLri;
      /// Operands for an CFLrs instruction.
      struct
      {
        GPR_e Rs;
      } CFLrs;
      /// Operands for an CFLrt instruction.
      struct
      {
        GPR_e Rs1;
        GPR_e Rs2;
      } CFLrt;
    } OPS;


    // -------------------------- DR -------------------------------------------

    /// Decoded immediate from DR stage.
    word_t DR_Imm;

    /// Read value from special register at the DR stage.
    word_t DR_Ss;

    /// Read value from special register at the DR stage.
    word_t DR_St;

    /// Read value from first general register operand at the DR stage.
    GPR_op_t DR_Rs1;

    /// Read value from second general register operand at the DR stage.
    GPR_op_t DR_Rs2;

    /// Read value from first predicate register operand at the DR stage.
    bit_t DR_Ps1;

    /// Read value from second predicate register operand at the DR stage.
    bit_t DR_Ps2;

    /// Value of the instruction's predicate as read at the DR stage.
    bit_t DR_Pred;

    /// Current base address of the method cache, as read at the DR stage.
    GPR_op_t DR_Base;

    /// Current method offset of the method cache, as read at the DR stage.
    GPR_op_t DR_Offset;

    // -------------------------- EX -------------------------------------------

    /// TODO make a union with mull/mulh, ss/st and base/offset/.. ?

    /// Result from EX stage.
    word_t EX_result;

    /// Result of a multiplication
    word_t EX_mull;

    /// Result of a multiplication
    word_t EX_mulh;

    /// New stack spill pointer from EX stage.
    word_t EX_Ss;
    
    /// New stack top pointer from EX stage.
    word_t EX_St;
    
    /// Result register operand from EX stage.
    GPR_by_pass_t GPR_EX_Rd;

    /// Result predicate operand from EX stage.
    //PRR_by_pass_t PRR_EX_Pd;

    /// Value for memory stores.
    word_t EX_Rs;

    /// Address for memory accesses.
    word_t EX_Address;

    /// Method base address as read in EX stage.
    word_t EX_Base;

    /// Method offset as read in EX stage.
    word_t EX_Offset;

    // -------------------------- MW -------------------------------------------
    /// Discard instructions in MW stage (stalling).
    word_t MW_Discard;
    
    /// Keep track if this is the first cycle in the MW stage.
    word_t MW_Initialized;

    // ------------------------ CONSTRUCTOR  -----------------------------------

    /// Create a bubble..
    instruction_data_t();

    /// Create an instruction with a predicate.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    instruction_data_t(const instruction_t &i, PRR_e pred);

    // -------------------- CONSTRUCTOR FUNCTIONS ------------------------------

    /// Create an ALUi or ALUl instruction with a register operands, an
    /// immediate, and a register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rd The destination register.
    /// @param rs1 The first operand register.
    /// @param imm2 The second immediate operand.
    static instruction_data_t mk_ALUil(const instruction_t &i, PRR_e pred,
                                       GPR_e rd, GPR_e rs1, word_t imm2);

    /// Create an ALUr instruction with two register operands and a register
    /// destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rd The destination register.
    /// @param rs1 The first operand register.
    /// @param rs2 The second operand register.
    static instruction_data_t mk_ALUr(const instruction_t &i, PRR_e pred,
                                      GPR_e rd, GPR_e rs1, GPR_e rs2);

    /// Create an ALUm instruction with two register operands.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rs1 The first operand register.
    /// @param rs2 The second operand register.
    static instruction_data_t mk_ALUm(const instruction_t &i, PRR_e pred,
                                      GPR_e rs1, GPR_e rs2);

    /// Create an ALUc instruction with two register operands and a predicate
    /// register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param pd The predicate destination register.
    /// @param rs1 The first operand register.
    /// @param rs2 The second operand register.
    static instruction_data_t mk_ALUc(const instruction_t &i, PRR_e pred,
                                      PRR_e pd, GPR_e rs1, GPR_e rs2);

    /// Create an ALUp instruction with two predicate register operands and a
    /// predicate register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param pd The predicate destination register.
    /// @param ps1 The first operand predicate register.
    /// @param ps2 The second operand predicate register.
    static instruction_data_t mk_ALUp(const instruction_t &i, PRR_e pred,
                                      PRR_e pd, PRR_e ps1, PRR_e ps2);

    /// Create an SPCn instruction with an immediate operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param imm The immediate operand.
    static instruction_data_t mk_SPCn(const instruction_t &i, PRR_e pred,
                                      word_t imm);

    /// Create an SPCw instruction without operands.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    static instruction_data_t mk_SPCw(const instruction_t &i, PRR_e pred);

    /// Create an SPCt instruction with a register operand and a special
    /// register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param sd The special destination register.
    /// @param rs1 The register operand.
    static instruction_data_t mk_SPCt(const instruction_t &i, PRR_e pred,
                                      SPR_e sd, GPR_e rs1);

    /// Create an SPCf instruction with a special register operand and a
    /// register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rd The destination register.
    /// @param ss The special register operand.
    static instruction_data_t mk_SPCf(const instruction_t &i, PRR_e pred,
                                      GPR_e rd, SPR_e ss);

    /// Create an LDT instruction with a register operand, an immediate operand,
    /// and a register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rd The destination register.
    /// @param ra The first operand register.
    /// @param imm The second operand immediate.
    static instruction_data_t mk_LDT(const instruction_t &i, PRR_e pred,
                                     GPR_e rd, GPR_e ra, word_t imm);

    /// Create an STT instruction with two register operands and an immediate
    /// operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param ra The address operand register.
    /// @param rs1 The first operand register.
    /// @param imm2 The second operand immediate.
    static instruction_data_t mk_STT(const instruction_t &i, PRR_e pred,
                                     GPR_e ra, GPR_e rs1, word_t imm2);

    /// Create an STC instruction with an immediate operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param imm The operand immediate.
    static instruction_data_t mk_STCi(const instruction_t &i, PRR_e pred,
                                      word_t imm);

    /// Create an STC instruction with a register operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param imm The operand register.
    static instruction_data_t mk_STCr(const instruction_t &i, PRR_e pred,
                                      GPR_e rs);

    /// Create an CFLi instruction with an immediate operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param imm The operand immediate.
    static instruction_data_t mk_CFLi(const instruction_t &i, PRR_e pred,
                                      word_t imm, uword_t uimm);

    /// Create an CFLr instruction with implicit operands.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    static instruction_data_t mk_CFLri(const instruction_t &i, PRR_e pred);

    /// Create an CFLr instruction with a single register operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rs The operand register.
    static instruction_data_t mk_CFLrs(const instruction_t &i, PRR_e pred,
                                       GPR_e rs);

    /// Create an CFLr instruction with two register operands.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rs1 The first operand register.
    /// @param rs2 The second operand register.
    static instruction_data_t mk_CFLrt(const instruction_t &i, PRR_e pred,
                                       GPR_e rs1, GPR_e rs2);

    /// Create an HLT instruction without operands.
    /// @param i The instruction.
    static instruction_data_t mk_HLT(const instruction_t &i);

    // ------------------------ UTILITY ----------------------------------------

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    void print(std::ostream &os, const symbol_map_t &symbols) const
    {
      if (I)
        I->print(os, *this, symbols);
      else
        os << "bubble";
    }

    /// Print the instruction's operands to an output stream.
    /// @param os The output stream to print to.
    /// @param symbols A mapping of addresses to symbols.
    void print_operands(const simulator_t &s, std::ostream &os, 
	       const symbol_map_t &symbols) const
    {
      if (I) {
	if (DR_Pred) {
          I->print_operands(s, os, *this, symbols);
	} else {
	  os << "skipped";
	}
      }
    }


    // ------------------------ SIMULATION -------------------------------------

    /// Invoke the DR simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void DR(simulator_t &s)
    {
      if (I)
        I->DR(s, *this);
    }

    /// Invoke the EX simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void EX(simulator_t &s)
    {
      if (I)
        I->EX(s, *this);
    }

    /// Invoke the MW simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void MW(simulator_t &s)
    {
      if (I)
        I->MW(s, *this);
    }

    /// Invoke the dMW simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void dMW(simulator_t &s)
    {
      if (I)
        I->dMW(s, *this);
    }
  };
}

#endif // PATMOS_INSTRUCTION_H

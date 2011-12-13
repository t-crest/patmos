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
  struct instruction_data_t;

  /// Representation of a program.
  typedef std::vector<instruction_data_t> program_t;
  
  /// Base class for all Patmos instructions.
  /// Every instruction consists of several "pipeline" functions, that operate
  /// on a shadow state of the processor, once all pipeline stages have 
  /// completed the simulation of a cycle a series of "commit"  functions are 
  /// invoked in order to commit the shadow state to the global processor state.  
  class instruction_t
  {
  public:   
    // -------------------------- UTILITY --------------------------------------
    
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os,
                       const instruction_data_t &ops) const = 0;

    // -------------------------- SIMULATION -----------------------------------
    
    /// Pipeline function to simulate the behavior of the instruction in
    /// the fetch pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void fetch(simulator_t &s, instruction_data_t &ops) const = 0;
    
    /// Commit function to commit the shadow state of the instruction in
    /// the fetch pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void fetch_commit(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void decode(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Commit function to commit the shadow state of the instruction in
    /// the decode pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void decode_commit(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void execute(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Commit function to commit the shadow state of the instruction in
    /// the execute pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void execute_commit(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void memory(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Commit function to commit the shadow state of the instruction in
    /// the memory pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void memory_commit(simulator_t &s, instruction_data_t &ops) const = 0;

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void writeback(simulator_t &s, instruction_data_t &ops) const = 0;
    
    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.

    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const = 0;
  };

  /// Data structure to keep data of instructions while executing.
  struct instruction_data_t
  {
  public:
    /// The instruction class that implements the behavior.
    const instruction_t *I;

    /// Flag indicating whether the bundle ends with this instruction.
    bool Bundle_end;
    
    PRR_e Pred;
    
    /// Union to keep operand information depending on instruction classes.
    union
    {
      /// Operands of compare instructions.
      struct {
        PRR_e Pd;
        GPR_e Rs1;
        GPR_e Rs2;
      } CMP;
      /// Operands of instructions having two register operands and a register
      /// destination.
      struct {
        GPR_e Rd;
        GPR_e Rs1;
        GPR_e Rs2;
      } RRR;
      /// Operands of instructions having on register operand, one immediate
      /// operand, and a register destination.
      struct
      {
        GPR_e Rd;
        GPR_e Rs1;
        word_t Imm2;
      } RRI;
      /// Operands of load high/low instructions.
      struct
      {
        GPR_e Rd;
        word_t Imm;
      } LDI;
      /// Operands of memory load instructions.
      struct
      {
        GPR_e Rd;
        GPR_e Ra;
        word_t Imm;
      } LD;
      /// Operands of memory store instructions.
      struct
      {
        GPR_e Ra;
        GPR_e Rs;
        word_t Imm;
      } ST;
      /// Operand of instructions with a single immediate operand.
      struct
      {
        word_t Imm;
      } I;
      /// Operand of call instructions.
      struct
      {
        GPR_e Ra;
      } JSR;
    } OPS;

    // ------------------------ DECODE -----------------------------------------
    GPR_op_t DE_Rs1;
    GPR_op_t DE_Rs2;
    PRR_op_t DE_Pred;
    word_t DE_PC;

    // ------------------------ EXECUTE ----------------------------------------
    /// Result register operand from execute stage.
    GPR_by_pass_t GPR_EX_Rd;

    /// Result predicate operand from execute stage.
    PRR_by_pass_t PRR_EX_Pd;

    /// Final value of the predicate to be used.
    bit_t EX_Pred;

    /// Value for memory stores.
    word_t EX_Rs;
    
    /// Address for memory accesses.
    word_t EX_Address;

    // ------------------------ MEMORY -----------------------------------------
    /// Result register operand from memory stage.
    GPR_by_pass_t GPR_MEM_Rd;

    /// Result predicate operand from memory stage.
    PRR_by_pass_t PRR_MEM_Pd;

    // ------------------------ WRITEBACK --------------------------------------
    /// Result register operand from writeback stage.
    GPR_by_pass_t GPR_WB_Rd;

    /// Result predicate operand from writeback stage.
    PRR_by_pass_t PRR_WB_Pd;

    // ------------------------ CONSTRUCTOR  -----------------------------------
    /// Construct a bubble "instruction", i.e., this will not even increments 
    /// the PC.
    instruction_data_t();

    /// Create an ALU instruction with two register operands and a register
    /// destination.
    /// @param i The instruction.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, bool bundle_end = true);

    /// Create an ALU instruction with two register operands and a register 
    /// destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is 
    /// executed.
    /// @param rd The destination register.
    /// @param rs1 The first operand register.
    /// @param rs2 The first operand register.
    /// @param bundle_end Indicate whether the instruction is the last of a 
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, GPR_e rd,
                       GPR_e rs1, GPR_e rs2, bool bundle_end = true);
    
    /// Create an ALU instruction with a register operands, an immediate, and a
    /// register destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param rd The destination register.
    /// @param rs1 The first operand register.
    /// @param imm2 The second immediate operand.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, GPR_e rd, GPR_e rs1,
                       word_t imm2, bool bundle_end = true);

    /// Create a comparison instruction with two register operands and a 
    /// predicate destination.
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param pd The destination predicate.
    /// @param rs1 The first operand register.
    /// @param rs2 The first operand register.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, PRR_e pd,
                       GPR_e rs1, GPR_e rs2, bool bundle_end = true);

    /// Create a branch or call instruction with an immediate operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the branch is executed.
    /// @param imm The immediate operand.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, word_t imm,
                       bool bundle_end = true);

    /// Create a call instruction with a register operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the branch is executed.
    /// @param ra The operand register.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, GPR_e ra,
                       bool bundle_end = true);

    /// Create a call instruction without any operand.
    /// @param i The instruction.
    /// @param pred The predicate register under which the branch is executed.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred,
                       bool bundle_end = true);

    /// Create an ALU instruction with two register operands and an immediate
    /// (stores).
    /// @param i The instruction.
    /// @param pred The predicate register under which the instruction is
    /// executed.
    /// @param ra The first operand register.
    /// @param imm The second immediate operand.
    /// @param rs The destination register.
    /// @param bundle_end Indicate whether the instruction is the last of a
    /// bundle.
    instruction_data_t(const instruction_t &i, PRR_e pred, GPR_e ra,
                       word_t imm, GPR_e rs, bool bundle_end = true);

    // ------------------------ UTILITY ----------------------------------------

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    virtual void print(std::ostream &os) const
    {
      if (I)
        I->print(os, *this);
      else
        os << "bubble";
    }

    // ------------------------ SIMULATION -------------------------------------

    /// Invoke the fetch simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void fetch(simulator_t &s)
    {
      if (I)
        I->fetch(s, *this);
    }

    /// Invoke the fetch commit function.
    /// @param s The Patmos simulator executing the instruction.
    void fetch_commit(simulator_t &s)
    {
      if (I)
        I->fetch_commit(s, *this);
    }

    /// Invoke the decode simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void decode(simulator_t &s)
    {
      if (I)
        I->decode(s, *this);
    }

    /// Invoke the decode commit function.
    /// @param s The Patmos simulator executing the instruction.
    void decode_commit(simulator_t &s)
    {
      if (I)
        I->decode_commit(s, *this);
    }

    /// Invoke the execute simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void execute(simulator_t &s)
    {
      if (I)
        I->execute(s, *this);
    }

    /// Invoke the execute commit function.
    /// @param s The Patmos simulator executing the instruction.
    void execute_commit(simulator_t &s)
    {
      if (I)
        I->execute_commit(s, *this);
    }

    /// Invoke the memory simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void memory(simulator_t &s)
    {
      if (I)
        I->memory(s, *this);
    }

    /// Invoke the memory commit function.
    /// @param s The Patmos simulator executing the instruction.
    void memory_commit(simulator_t &s)
    {
      if (I)
        I->memory_commit(s, *this);
    }

    /// Invoke the writeback simulation function.
    /// @param s The Patmos simulator executing the instruction.
    void writeback(simulator_t &s)
    {
      if (I)
        I->writeback(s, *this);
    }

    /// Invoke the writeback commit function.
    /// @param s The Patmos simulator executing the instruction.
    void writeback_commit(simulator_t &s)
    {
      if (I)
        I->writeback_commit(s, *this);
    }
  };
}

#endif // PATMOS_INSTRUCTION_H


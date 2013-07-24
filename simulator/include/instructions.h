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
// Definition of simulation functions for every Patmos instruction.
//

#ifndef PATMOS_INSTRUCTIONS_H
#define PATMOS_INSTRUCTIONS_H

#include "endian-conversion.h"
#include "data-cache.h"
#include "instruction.h"
#include "memory.h"
#include "method-cache.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "symbol.h"

#include <ostream>
#include <boost/format.hpp>

namespace patmos
{
  template<typename T>
  T &i_mk()
  {
    static T t;
    return t;
  }

  /// A NOP instruction, which does really nothing, except incrementing the PC.
  class i_nop_t : public instruction_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      os << "nop";
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
	               const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    { }

    /// Returs false, nop is not a flow control instruction
    virtual bool is_flow_control() const
    {
      return false;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the IF pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void IF(simulator_t &s, instruction_data_t &ops) const
    {
      // store the PC for PC-relative addressing in EX stage, if we are not stalling
      // NB: s.Stall is already set the first time s.PC is updated.
      //     If we checked (s.Stall==SIF), we would have to include i_pred_t::IF,
      //     which is rather ugly.
      if (s.PC != s.nPC) {
        ops.IF_PC = s.PC;
      }
      
      s.PC = s.nPC;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of a decoupled 
    /// instruction running in parallel to the pipeline.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void dMW(simulator_t &s, instruction_data_t &ops) const
    {
    }
  };

  /// Abstract base class of predicated instructions, i.e., all instructions
  /// that actually do something.
  class i_pred_t : public i_nop_t
  {
  protected:
    /// Read a GPR register at the EX stage.
    /// @param s The parent simulator.
    /// @param op The register operand.
    /// @return The register value, considering by-passing from the EX, and MW
    /// stages.
    static inline word_t read_GPR_EX(const simulator_t &s, GPR_op_t op)
    {
      // Note: Usually we would need forwarding from MW here, but since we
      // execute MW before EX stages and there is no WB stage, data is already
      // commited to the register file and DR stage already got the correct
      // value.
      // Also note that we read out EX bypass from MW, since the intruction
      // writing the bypass has already moved on in this cycle.
      //
      // If we modifiy the simulator to 5 stages or change the order of
      // execution, we need a bypass from MW to EX (but any EX bypass must have
      // precendence!), which must be fetched from WB stage.

      return s.Pipeline[SMW][1].GPR_EX_Rd.get(
             s.Pipeline[SMW][0].GPR_EX_Rd.get(
               op)).get();
    }

    /// Write the result of the EX stage into the bypass.
    static inline void store_GPR_EX_result(simulator_t &s,
                                           instruction_data_t &ops,
                                           GPR_e reg, word_t value)
    {
      if (ops.DR_Pred)
      {
        // store the result by writing it into a by-pass
        ops.GPR_EX_Rd.set(reg, value);
      }
    }

    /// Write the result of the MW stage into the bypass and to the register.
    static inline void store_GPR_MW_result(simulator_t &s,
                                           instruction_data_t &ops,
                                           GPR_e reg, word_t value)
    {
      if (ops.DR_Pred)
      {
        // Store to register file
        s.GPR.set(reg, value);
        // Resetting the EX bypass is not necessary, this instruction will be
        // dropped after this stage.
        //ops.GPR_EX_Rd.reset();
      }
    }

    /// Print a predicate to an output stream.
    /// @param os The output stream to print to.
    /// @param pred The predicate.
    /// @param bare If true, print the operand without parentheses,
    ///        also the default operand (always true).
    static inline void printPred(std::ostream &os, const PRR_e pred,
                                 bool bare=false)
    {
      int preg = pred & (NUM_PRR-1);
      bit_t pflag = (pred >> 3) & 1;

      if (bare) {
        if (pflag) os << "!";
        os << "p" << preg;
        return;
      }

      // hide the default predicate operand
      if (preg != p0 || pflag == true) {
        os << boost::format("(%1%p%2%) ") % ((pflag)?"!":" ") % preg;
      } else {
        // omit default predicate
        //os << "(!pN) ";
        os <<   "      ";
      }
    }

    static inline void printGPReg(std::ostream &os, const char* sep, 
          GPR_e reg, word_t val) 
    {
      // Not interested in r0, but this messes up the output format
      // if (reg == r0 && val == 0) return;
      os << boost::format("%1%r%2$-2d = %3$08x") % sep % reg % val;
    }
    
    static inline void printGPReg(std::ostream &os, const char* sep, 
          GPR_e reg, GPR_op_t op, const simulator_t &s) 
    {
      // Note: we are already after EX_commit, so EX bypasses already 
      // contain the new values, but we want the inputs.
      printGPReg(os, sep, reg, read_GPR_EX(s, op));
    }
        
    static inline void printSPReg(std::ostream &os, const char* sep, 
          SPR_e reg, word_t val) 
    {
      os << boost::format("%1%s%2$-2d = %3$08x") % sep % reg % val;
    }
    
    static inline void printPPReg(std::ostream &os, const char* sep,
          PRR_e reg, bool val)
    {
      os << boost::format("%1%p%2% = %3$1u") % sep % reg % val;
    }
    
    static inline void printPPReg(std::ostream &os, const char* sep,
          PRR_e reg, const simulator_t &s)
    {
      printPPReg(os, sep, reg, s.PRR.get(reg).get());
    }
    
    static inline void printSymbol(std::ostream &os, const char* name,
           word_t val,
           const symbol_map_t &symbols)
    {
      std::string sym = symbols.find(val);
      if (!sym.empty()) {
        os << name << ": " << sym;
      }      
    }
           
    
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      assert(false);
    }
  };

  /// Base class for ALUi or ALUl instructions.
  class i_aluil_t : public i_pred_t
  {
    uint64_t cnt_nops;
    uint64_t cnt_short_imm;
    uint64_t cnt_short_loads;
  public:
    i_aluil_t() { reset_stats(); }
    
    /// Compute the result of an ALUi or ALUl instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual word_t compute(word_t value1, word_t value2) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.ALUil.Rs1);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      // compute the result of the ALU instruction
      ops.EX_result = compute(read_GPR_EX(s, ops.DR_Rs1), ops.OPS.ALUil.Imm2);

      store_GPR_EX_result(s, ops, ops.OPS.ALUil.Rd, ops.EX_result);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      store_GPR_MW_result(s, ops, ops.OPS.ALUil.Rd, ops.EX_result);
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      // skip NOPs
      if (ops.OPS.ALUil.Rd == r0 && ops.OPS.ALUil.Rs1 == r0 && 
          ops.EX_result == 0 && ops.OPS.ALUil.Imm2 == 0) {
        os << "nop";
        return;
      }
      
      printGPReg(os, "out: ", ops.OPS.ALUil.Rd,  ops.EX_result);
      printGPReg(os, " in: ", ops.OPS.ALUil.Rs1, ops.DR_Rs1, s);

      printSymbol(os, " imm", ops.OPS.ALUil.Imm2, symbols);
    }
    
    virtual void reset_stats() {
      cnt_nops = 0;
      cnt_short_imm = 0;
      cnt_short_loads = 0;
    }
    
    virtual void collect_stats(const simulator_t &s, 
                               const instruction_data_t &ops) {
      if (ops.OPS.ALUil.Rd == r0 && 
          ops.OPS.ALUil.Rs1 == r0 &&
          ops.OPS.ALUil.Imm2 == 0) {
        ++cnt_nops;
      }
      else if (ops.OPS.ALUil.Rs1 == r0 && ops.OPS.ALUil.Imm2 < (1<<12)) {
        ++cnt_short_loads;
      }
      else if (ops.OPS.ALUil.Imm2 < (1<<12)) {
        ++cnt_short_imm;
      }
    }
    
    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const symbol_map_t &symbols) const {
      bool printed = false;
      if (cnt_nops) {
        os << "NOPs: " << cnt_nops;
        printed = true;
      }
      if (cnt_short_loads) {
        if (printed) os << ", ";
        os << "Short Load Imm: " << cnt_short_loads;
        printed = true;
      }
      if (cnt_short_imm) {
        if (printed) os << ", ";
        os << "Short Imm: " << cnt_short_imm;
      }
    }
  };

#define ALUil_INSTR(name, expr) \
  class i_ ## name ## _t : public i_aluil_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% r%2% = r%3%, %4%") % #name \
          % ops.OPS.ALUil.Rd % ops.OPS.ALUil.Rs1  % ops.OPS.ALUil.Imm2; \
      symbols.print(os, ops.OPS.ALUil.Imm2); \
    } \
    virtual word_t compute(word_t value1, word_t value2) const \
    { \
      return expr; \
    } \
  };

  ALUil_INSTR(addil , value1          +  value2                  )
  ALUil_INSTR(subil , value1          -  value2                  )
  ALUil_INSTR(xoril , value1          ^  value2                  )
  ALUil_INSTR(slil  , value1          << (value2 & 0x1F)         )
  ALUil_INSTR(sril  , (uword_t)value1 >> (uword_t)(value2 & 0x1F))
  ALUil_INSTR(srail , value1          >> (value2 & 0x1F)         )
  ALUil_INSTR(oril  , value1          |  value2                  )
  ALUil_INSTR(andil , value1          &  value2                  )

  ALUil_INSTR(norl   , ~(value1        |  value2)                 )
  ALUil_INSTR(shaddl , (value1 << 1)   +  value2                  )
  ALUil_INSTR(shadd2l, (value1 << 2)   +  value2                  )

  /// Base class for ALUr instructions.
  class i_alur_t : public i_pred_t
  {
  public:
    /// Compute the result of an ALUr instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual word_t compute(word_t value1, word_t value2) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.ALUr.Rs1);
      ops.DR_Rs2 = s.GPR.get(ops.OPS.ALUr.Rs2);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      // compute the result of the ALU instruction
      ops.EX_result = compute(read_GPR_EX(s, ops.DR_Rs1),
                              read_GPR_EX(s, ops.DR_Rs2));

      store_GPR_EX_result(s, ops, ops.OPS.ALUr.Rd, ops.EX_result);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      store_GPR_MW_result(s, ops, ops.OPS.ALUr.Rd, ops.EX_result);
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "out: ", ops.OPS.ALUr.Rd , ops.EX_result);
      printGPReg(os, " in: ", ops.OPS.ALUr.Rs1, ops.DR_Rs1, s);
      printGPReg(os, ", "   , ops.OPS.ALUr.Rs2, ops.DR_Rs2, s);
    }
  };

#define ALUr_INSTR(name, expr) \
  class i_ ## name ## _t : public i_alur_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% r%2% = r%3%, r%4%") % #name \
          % ops.OPS.ALUr.Rd % ops.OPS.ALUr.Rs1 % ops.OPS.ALUr.Rs2; \
    } \
    virtual word_t compute(word_t value1, word_t value2) const \
    { \
      return expr; \
    } \
  };

  ALUr_INSTR(add   , value1          +  value2                  )
  ALUr_INSTR(sub   , value1          -  value2                  )
  ALUr_INSTR(xor   , value1          ^  value2                  )
  ALUr_INSTR(sl    , value1          << (value2 & 0x1F)         )
  ALUr_INSTR(sr    , (uword_t)value1 >> (uword_t)(value2 & 0x1F))
  ALUr_INSTR(sra   , value1          >> (value2 & 0x1F)         )
  ALUr_INSTR(or    , value1          |  value2                  )
  ALUr_INSTR(and   , value1          &  value2                  )

  ALUr_INSTR(nor   , ~(value1        |  value2)                 )
  ALUr_INSTR(shadd , (value1 << 1)   +  value2                  )
  ALUr_INSTR(shadd2, (value1 << 2)   +  value2                  )

  /// Base class for ALUm instructions.
  class i_alum_t : public i_pred_t
  {
  public:
    /// Compute the result of an ALUr instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual dword_t compute(word_t value1, word_t value2) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.ALUm.Rs1);
      ops.DR_Rs2 = s.GPR.get(ops.OPS.ALUm.Rs2);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      // compute the result of the ALU instruction
      dword_t result = compute(read_GPR_EX(s, ops.DR_Rs1),
                               read_GPR_EX(s, ops.DR_Rs2));

      ops.EX_mull = (word_t)result;
      ops.EX_mulh = result >> sizeof(word_t)*8;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        s.SPR.set(sl, ops.EX_mull);
        s.SPR.set(sh, ops.EX_mulh);
      }
    }
    
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      // TODO result should actually not be available yet?
      printSPReg(os, "out: ", sl, ops.EX_mull);
      printSPReg(os, ", "   , sh, ops.EX_mulh);
      printGPReg(os, " in: ", ops.OPS.ALUm.Rs1, ops.DR_Rs1, s);
      printGPReg(os, ", "   , ops.OPS.ALUm.Rs2, ops.DR_Rs2, s);
    }

  };

#define ALUm_INSTR(name, type, stype) \
  class i_ ## name ## _t : public i_alum_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% r%2%, r%3%") % #name \
          % ops.OPS.ALUm.Rs1 % ops.OPS.ALUm.Rs2; \
    } \
    virtual dword_t compute(word_t value1, word_t value2) const \
    { \
      return ((type)(stype)value1) * ((type)(stype)value2); \
    } \
  };

  ALUm_INSTR(mul , dword_t, word_t)
  ALUm_INSTR(mulu, udword_t, uword_t)

  /// Base class for ALUc instructions.
  class i_aluc_t : public i_pred_t
  {
    uint64_t cnt_cmp_r0;
    uint64_t cnt_cmp_zero;
    uint64_t cnt_cmp_short_negimm;
    uint64_t cnt_cmp_short_imm;
    uint64_t cnt_cmp_short_uimm;
  public:
    i_aluc_t() { reset_stats(); }
    
    /// Compute the result of an ALUc instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual bit_t compute(word_t value1, word_t value2) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.ALUc.Rs1);
      ops.DR_Rs2 = s.GPR.get(ops.OPS.ALUc.Rs2);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        // compute the result of the comparison instruction
        bit_t result = compute(read_GPR_EX(s, ops.DR_Rs1),
                               read_GPR_EX(s, ops.DR_Rs2));

        // store the result by writing it into the register file.
        s.PRR.set(ops.OPS.ALUc.Pd, result);
        // store the negation as well
        s.PRR.set( (PRR_e) (NUM_PRR + ops.OPS.ALUc.Pd), !result);
      }
    }

    // MW inherited from NOP
    
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPPReg(os, "out: ", ops.OPS.ALUc.Pd, s);
      printGPReg(os, ", in: ", ops.OPS.ALUc.Rs1, ops.DR_Rs1, s);
      printGPReg(os, ", "  , ops.OPS.ALUc.Rs2, ops.DR_Rs2, s);
    }    
    
    virtual void reset_stats() {
      cnt_cmp_r0 = 0;
      cnt_cmp_zero = 0;
      cnt_cmp_short_imm = 0;
      cnt_cmp_short_negimm = 0;
      cnt_cmp_short_uimm = 0;
    }
    
    virtual void collect_stats(const simulator_t &s, 
                               const instruction_data_t &ops) {
      if (ops.OPS.ALUc.Rs1 == r0 || 
          ops.OPS.ALUc.Rs2 == r0) {
        cnt_cmp_r0++;
      } else {
        word_t value = std::min(s.GPR.get(ops.OPS.ALUc.Rs1).get(),
                                s.GPR.get(ops.OPS.ALUc.Rs2).get());
        if (value == 0) {
          cnt_cmp_zero++;
        } else if (value < 0 && -value < (1<<5)) {
          cnt_cmp_short_negimm++;
        } else if (value > 0 && value < (1<<5)) {
          cnt_cmp_short_imm++;
        } else if (value > 0 && value < (1<<6)) {
          cnt_cmp_short_uimm++;
        }
      }
    }
    
    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const symbol_map_t &symbols) const {
      os << boost::format("r0: %d, zero: %d, negimm: %d, short imm: %d, uimm: %d") 
         % cnt_cmp_r0 % cnt_cmp_zero % cnt_cmp_short_negimm % cnt_cmp_short_imm % cnt_cmp_short_uimm;
    }
  };

#define ALUc_INSTR(name, operator) \
  class i_ ## name ## _t : public i_aluc_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% p%2% = r%3%, r%4%") % #name \
          % ops.OPS.ALUc.Pd % ops.OPS.ALUc.Rs1 % ops.OPS.ALUc.Rs2; \
    } \
    virtual bit_t compute(word_t value1, word_t value2) const \
    { \
      return value1 operator value2; \
    } \
  };

  ALUc_INSTR(cmpeq , ==)
  ALUc_INSTR(cmpneq, !=)
  ALUc_INSTR(cmplt , <)
  ALUc_INSTR(cmple , <=)

  #define ALUcu_INSTR(name, operator) \
  class i_ ## name ## _t : public i_aluc_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% p%2% = r%3%, r%4%") % #name \
          % ops.OPS.ALUc.Pd % ops.OPS.ALUc.Rs1 % ops.OPS.ALUc.Rs2; \
    } \
    virtual bit_t compute(word_t value1, word_t value2) const \
    { \
      return ((uword_t)value1) operator ((uword_t)value2); \
    } \
  };

  ALUcu_INSTR(cmpult, <)
  ALUcu_INSTR(cmpule, <=)

  class i_btest_t : public i_aluc_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << boost::format("btest p%1% = r%2%, r%3%")
          % ops.OPS.ALUc.Pd % ops.OPS.ALUc.Rs1 % ops.OPS.ALUc.Rs2;
    }

    virtual bit_t compute(word_t value1, word_t value2) const
    {
      return (((uword_t)value1) & (1 << ((uword_t)value2))) != 0;
    }
  };

  /// Base class for ALUp instructions.
  class i_alup_t : public i_pred_t
  {
  public:
    /// Compute the result of an ALUp instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual bit_t compute(word_t value1, word_t value2) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Ps1 = s.PRR.get(ops.OPS.ALUp.Ps1).get();
      ops.DR_Ps2 = s.PRR.get(ops.OPS.ALUp.Ps2).get();
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        // compute the result of the ALU instruction
        bit_t result = compute(ops.DR_Ps1, ops.DR_Ps2);

        // store the result by writing it into the register file.
        s.PRR.set(ops.OPS.ALUp.Pd, result);
        // store the negation as well
        s.PRR.set( (PRR_e) (NUM_PRR + ops.OPS.ALUp.Pd), !result);
      }
    }

    // MW inherited from NOP.
    
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPPReg(os, "out: ", ops.OPS.ALUp.Pd, s);
      printPPReg(os, ", in: ", ops.OPS.ALUp.Ps1, ops.DR_Ps1);
      printPPReg(os, ", "  , ops.OPS.ALUp.Ps2, ops.DR_Ps2);
    }
  };

#define ALUp_INSTR(name, lval, operator) \
  class i_ ## name ## _t : public i_alup_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% p%2% = ") % #name % ops.OPS.ALUp.Pd; \
      printPred(os, ops.OPS.ALUp.Ps1, true); \
      os << ", "; \
      printPred(os, ops.OPS.ALUp.Ps2, true); \
    } \
    virtual bit_t compute(word_t value1, word_t value2) const \
    { \
      return lval == ((value1 operator value2) & 0x1); \
    } \
  };

  ALUp_INSTR(por , 1, |)
  ALUp_INSTR(pand, 1, &)
  ALUp_INSTR(pxor, 1, ^)

  /// A multi-cycle NOP operation.
  class i_spcn_t : public i_pred_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << "nop " << ops.OPS.SPCn.Imm;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the IF pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void IF(simulator_t &s, instruction_data_t &ops) const
    {
      i_pred_t::IF(s, ops);
      ops.DR_Imm = 0;
      i_pred_t::IF(s, ops);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      bit_t Pred = ops.DR_Pred = s.PRR.get(ops.Pred).get();

      if (Pred)
      {
        if (ops.DR_Imm != ops.OPS.SPCn.Imm)
        {
          // increment NOP cycle counter
          ops.DR_Imm++;

          // stall the pipeline
          s.pipeline_stall(SDR);
        }
      }
    }

    // EX inherited from NOP

    // MW inherited from NOP
  };

  /// Wait for memory operations to complete.
  class i_spcw_t : public i_pred_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << "waitm";
    }

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      bit_t Pred = s.PRR.get(ops.Pred).get();

      if (Pred)
      {
        if (s.Is_decoupled_load_active)
        {
          // stall the pipeline
          s.pipeline_stall(SDR);
        }
      }
    }

    // EX inherited from NOP

    // MW inherited from NOP
  };

  /// Move a value from a general purpose register to a special purpose
  /// register.
  class i_spct_t : public i_pred_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << boost::format("mts s%1% = r%2%")
                          % ops.OPS.SPCt.Sd % ops.OPS.SPCt.Rs1;
    }

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.SPCt.Rs1);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        uword_t result = read_GPR_EX(s, ops.DR_Rs1);

        // store the result by writing it into the special purpose register file
        // or resetting the predicate registers
        if (ops.OPS.SPCt.Sd == 0)
        {
          // p0 is always 1, so skip it
          for(unsigned int i = 1; i < NUM_PRR; i++) {
            s.PRR.set ((PRR_e)i, ((result >> i) & 1) == 1);
            s.PRR.set ((PRR_e)(NUM_PRR+i), ((result >> i) & 1) == 0);
          }
        }
        else
          s.SPR.set(ops.OPS.SPCt.Sd, result);
      }
    }

    // MW inherited from NOP

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.

    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "in: ", ops.OPS.SPCt.Rs1, ops.DR_Rs1, s);
    }
  };

  /// Move a value from a special purpose register to a general purpose
  /// register.
  class i_spcf_t : public i_pred_t
  {
    uint64_t cnt_accesses[8];
  public:
    i_spcf_t() { reset_stats(); }
    
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << boost::format("mfs r%1% = s%2%")
                          % ops.OPS.SPCf.Rd % ops.OPS.SPCf.Ss;
    }

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      // read a value from the special register file or all predicate registers
      if (ops.OPS.SPCf.Ss == 0)
      {
        ops.DR_Ss = 0;
        for(unsigned int i = 0; i < NUM_PRR; i++)
          ops.DR_Ss |= (s.PRR.get((PRR_e)i).get() << i);
      }
      else
        ops.DR_Ss = s.SPR.get(ops.OPS.SPCf.Ss).get();
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      // read the special purpose register, without forwarding
      ops.EX_result = ops.DR_Ss;

      store_GPR_EX_result(s, ops, ops.OPS.SPCf.Rd, ops.EX_result);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      store_GPR_MW_result(s, ops, ops.OPS.SPCf.Rd, ops.EX_result);
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.

    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "out: ", ops.OPS.SPCf.Rd, ops.EX_result);
    }    
    
    virtual void reset_stats() {
      for (int i = 0; i < 8; i++) {
        cnt_accesses[i] = 0;
      }
    }

    virtual void collect_stats(const simulator_t &s, 
                               const instruction_data_t &ops) {
      if (ops.OPS.SPCf.Ss < 8) {
        cnt_accesses[ops.OPS.SPCf.Ss]++;
      }
    }
    
    virtual void print_stats(const simulator_t &s, std::ostream &os,
                             const symbol_map_t &symbols) const {
      for (int i = 0; i < 8; i++) {
        if (i) os << ", ";
        os << boost::format("s%d: %d") % i % cnt_accesses[i];
      }
    }
    
  };

  /// Base class for memory load instructions.
  class i_ldt_t : public i_pred_t
  {
  public:
    /// load the value from memory.
    /// @param s The Patmos simulator executing the instruction.
    /// @param address The address of the memory access.
    /// @param value If the function returns true, the loaded value is returned
    /// here.
    /// @return True if the value was loaded, false if the value is not yet
    /// available and stalling is needed.
    virtual bool load(simulator_t &s, word_t address, word_t &value) const = 0;

    /// peek into the memory to read the value without delay.
    /// @param s The Patmos simulator executing the instruction.
    /// @param address The address of the memory access.
    /// @return The read value.
    virtual word_t peek(simulator_t &s, word_t address) const = 0;
    
    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.LDT.Ra);
    }

    // EX implemented by sub classes

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        // load from memory
        word_t result;
        bool is_available = load(s, ops.EX_Address, result);

        // the value is already available?
        if (is_available)
        {
          store_GPR_MW_result(s, ops, ops.OPS.LDT.Rd, result);
        }
        else
        {
          // stall and wait for the memory/cache
          s.pipeline_stall(SMW);
        }
      }
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      // This might return an old value if there is a store in MW in progress,
      // but this is much better than nothing.
      // TODO this is nasty, that we have to cast const away, peek should be const.
      word_t result = peek(const_cast<simulator_t &>(s), ops.EX_Address);
      
      printGPReg(os, "peek: ", ops.OPS.LDT.Rd, result);
      printGPReg(os, " in: ", ops.OPS.LDT.Ra, ops.DR_Rs1, s);
      os << boost::format(" addr: %1$08x ") % ops.EX_Address;
      symbols.print(os, ops.EX_Address);
    }    
  };

#define LD_INSTR(name, base, atype, ctype) \
  class i_ ## name ## _t : public i_ldt_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% r%2% = [r%3% + %4%]") % #name \
          % ops.OPS.LDT.Rd % ops.OPS.LDT.Ra % ops.OPS.LDT.Imm; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = read_GPR_EX(s, ops.DR_Rs1) + ops.OPS.LDT.Imm*sizeof(atype); \
    } \
    virtual bool load(simulator_t &s, word_t address, word_t &value) const \
    { \
      atype tmp=0; \
      if ((address & (sizeof(atype) - 1)) != 0) \
        simulation_exception_t::unaligned(address); \
      bool is_available = base.read_fixed(address, tmp); \
      value = (ctype)from_big_endian<big_ ## atype>(tmp); \
      return is_available; \
    } \
    virtual word_t peek(simulator_t &s, word_t address) const \
    { \
      atype tmp=0; \
      base.peek_fixed(address, tmp); \
      return (ctype)from_big_endian<big_ ## atype>(tmp); \
    } \
  };

  LD_INSTR(lws , s.Stack_cache, word_t, word_t)
  LD_INSTR(lhs , s.Stack_cache, hword_t, word_t)
  LD_INSTR(lbs , s.Stack_cache, byte_t, word_t)
  LD_INSTR(lwus, s.Stack_cache, uword_t, uword_t)
  LD_INSTR(lhus, s.Stack_cache, uhword_t, uword_t)
  LD_INSTR(lbus, s.Stack_cache, ubyte_t, uword_t)

  LD_INSTR(lwl , s.Local_memory, word_t, word_t)
  LD_INSTR(lhl , s.Local_memory, hword_t, word_t)
  LD_INSTR(lbl , s.Local_memory, byte_t, word_t)
  LD_INSTR(lwul, s.Local_memory, uword_t, uword_t)
  LD_INSTR(lhul, s.Local_memory, uhword_t, uword_t)
  LD_INSTR(lbul, s.Local_memory, ubyte_t, uword_t)

  LD_INSTR(lwc , s.Data_cache, word_t, word_t)
  LD_INSTR(lhc , s.Data_cache, hword_t, word_t)
  LD_INSTR(lbc , s.Data_cache, byte_t, word_t)
  LD_INSTR(lwuc, s.Data_cache, uword_t, uword_t)
  LD_INSTR(lhuc, s.Data_cache, uhword_t, uword_t)
  LD_INSTR(lbuc, s.Data_cache, ubyte_t, uword_t)

  LD_INSTR(lwm , s.Memory, word_t, word_t)
  LD_INSTR(lhm , s.Memory, hword_t, word_t)
  LD_INSTR(lbm , s.Memory, byte_t, word_t)
  LD_INSTR(lwum, s.Memory, uword_t, uword_t)
  LD_INSTR(lhum, s.Memory, uhword_t, uword_t)
  LD_INSTR(lbum, s.Memory, ubyte_t, uword_t)


  /// Base class for memory load instructions.
  class i_dldt_t : public i_pred_t
  {
  public:
    /// load the value from memory.
    /// @param s The Patmos simulator executing the instruction.
    /// @param address The address of the memory access.
    /// @param value If the function returns true, the loaded value is returned
    /// here.
    /// @return True if the value was loaded, false if the value is not yet
    /// available and stalling is needed.
    virtual bool load(simulator_t &s, word_t address, word_t &value) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.LDT.Ra);

      if (ops.DR_Pred && s.Is_decoupled_load_active)
      {
        // stall the pipeline
        s.pipeline_stall(SDR);
      }
    }

    // EX implemented by base class

    // MW inherited from NOP

    /// Pipeline function to simulate the behavior of a decoupled load
    /// instruction running in parallel to the pipeline.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void dMW(simulator_t &s, instruction_data_t &ops) const
    {
      assert(s.Is_decoupled_load_active);

      // load from memory
      word_t result;
      bool is_available = load(s, ops.EX_Address, result);

      // the value is already available?
      if (is_available)
      {
        // store the loaded value by writing it into a by-pass
        s.SPR.set(sm, result);
        s.Decoupled_load = instruction_data_t();
        s.Is_decoupled_load_active = false;
      }
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "in: " , ops.OPS.LDT.Ra, ops.DR_Rs1, s);
      os << boost::format(" addr: %1$08x ") % ops.EX_Address;
      symbols.print(os, ops.EX_Address);
    }    
  };

#define DLD_INSTR(name, base, atype, ctype) \
  class i_ ## name ## _t : public i_dldt_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% sm = [r%2% + %3%]") % #name \
          % ops.OPS.LDT.Ra % ops.OPS.LDT.Imm; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = read_GPR_EX(s, ops.DR_Rs1) + ops.OPS.LDT.Imm*sizeof(atype); \
      if (ops.DR_Pred) \
      { \
        assert(!s.Is_decoupled_load_active); \
        s.Decoupled_load = ops; \
        s.Is_decoupled_load_active = true; \
      } \
    } \
    virtual bool load(simulator_t &s, word_t address, word_t &value) const \
    { \
      atype tmp; \
      if ((address & (sizeof(atype) - 1)) != 0) \
        simulation_exception_t::unaligned(address); \
      bool is_available = base.read_fixed(address, tmp); \
      value = (ctype)from_big_endian<big_ ## atype>(tmp); \
      return is_available; \
    } \
  };

  DLD_INSTR(dlwc , s.Data_cache, word_t, word_t)
  DLD_INSTR(dlhc , s.Data_cache, hword_t, word_t)
  DLD_INSTR(dlbc , s.Data_cache, byte_t, word_t)
  DLD_INSTR(dlwuc, s.Data_cache, uword_t, uword_t)
  DLD_INSTR(dlhuc, s.Data_cache, uhword_t, uword_t)
  DLD_INSTR(dlbuc, s.Data_cache, ubyte_t, uword_t)

  DLD_INSTR(dlwm , s.Memory, word_t, word_t)
  DLD_INSTR(dlhm , s.Memory, hword_t, word_t)
  DLD_INSTR(dlbm , s.Memory, byte_t, word_t)
  DLD_INSTR(dlwum, s.Memory, uword_t, uword_t)
  DLD_INSTR(dlhum, s.Memory, uhword_t, uword_t)
  DLD_INSTR(dlbum, s.Memory, ubyte_t, uword_t)

  /// Base class for memory store instructions.
  class i_stt_t : public i_pred_t
  {
  public:
    /// Store the value to memory.
    /// @param s The Patmos simulator executing the instruction.
    /// @param address The address of the memory access.
    /// @param value The value to be stored.
    /// @return True when the value was finally written to the memory, false
    /// if the instruction has to stall.
    virtual bool store(simulator_t &s, word_t address, word_t value) const = 0;

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.STT.Ra);
      ops.DR_Rs2 = s.GPR.get(ops.OPS.STT.Rs1);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.DR_Pred)
      {
        // store to memory
        if (!store(s, ops.EX_Address, ops.EX_Rs))
        {
          // we need to stall in order to ensure that the value was actually
          // propagated down to the memory
          s.pipeline_stall(SMW);
        }
      }
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "in: " , ops.OPS.STT.Ra, ops.DR_Rs1, s);
      printGPReg(os, ", "   , ops.OPS.STT.Rs1, ops.EX_Rs);
      os << boost::format(" addr: %1$08x ") % ops.EX_Address;
      symbols.print(os, ops.EX_Address);
    }    
  };

#define ST_INSTR(name, base, type) \
  class i_ ## name ## _t : public i_stt_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << boost::format("%1% [r%2% + %3%] = r%4%") % #name \
          % ops.OPS.STT.Ra % ops.OPS.STT.Imm2 % ops.OPS.STT.Rs1; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = read_GPR_EX(s, ops.DR_Rs1) + ops.OPS.STT.Imm2*sizeof(type); \
      ops.EX_Rs = read_GPR_EX(s, ops.DR_Rs2); \
    } \
    virtual bool store(simulator_t &s, word_t address, word_t value) const \
    { \
      type big_value = to_big_endian<big_ ## type>((type)value); \
      if ((address & (sizeof(type) - 1)) != 0) \
        simulation_exception_t::unaligned(address); \
      return base.write_fixed(address, big_value); \
    } \
  };

  ST_INSTR(sws, s.Stack_cache, word_t)
  ST_INSTR(shs, s.Stack_cache, hword_t)
  ST_INSTR(sbs, s.Stack_cache, byte_t)

  ST_INSTR(swl, s.Local_memory, word_t)
  ST_INSTR(shl, s.Local_memory, hword_t)
  ST_INSTR(sbl, s.Local_memory, byte_t)

  ST_INSTR(swc, s.Data_cache, word_t)
  ST_INSTR(shc, s.Data_cache, hword_t)
  ST_INSTR(sbc, s.Data_cache, byte_t)

  ST_INSTR(swm, s.Memory, word_t)
  ST_INSTR(shm, s.Memory, hword_t)
  ST_INSTR(sbm, s.Memory, byte_t)


#define STCi_INSTR(name, function) \
  class i_ ## name ## _t : public i_pred_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " " << ops.OPS.STCi.Imm; \
    } \
    virtual void DR(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.DR_Pred = s.PRR.get(ops.Pred).get(); \
      ops.DR_Ss = s.SPR.get(ss).get(); \
      ops.DR_St = s.SPR.get(st).get(); \
    } \
    virtual void MW(simulator_t &s, instruction_data_t &ops) const \
    { \
      uword_t stack_spill = ops.DR_Ss; \
      uword_t stack_top = ops.DR_St; \
      if(ops.DR_Pred && !s.Stack_cache.function(ops.OPS.STCi.Imm * \
                                                sizeof(word_t), \
                                                stack_spill, stack_top)) \
      { \
        s.pipeline_stall(SMW); \
      } \
      s.SPR.set(ss, stack_spill); \
      s.SPR.set(st, stack_top); \
    } \
  };

#define STCr_INSTR(name, function) \
  class i_ ## name ## _t : public i_pred_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " r" << ops.OPS.STCr.Rs; \
    } \
    virtual void DR(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.DR_Pred = s.PRR.get(ops.Pred).get(); \
      ops.DR_Ss = s.SPR.get(ss).get(); \
      ops.DR_St = s.SPR.get(st).get(); \
      ops.DR_Rs1 = s.GPR.get(ops.OPS.STCr.Rs); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Rs = read_GPR_EX(s, ops.DR_Rs1); \
    } \
    virtual void MW(simulator_t &s, instruction_data_t &ops) const \
    { \
      uword_t stack_spill = ops.DR_Ss; \
      uword_t stack_top = ops.DR_St; \
      if(ops.DR_Pred && !s.Stack_cache.function(ops.EX_Rs, \
                                                stack_spill, stack_top)) \
      { \
        s.pipeline_stall(SMW); \
      } \
      s.SPR.set(ss, stack_spill); \
      s.SPR.set(st, stack_top); \
    } \
    virtual void print_operands(const simulator_t &s, std::ostream &os, \
                       const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printGPReg(os, "in: " , ops.OPS.STCr.Rs, ops.EX_Rs); \
    } \
  };

  STCi_INSTR(sres, reserve)
  STCi_INSTR(sens, ensure)
  STCi_INSTR(sfree, free)
  STCi_INSTR(sspill, spill)

  STCr_INSTR(sensr, ensure)
  STCr_INSTR(sspillr, spill)

  /// Base class for branch, call, and return instructions.
  class i_cfl_t : public i_pred_t
  {
  protected:
    /// Store the method base address and offset to the respective special
    /// purpose registers.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param base The base address of the current method.
    /// @param pc The current program counter.
    void no_store_return_address(simulator_t &s, instruction_data_t &ops,
                                 bit_t pred, uword_t base, uword_t pc, word_t address) const
    {
      assert(base <= pc);
    }

    /// Store the method base address and offset to the respective special
    /// purpose registers.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param base The base address of the current method.
    /// @param pc The current program counter.
    void store_return_address(simulator_t &s, instruction_data_t &ops,
                              bit_t pred, uword_t base, uword_t pc, word_t address) const
    {
      if (pred && !ops.MW_CFL_Discard)
      {
        assert(base <= pc);
        assert(pc == ops.IF_PC + 16 && "Wrong delay slot size of call instruction.");

        s.Dbg_stack.push(address);
        s.Profiling.enter(address, s.Cycle);

        // store the return function offset (return PC) into
        // a general purpose register
        s.GPR.set(rfo, pc - base);
        // function base is compiler/programmer managed,
        // i.e., not stored implicitly
      }
    }

    /// Perform a function branch/call/return.
    /// Fetch the function into the method cache, stall the pipeline, and set
    /// the program counter.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param base The base address of the target method.
    /// @param address The target address.
    void fetch_and_dispatch(simulator_t &s, instruction_data_t &ops,
                            bit_t pred, word_t base, word_t address) const
    {
      if (pred && !ops.MW_CFL_Discard)
      {
        // check if the target method is in the cache, otherwise stall until
        // it is loaded.
        if (!s.Method_cache.is_available(base))
        {
          // stall the pipeline
          s.pipeline_stall(SMW);
        }
        else
        {
          // set the program counter and base
          s.BASE = base;
          s.nPC = address;
          ops.MW_CFL_Discard = 1;
        }
      }
    }

    /// Perform a function branch or call.
    /// The function is assumed to be in the method cache, thus simply set the
    /// program counter.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param base The base address of the target method.
    /// @param address The target address.
    void dispatch(simulator_t &s, instruction_data_t &ops, bit_t pred,
                  word_t base, word_t address) const
    {
      if (pred && !ops.MW_CFL_Discard)
      {
        // assure that the target method is in the cache.
        assert(s.Method_cache.assert_availability(base));

        // set the program counter and base
        s.BASE = base;
        s.nPC = address;
        ops.MW_CFL_Discard = 1;
      }
    }
  public:
    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.MW_CFL_Discard = 0;
    }

    // EX, MW implemented by sub-classes

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printSymbol(os, "PC", ops.EX_Address, symbols);
    }

    /// Returns true because these are all flow control instruction
    virtual bool is_flow_control() const
    {
      return true;
    }
  };

#define CFLB_EX_INSTR(name, store, dispatch, new_base, target)	\
  class i_ ## name ## _t : public i_cfl_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " " << ops.OPS.CFLb.Imm; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = target; \
      store(s, ops, ops.DR_Pred, s.BASE, s.nPC, ops.EX_Address); \
      dispatch(s, ops, ops.DR_Pred, new_base, ops.EX_Address); \
    } \
  };
#define CFLB_MW_INSTR(name, store, dispatch, new_base, target)      \
  class i_ ## name ## _t : public i_cfl_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                        const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " " << ops.OPS.CFLb.Imm; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = target; \
    } \
    virtual void MW(simulator_t &s, instruction_data_t &ops) const \
    { \
      store(s, ops, ops.DR_Pred, s.BASE, s.nPC, ops.EX_Address); \
      dispatch(s, ops, ops.DR_Pred, new_base, ops.EX_Address); \
    } \
  };

  CFLB_MW_INSTR(call, store_return_address, fetch_and_dispatch,
                ops.OPS.CFLb.UImm*sizeof(word_t),
                ops.OPS.CFLb.UImm*sizeof(word_t))
  CFLB_EX_INSTR(br, no_store_return_address, dispatch,
                s.BASE,
                ops.IF_PC + ops.OPS.CFLb.Imm*sizeof(word_t))
  CFLB_MW_INSTR(brcf, no_store_return_address, fetch_and_dispatch,
                ops.IF_PC + ops.OPS.CFLb.Imm*sizeof(word_t),
                ops.IF_PC + ops.OPS.CFLb.Imm*sizeof(word_t))

  class i_intr_t : public i_cfl_t 
  { 
  protected:
    /// Perform a function branch/call/return.
    /// Fetch the function into the method cache, stall the pipeline, and set
    /// the program counter.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param base The base address of the target method.
    /// @param address The target address.
    void fetch_and_dispatch(simulator_t &s, instruction_data_t &ops,
                            bit_t pred, word_t base, word_t address) const
    {
      if (pred && !ops.MW_CFL_Discard)
      {
        // set the program counter and base
        s.BASE = base;
        s.nPC = address;
        s.PC = address;
        ops.MW_CFL_Discard = 1;
      }
    }

  public:
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << "intr" << " " << ops.OPS.CFLb.Imm;
      symbols.print(os, ops.EX_Address);
    }

    virtual void EX(simulator_t &s, instruction_data_t &ops) const 
    {
      ops.EX_Address = ops.OPS.CFLb.Imm*sizeof(word_t);
      fetch_and_dispatch(s, ops, ops.DR_Pred, ops.EX_Address, ops.EX_Address); 
    }
  };

  /// Branch and call instructions with a register operand.
  class i_cfli_t : public i_cfl_t
  {
  public:
    // IF inherited from i_cfl_t

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Rs1 = s.GPR.get(ops.OPS.CFLi.Rs);
      ops.MW_CFL_Discard = 0;
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "in: ", ops.OPS.CFLi.Rs, ops.EX_Address);
      os << " ";
      symbols.print(os, ops.EX_Address);
    }
  };

#define CFLI_EX_INSTR(name, store, dispatch, new_base)	\
  class i_ ## name ## _t : public i_cfli_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " r" << ops.OPS.CFLi.Rs; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = read_GPR_EX(s, ops.DR_Rs1); \
      store(s, ops, ops.DR_Pred, s.BASE, s.nPC, ops.EX_Address); \
      dispatch(s, ops, ops.DR_Pred, new_base, ops.EX_Address); \
    } \
  };

#define CFLI_MW_INSTR(name, store, dispatch, new_base)   \
  class i_ ## name ## _t : public i_cfli_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops, \
                       const symbol_map_t &symbols) const \
    { \
      printPred(os, ops.Pred); \
      os << #name << " r" << ops.OPS.CFLi.Rs; \
      symbols.print(os, ops.EX_Address); \
    } \
    virtual void EX(simulator_t &s, instruction_data_t &ops) const \
    { \
      ops.EX_Address = read_GPR_EX(s, ops.DR_Rs1); \
    } \
    virtual void MW(simulator_t &s, instruction_data_t &ops) const \
    { \
      store(s, ops, ops.DR_Pred, s.BASE, s.nPC, ops.EX_Address); \
      dispatch(s, ops, ops.DR_Pred, new_base, ops.EX_Address); \
    } \
  };


  CFLI_MW_INSTR(callr, store_return_address, fetch_and_dispatch,
                ops.EX_Address)
  CFLI_EX_INSTR(brr, no_store_return_address, dispatch,
                s.BASE)
  CFLI_MW_INSTR(brcfr, no_store_return_address, fetch_and_dispatch,
                ops.EX_Address)

  /// An instruction for returning from function calls.
  class i_ret_t : public i_cfl_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print(std::ostream &os, const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printPred(os, ops.Pred);
      os << boost::format("ret r%1%, r%2%")
                          % ops.OPS.CFLr.Rb % ops.OPS.CFLr.Ro;
    }

    // IF inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the DR pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void DR(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DR_Pred = s.PRR.get(ops.Pred).get();
      ops.DR_Base   = s.GPR.get(ops.OPS.CFLr.Rb);
      ops.DR_Offset = s.GPR.get(ops.OPS.CFLr.Ro);
      ops.MW_CFL_Discard = 0;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the EX pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void EX(simulator_t &s, instruction_data_t &ops) const
    {
      ops.EX_Base   = read_GPR_EX(s, ops.DR_Base);
      ops.EX_Offset = read_GPR_EX(s, ops.DR_Offset);
      ops.EX_Address = ops.EX_Base + ops.EX_Offset;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the MW pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void MW(simulator_t &s, instruction_data_t &ops) const
    {
      // returning to address 0? interpret this as a halt.
      if (ops.DR_Pred && ops.EX_Base == 0)
      {
        // TODO this is one really ugly hack to let the simulator finish the
        // other stages before we exit, so that the PC is updated and stuff.
        // This should move into some simulator.halt(retvalue) call.
        if (ops.EX_Offset == 0) {
          ops.EX_Offset = 1;
          s.pipeline_stall(SMW);
        } else {
          simulation_exception_t::halt(s.GPR.get(GPR_EXIT_CODE_INDEX).get());
        }
      }
      else if (ops.DR_Pred)
      {
	s.Dbg_stack.pop(ops.EX_Base, ops.EX_Offset);
        s.Profiling.leave(s.Cycle);
        fetch_and_dispatch(s, ops, ops.DR_Pred, ops.EX_Base, ops.EX_Address);
      }
    }

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    /// @param symbols A mapping of addresses to symbols.
    virtual void print_operands(const simulator_t &s, std::ostream &os, 
		       const instruction_data_t &ops,
                       const symbol_map_t &symbols) const
    {
      printGPReg(os, "in: ", ops.OPS.CFLr.Rb, ops.EX_Base);
      printGPReg(os, ", "  , ops.OPS.CFLr.Ro, ops.EX_Offset);
    }
  };
}

#endif // PATMOS_INSTRUCTIONS_H


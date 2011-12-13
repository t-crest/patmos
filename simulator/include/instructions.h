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

#include "instruction.h"
#include "memory.h"
#include "method-cache.h"
#include "stack-cache.h"

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
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << "nop";
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the fetch pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void fetch(simulator_t &s, instruction_data_t &ops) const
    {
      s.PC = s.PC + 4;
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the fetch pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void fetch_commit(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the decode pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode_commit(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the execute pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute_commit(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the memory pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory_commit(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
    }
  };

  /// Halt the simulation
  class i_halt_t : public i_nop_t
  {
  public:
    static const i_halt_t i_halt;

    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << "halt";
    }

    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      throw HALT;
    }
  };

  class i_imm_t : public i_nop_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << "imm";
    }
  };
  
  /// Abstract base class of predicated instructions, i.e., all instructions 
  /// that actually do something.
  class i_pred_t : public i_nop_t
  {
  protected:
    /// Read a GPR register at the execute stage.
    /// @param op The register operand.
    /// @return The register value, considering by-passing from the EX, MEM, and
    /// WB stages.
    static inline word_t read_GPR_EX(simulator_t &s, GPR_op_t op)
    {
      return s.Pipeline[EX][0].GPR_EX_Rd.get(
             s.Pipeline[EX][1].GPR_EX_Rd.get(
              s.Pipeline[MEM][0].GPR_MEM_Rd.get(
              s.Pipeline[MEM][1].GPR_MEM_Rd.get(
                s.Pipeline[WB][0].GPR_WB_Rd.get(
                s.Pipeline[WB][1].GPR_WB_Rd.get(
                  op)))))).get();
    }

    /// Read a PRR register at the execute stage.
    /// @param op The predicate operand.
    /// @return The predicate value, considering by-passing from the EX, MEM, and
    /// WB stages.
    static inline bit_t read_PRR_EX(simulator_t &s, PRR_op_t op)
    {
      return s.Pipeline[EX][0].PRR_EX_Pd.get(
             s.Pipeline[EX][1].PRR_EX_Pd.get(
              s.Pipeline[MEM][0].PRR_MEM_Pd.get(
              s.Pipeline[MEM][1].PRR_MEM_Pd.get(
                s.Pipeline[WB][0].PRR_WB_Pd.get(
                s.Pipeline[WB][1].PRR_WB_Pd.get(
                  op)))))).get();
    }

  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      assert(false);
    }
  };

  /// Base class for two-register-operand ALU instructions.
  class i_alu_rr_t : public i_pred_t
  {
  public:
    /// Compute the result of an ALU instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual word_t compute(word_t value1, word_t value2) const = 0;

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DE_Pred = s.PRR.get(ops.Pred);
      ops.DE_Rs1 = s.GPR.get(ops.OPS.RRR.Rs1);
      ops.DE_Rs2 = s.GPR.get(ops.OPS.RRR.Rs2);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // read predicate
      ops.EX_Pred = read_PRR_EX(s, ops.DE_Pred);

      if (ops.EX_Pred)
      {
        // compute the result of the ALU instruction
        word_t result = compute(read_GPR_EX(s, ops.DE_Rs1),
                                read_GPR_EX(s, ops.DE_Rs2));

        // store the result by writing it into a by-pass.
        ops.GPR_EX_Rd.set(ops.OPS.RRR.Rd, result);
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.GPR_MEM_Rd.set(ops.GPR_EX_Rd.get());
        ops.GPR_EX_Rd.reset();
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        s.GPR.set(ops.GPR_MEM_Rd.get());
        ops.GPR_WB_Rd.set(ops.GPR_MEM_Rd.get());
        ops.GPR_MEM_Rd.reset();
      }
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.GPR_WB_Rd.reset();
      }
    }
  };

#define ALU_RR_INSTR(name, operator) \
  class i_ ## name ## _t : public i_alu_rr_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% r%3% = r%4%, r%5%") % #name \
          % ops.Pred % ops.OPS.RRR.Rd % ops.OPS.RRR.Rs1 % ops.OPS.RRR.Rs2; \
    } \
    virtual word_t compute(word_t value1, word_t value2) const \
    { \
      return value1 operator value2; \
    } \
  };

  ALU_RR_INSTR(add, +)
  ALU_RR_INSTR(sub, -)
  ALU_RR_INSTR(or,|)
  ALU_RR_INSTR(and, &)
  ALU_RR_INSTR(xor, ^)
  ALU_RR_INSTR(mul, *)

  /// Base class for immediate-operand ALU instructions.
  class i_alu_ri_t : public i_pred_t
  {
  public:
    /// Compute the result of an ALU instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual word_t compute(word_t value1, word_t value2) const = 0;

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DE_Pred = s.PRR.get(ops.Pred);
      ops.DE_Rs1 = s.GPR.get(ops.OPS.RRI.Rs1);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // read predicate
      ops.EX_Pred = read_PRR_EX(s, ops.DE_Pred);

      if (ops.EX_Pred)
      {
        // compute the result of the ALU instruction
        word_t result = compute(read_GPR_EX(s, ops.DE_Rs1), ops.OPS.RRI.Imm2);

        // store the result by writing it into a by-pass
        ops.GPR_EX_Rd.set(ops.OPS.RRI.Rd, result);
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.GPR_MEM_Rd.set(ops.GPR_EX_Rd.get());
        ops.GPR_EX_Rd.reset();
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        s.GPR.set(ops.GPR_MEM_Rd.get());
        ops.GPR_WB_Rd.set(ops.GPR_MEM_Rd.get());
        ops.GPR_MEM_Rd.reset();
      }
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.GPR_WB_Rd.reset();
      }
    }
  };

#define ALU_RI_INSTR(name, operator) \
  class i_ ## name ## _t : public i_alu_ri_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% r%3% = r%4%, %5%") % #name \
          % ops.Pred % ops.OPS.RRI.Rd % ops.OPS.RRI.Rs1 % ops.OPS.RRI.Imm2; \
    } \
    virtual word_t compute(word_t value1, word_t value2) const \
    { \
      return value1 operator value2; \
    } \
  };

  ALU_RI_INSTR(addi, +)
  ALU_RI_INSTR(subi, -)
  ALU_RI_INSTR(ori,|)
  ALU_RI_INSTR(andi, &)
  ALU_RI_INSTR(xori, ^)


  /// Base class for comparison instructions.
  class i_cmp_t : public i_pred_t
  {
  public:
    /// Compute the result of a comparison instruction.
    /// @param value1 The value of the first operand.
    /// @param value2 The value of the second operand.
    virtual bit_t compute(word_t value1, word_t value2) const = 0;

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DE_Pred = s.PRR.get(ops.Pred);
      ops.DE_Rs1 = s.GPR.get(ops.OPS.CMP.Rs1);
      ops.DE_Rs2 = s.GPR.get(ops.OPS.CMP.Rs2);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // read predicate
      ops.EX_Pred = read_PRR_EX(s, ops.DE_Pred);

      if (ops.EX_Pred)
      {
        // compute the result of the comparison instruction
        bit_t result = compute(read_GPR_EX(s, ops.DE_Rs1),
                               read_GPR_EX(s, ops.DE_Rs2));

        // store the result by writing it into a by-pass.
        ops.PRR_EX_Pd.set(ops.OPS.CMP.Pd, result);
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.PRR_MEM_Pd.set(ops.PRR_EX_Pd.get());
        ops.PRR_EX_Pd.reset();
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        s.PRR.set(ops.PRR_MEM_Pd.get());
        ops.PRR_WB_Pd.set(ops.PRR_MEM_Pd.get());
        ops.PRR_MEM_Pd.reset();
      }
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.PRR_WB_Pd.reset();
      }
    }
  };
  
#define CMP_INSTR(name, operator) \
  class i_ ## name ## _t : public i_cmp_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% p%3% = r%4%, %5%") % #name \
          % ops.Pred % ops.OPS.CMP.Pd % ops.OPS.CMP.Rs1 % ops.OPS.CMP.Rs2; \
    } \
    virtual bit_t compute(word_t value1, word_t value2) const \
    { \
      return value1 operator value2; \
    } \
  };

  CMP_INSTR(eq, ==)
  CMP_INSTR(neq, !=)
  CMP_INSTR(gt, >)
  CMP_INSTR(lt, <)
  CMP_INSTR(leq, <=)
  CMP_INSTR(geq, >=)

  #define CMPU_INSTR(name, operator) \
  class i_ ## name ## _t : public i_cmp_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% p%3% = r%4%, %5%") % #name \
          % ops.Pred % ops.OPS.CMP.Pd % ops.OPS.CMP.Rs1 % ops.OPS.CMP.Rs2; \
    } \
    virtual bit_t compute(word_t value1, word_t value2) const \
    { \
      return ((uword_t)value1) operator ((uword_t)value2); \
    } \
  };

  CMPU_INSTR(gtu, >)
  CMPU_INSTR(ltu, <)
  CMPU_INSTR(lequ, <=)
  CMPU_INSTR(gequ, >=)

  /// A function-local branch instruction.
  class i_br_t : public i_pred_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << boost::format("br.p%1% %2%") % ops.Pred % ops.OPS.I.Imm;
    }

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      bit_t Pred = s.PRR.get(ops.Pred).get();

      if (Pred)
      {
        // flush the pipeline
        s.pipeline_flush(FE);
        s.PC = s.PC + ops.OPS.I.Imm;
      }
    }
  };  

  /// Base class for function call/return instructions
  class i_dispatch_t : public i_pred_t
  {
  protected:
    /// Perform a function call/return.
    /// Fetch the function into the method cache, stall and flush the pipeline,
    /// and set the program counter.
    /// @param s The Patmos simulator executing the instruction.
    /// @param pred The predicate under which the instruction is executed.
    /// @param method The base address of the target method.
    /// @param address The target address.
    void dispatch(simulator_t &s, bit_t pred, word_t method,
                  word_t address) const
    {
      if (pred)
      {
        // check if the target method is in the cache, otherwise stall until
        // it is loaded.
        if (!s.Method_cache.is_available(method))
        {
          // flush and stall the pipeline
          s.pipeline_flush(FE);
          s.pipeline_stall(DE);
        }
        else
        {
          // flush the pipeline
          s.pipeline_flush(FE);

          // set the program counter
          s.PC = address;
        }
      }
    }
  public:
  };
  
  /// An instructions for function calls.
  class i_jsr__t : public i_dispatch_t
  {
  public:
    // FETCH inherited from NOP

    // DECODE implemented below
    
    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // keep the potentially outdated predicate from the decode stage
      if (ops.DE_Pred.get())
      {
        // store the return address by writing it into a by-pass.
        ops.GPR_EX_Rd.set(rA, ops.DE_PC);
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      // keep the potentially outdated predicate from the decode stage
      if (ops.DE_Pred.get())
      {
        ops.GPR_MEM_Rd.set(ops.GPR_EX_Rd.get());
        ops.GPR_EX_Rd.reset();
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
      // keep the potentially outdated predicate from the decode stage
      if (ops.DE_Pred.get())
      {
        s.GPR.set(ops.GPR_MEM_Rd.get());
        ops.GPR_WB_Rd.set(ops.GPR_MEM_Rd.get());
        ops.GPR_MEM_Rd.reset();
      }
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      // keep the potentially outdated predicate from the decode stage
      if (ops.DE_Pred.get())
      {
        ops.GPR_WB_Rd.reset();
      }
    }
  };

  /// An instructions for absolute function calls.
  class i_jsri_t : public i_jsr__t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << boost::format("jsri.p%1% %2%") % ops.Pred % ops.OPS.I.Imm;
    }
    
    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      // get the predicate
      ops.DE_Pred = s.PRR.get(ops.Pred);
      
      // store the return address.
      ops.DE_PC = s.PC;

      // perform the dispatch
      dispatch(s, ops.DE_Pred.get(), ops.OPS.I.Imm, ops.OPS.I.Imm);
    }
  };
  
  /// An instruction for indirect function calls.
  class i_jsr_t : public i_jsr__t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << boost::format("jsr.p%1% r%2%") % ops.Pred % ops.OPS.JSR.Ra;
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      // get the predicate
      ops.DE_Pred = s.PRR.get(ops.Pred);

      // store the return address.
      ops.DE_PC = s.PC;

      // perform the dispatch
      dispatch(s, ops.DE_Pred.get(), s.GPR.get(ops.OPS.JSR.Ra).get(),
               s.GPR.get(ops.OPS.JSR.Ra).get());
    }
  };

  /// An instruction for returning from function calls.
  class i_ret_t : public i_dispatch_t
  {
  public:
    /// Print the instruction to an output stream.
    /// @param os The output stream to print to.
    /// @param ops The operands of the instruction.
    virtual void print(std::ostream &os, const instruction_data_t &ops) const
    {
      os << boost::format("ret.p%1%") % ops.Pred;
    }

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      // note that we use rM and rA here!
      dispatch(s, s.PRR.get(ops.Pred).get(), s.GPR.get(rM).get(),
               s.GPR.get(rA).get());
    }
  };  

  /// Base class for memory load instructions.
  class i_load_t : public i_pred_t
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

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DE_Pred = s.PRR.get(ops.Pred);
      ops.DE_Rs1 = s.GPR.get(ops.OPS.LD.Ra);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // read predicate
      ops.EX_Pred = read_PRR_EX(s, ops.DE_Pred);

      if (ops.EX_Pred)
      {
        // compute the address of the load instruction
        ops.EX_Address = read_GPR_EX(s, ops.DE_Rs1) + ops.OPS.LD.Imm;
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        // load from memory
        word_t result;
        bool is_available = load(s, ops.EX_Address, result);

        // the value is already available?
        if (is_available)
        {
          // store the loaded value by writing it into a by-pass
          ops.GPR_MEM_Rd.set(ops.OPS.LD.Rd, result);
        }
        else
        {
          // stall and wait for the memory/cache
          s.pipeline_stall(MEM);
        }
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the writeback pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        s.GPR.set(ops.GPR_MEM_Rd.get());
        ops.GPR_WB_Rd.set(ops.GPR_MEM_Rd.get());
        ops.GPR_MEM_Rd.reset();
      }
    }

    /// Commit function to commit the shadow state of the instruction in
    /// the writeback pipeline stage to the global state.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void writeback_commit(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        ops.GPR_WB_Rd.reset();
      }
    }
  };
  
#define LD_INSTR(name, base, atype, ctype) \
  class i_ ## name ## _t : public i_load_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% r%3% = [r%4% + %5%]") % #name \
          % ops.Pred % ops.OPS.LD.Rd % ops.OPS.LD.Ra % ops.OPS.LD.Imm; \
    } \
    virtual bool load(simulator_t &s, word_t address, word_t &value) const \
    { \
      atype tmp; \
      bool is_available = base.read_fixed(address * sizeof(atype), tmp); \
      value = (ctype)tmp; \
      return is_available; \
    } \
  };

  LD_INSTR(lws, s.Stack_cache, word_t, word_t)
  LD_INSTR(lhs, s.Stack_cache, hword_t, word_t)
  LD_INSTR(lbs, s.Stack_cache, byte_t, word_t)
  LD_INSTR(ulws, s.Stack_cache, uword_t, uword_t)
  LD_INSTR(ulhs, s.Stack_cache, uhword_t, uword_t)
  LD_INSTR(ulbs, s.Stack_cache, ubyte_t, uword_t)

//   TODO: implement local memories
//   LD_INSTR(lwl, s.Local_memory, word_t, word_t)
//   LD_INSTR(lhl, s.Local_memory, hword_t, word_t)
//   LD_INSTR(lbl, s.Local_memory, byte_t, word_t)
//   LD_INSTR(ulwl, s.Local_memory, uword_t, uword_t)
//   LD_INSTR(ulhl, s.Local_memory, uhword_t, uword_t)
//   LD_INSTR(ulbl, s.Local_memory, ubyte_t, uword_t)

  LD_INSTR(lwg, s.Memory, word_t, word_t)
  LD_INSTR(lhg, s.Memory, hword_t, word_t)
  LD_INSTR(lbg, s.Memory, byte_t, word_t)
  LD_INSTR(ulwg, s.Memory, uword_t, uword_t)
  LD_INSTR(ulhg, s.Memory, uhword_t, uword_t)
  LD_INSTR(ulbg, s.Memory, ubyte_t, uword_t)

  /// Base class for memory store instructions.
  class i_store_t : public i_pred_t
  {
  public:
    /// Store the value to memory.
    /// @param s The Patmos simulator executing the instruction.
    /// @param address The address of the memory access.
    /// @param value The value to be stored.
    /// @return True when the value was finally written to the memory, false
    /// if the instruction has to stall.
    virtual bool store(simulator_t &s, word_t address, word_t value) const = 0;

    // FETCH inherited from NOP

    /// Pipeline function to simulate the behavior of the instruction in
    /// the decode pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void decode(simulator_t &s, instruction_data_t &ops) const
    {
      ops.DE_Pred = s.PRR.get(ops.Pred);
      ops.DE_Rs1 = s.GPR.get(ops.OPS.ST.Ra);
      ops.DE_Rs2 = s.GPR.get(ops.OPS.ST.Rs);
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the execute pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void execute(simulator_t &s, instruction_data_t &ops) const
    {
      // read predicate
      ops.EX_Pred = read_PRR_EX(s, ops.DE_Pred);

      if (ops.EX_Pred)
      {
        // compute the address of the store instruction
        ops.EX_Address = read_GPR_EX(s, ops.DE_Rs1) + ops.OPS.ST.Imm;
        ops.EX_Rs = read_GPR_EX(s, ops.DE_Rs2);
      }
    }

    /// Pipeline function to simulate the behavior of the instruction in
    /// the memory pipeline stage.
    /// @param s The Patmos simulator executing the instruction.
    /// @param ops The operands of the instruction.
    virtual void memory(simulator_t &s, instruction_data_t &ops) const
    {
      if (ops.EX_Pred)
      {
        // store to memory
        if (!store(s, ops.EX_Address, ops.EX_Rs))
        {
          // we need to stall in order to ensure that the value was actually 
          // propagated down to the memory
          s.pipeline_stall(MEM);
        }
      }
    }
  };

#define ST_INSTR(name, base, type) \
  class i_ ## name ## _t : public i_store_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% [r%3% + %4%] = r%5%") % #name \
          % ops.Pred % ops.OPS.ST.Ra % ops.OPS.ST.Imm % ops.OPS.ST.Rs; \
    } \
    virtual bool store(simulator_t &s, word_t address, word_t value) const \
    { \
      return base.write_fixed(address * sizeof(type), value); \
    } \
  };

//   TODO: implement stores to stack-cache
//   ST_INSTR(sws, s.Stack_cache, word_t)
//   ST_INSTR(shs, s.Stack_cache, hword_t)
//   ST_INSTR(sbs, s.Stack_cache, byte_t)
//
//   TODO: implement local memories
//   ST_INSTR(swl, s.Local_memory, word_t)
//   ST_INSTR(shl, s.Local_memory, hword_t)
//   ST_INSTR(sbl, s.Local_memory, byte_t)

  ST_INSTR(swg, s.Memory, word_t)
  ST_INSTR(shg, s.Memory, hword_t)
  ST_INSTR(sbg, s.Memory, byte_t)

#define STACK_INSTR(name) \
  class i_ ## name ## _t : public i_pred_t \
  { \
  public:\
    virtual void print(std::ostream &os, const instruction_data_t &ops) const \
    { \
      os << boost::format("%1%.p%2% %3%") % #name % ops.Pred % ops.OPS.I.Imm; \
    } \
    virtual void memory(simulator_t &s, instruction_data_t &ops) const \
    { \
      if(ops.EX_Pred && !s.Stack_cache.name(ops.OPS.I.Imm)) \
      { \
        s.pipeline_stall(MEM); \
      } \
    } \
  };

  STACK_INSTR(reserve)
  STACK_INSTR(free)
  STACK_INSTR(ensure)
}

#endif // PATMOS_INSTRUCTIONS_H


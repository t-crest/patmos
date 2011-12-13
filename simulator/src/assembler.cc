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

#include "simulation-core.h"
#include "instruction.h"
#include "instructions.h"

#include <iostream>

namespace patmos
{
  /// Opcode symbols for instructions accepting two general purpose registers as
  /// operands
  enum OPCrr
  {
    add, sub, mul
  };

  /// Opcode symbols for instructions accepting a general purpose register and
  /// an immediate (we do not differentiate the short and long variants at this
  /// point)
  enum OPCri
  {
    addi, subi
  };

  /// Opcode symbols for compare instructions accepting two general purpose 
  /// registers.
  enum OPCc
  {
    ceq, cneq
  };

  /// Opcode symbols for instructions with one immediate operand.
  enum OPCi
  {
    br, jsri, reserve, free, ensure
  };

  /// Opcode symbols for instructions with one register operand.
  enum OPCr
  {
    jsr
  };

  /// Opcode symbols for instructions without any operands.
  enum OPCp
  {
    ret
  };

  /// Opcode symbols for load instructions with a register and an immediate 
  /// operand.
  enum OPCl
  {
    // load from stack cache
    lws, lhs, lbs, ulws, ulhs, ulbs,
    // load from local memory
    lwl, lhl, lbl, ulwl, ulhl, ulbl,
    // load from global memory
    lwg, lhg, lbg, ulwg, ulhg, ulbg
  };

  /// Opcode symbols for store instructions with a register and an immediate
  /// operand.
  enum OPCs
  {
    // store to stack cache
    sws, shs, sbs,
    // store to local memory
    swl, shl, sbl,
    // store to global memory
    swg, shg, sbg
  };

  /// Opcode symbols for unpredicated instructions without any operands.
  enum OPCn
  {
    nop, halt
  };

  /// Intermediate representation of two general purpose registers operands.
  struct rr_t
  {
    GPR_e a;
    GPR_e b;
  };

  /// Intermediate representation of a general purpose register operand and an
  /// immediate operand.
  struct ri_t
  {
    GPR_e a;
    word_t b;
  };


  /// Intermediate representation of two general purpose register operands and
  /// a destination register operand.
  struct rrr_t
  {
    GPR_e d;
    GPR_e a;
    GPR_e b;
  };

  /// Intermediate representation of a general purpose register operand, an
  /// immediate operand, and a destination register operand.
  struct rri_t
  {
    GPR_e d;
    GPR_e a;
    word_t b;
  };

  /// Intermediate representation of two general purpose register operand and an
  /// immediate operand for store instructions.
  struct rir_t
  {
    GPR_e s;
    GPR_e a;
    word_t b;
  };

  /// Intermediate representation of two general purpose register operands and a
  /// destination predicate operand.
  struct prr_t
  {
    PRR_e d;
    GPR_e a;
    GPR_e b;
  };


  /// Parse the arguments of instructions having two general purpose register
  /// operands.
  rr_t operator,(GPR_e a, GPR_e b)
  {
    rr_t t = {a:a, b:b};
    return t;
  }

  /// Parse the arguments of instructions having a general purpose register and
  /// an immediate operand.
  ri_t operator,(GPR_e a, word_t b)
  {
    ri_t t = {a:a, b:b};
    return t;
  }

  /// Parse the arguments of instructions having two general purpose register
  /// operands and a destination register.
  rrr_t operator == (GPR_e d, const rr_t &rr)
  {
    rrr_t t = {d:d, a:rr.a, b:rr.b};
    return t;
  }

  /// Parse the arguments of instructions having a general purpose register, an
  /// immediate operand, and a destination register.
  rri_t operator == (GPR_e d, const ri_t &ri)
  {
    rri_t t = {d:d, a:ri.a, b:ri.b};
    return t;
  }

  /// Parse the arguments of instructions having two general purpose registers 
  /// and an immediate operand (stores).
  rir_t operator == (const ri_t &ri, GPR_e s)
  {
    rir_t t = {s:s, a:ri.a, b:ri.b};
    return t;
  }

  /// Parse the arguments of comparison instructions having two general purpose
  /// register operands and a destination predicate.
  prr_t operator == (PRR_e d, const rr_t &rr)
  {
    prr_t t = {d:d, a:rr.a, b:rr.b};
    return t;
  }


  template<typename T>
  struct OPC_Pred
  {
    T opc;
    PRR_e pred;
  };

  template<typename T>
  OPC_Pred<T> operator,(T o, PRR_e p)
  {
    OPC_Pred<T> t = {o, p};
    return t;
  }
  
  program_t operator,(OPC_Pred<OPCrr> opc, rrr_t ops)
  {
    program_t p;

    switch (opc.opc)
    {
      case add:
        p.push_back(instruction_data_t(i_mk<i_add_t>(), opc.pred, ops.d, ops.a,
                                       ops.b));
        break;
      case sub:
        p.push_back(instruction_data_t(i_mk<i_sub_t>(), opc.pred, ops.d, ops.a,
                                       ops.b));
        break;
      case mul:
        p.push_back(instruction_data_t(i_mk<i_mul_t>(), opc.pred, ops.d, ops.a,
                                       ops.b));
        break;
      default:
        assert(false);
    }

    return p;
  }

  program_t operator,(OPC_Pred<OPCri> opc, rri_t ops)
  {
    program_t p;

    bool is_long = (ops.b & 0xffffff000) != 0;
    switch (opc.opc)
    {
      case addi:
        p.push_back(instruction_data_t(i_mk<i_addi_t>(), opc.pred, ops.d, ops.a,
                                       ops.b, !is_long));
        break;
      case subi:
        p.push_back(instruction_data_t(i_mk<i_subi_t>(), opc.pred, ops.d, ops.a,
                                       ops.b, !is_long));
        break;
      default:
        assert(false);
    }

    // emit the instruction(s) to a program
    if (is_long)
    {
      p.push_back(instruction_data_t(i_mk<i_imm_t>()));
    }

    return p;
  }

  program_t operator,(OPC_Pred<OPCc> opc, prr_t ops)
  {
    program_t p;
    
    switch (opc.opc)
    {
      case ceq:
        p.push_back(instruction_data_t(i_mk<i_eq_t>(), opc.pred, ops.d, ops.a,
                                       ops.b));
        break;
      case cneq:
        p.push_back(instruction_data_t(i_mk<i_neq_t>(), opc.pred, ops.d, ops.a,
                                       ops.b));
        break;
      default:
        assert(false);
    }
    
    return p;
  }

  program_t operator,(OPC_Pred<OPCi> opc, word_t imm)
  {
    program_t p;

    switch (opc.opc)
    {
      case br:
        p.push_back(instruction_data_t(i_mk<i_br_t>(), opc.pred, imm));
        break;
      case jsri:
        p.push_back(instruction_data_t(i_mk<i_jsri_t>(), opc.pred, imm));
        break;
      case reserve:
        p.push_back(instruction_data_t(i_mk<i_reserve_t>(), opc.pred, imm));
        break;
      case free:
        p.push_back(instruction_data_t(i_mk<i_free_t>(), opc.pred, imm));
        break;
      case ensure:
        p.push_back(instruction_data_t(i_mk<i_ensure_t>(), opc.pred, imm));
        break;
      default:
        assert(false);
    }

    return p;
  }

  program_t operator,(OPC_Pred<OPCr> opc, GPR_e ra)
  {
    program_t p;

    switch (opc.opc)
    {
      case jsr:
        p.push_back(instruction_data_t(i_mk<i_jsr_t>(), opc.pred, ra));
        break;
      default:
        assert(false);
    }

    return p;
  }
  
  program_t operator,(OPCp opc, PRR_e pred)
  {
    program_t p;

    switch (opc)
    {
      case ret:
        p.push_back(instruction_data_t(i_mk<i_ret_t>(), pred));
        break;
      default:
        assert(false);
    }

    return p;
  }

  program_t operator,(OPC_Pred<OPCl> opc, rri_t ops)
  {
#define MATCH_LD(name) \
      case name: \
        p.push_back(instruction_data_t(i_mk<i_## name ##_t>(), opc.pred, \
                                       ops.d, ops.a, ops.b)); \
        break;
        
    program_t p;

    switch (opc.opc)
    {
      MATCH_LD(lws)
      MATCH_LD(lhs)
      MATCH_LD(lbs)
      MATCH_LD(ulws)
      MATCH_LD(ulhs)
      MATCH_LD(ulbs)
/*      MATCH_LD(lwl)
      MATCH_LD(lhl)
      MATCH_LD(lbl)
      MATCH_LD(ulwl)
      MATCH_LD(ulhl)
      MATCH_LD(ulbl)*/
      MATCH_LD(lwg)
      MATCH_LD(lhg)
      MATCH_LD(lbg)
      MATCH_LD(ulwg)
      MATCH_LD(ulhg)
      MATCH_LD(ulbg)
      default:
        assert(false);
    }

    return p;
  }

  program_t operator,(OPC_Pred<OPCs> opc, rir_t ops)
  {
#define MATCH_ST(name) \
      case name: \
        p.push_back(instruction_data_t(i_mk<i_## name ##_t>(), opc.pred, \
                                       ops.a, ops.b, ops.s)); \
        break;

    program_t p;

    switch (opc.opc)
    {
/*      MATCH_ST(sws)
      MATCH_ST(shs)
      MATCH_ST(sbs)
      MATCH_ST(swl)
      MATCH_ST(shl)
      MATCH_ST(sbl) */
      MATCH_ST(swg)
      MATCH_ST(shg)
      MATCH_ST(sbg)
      default:
        assert(false);
    }

    return p;
  }

  program_t operator!(OPCn opc)
  {
    program_t p;

    switch (opc)
    {
      case nop:
        p.push_back(instruction_data_t(i_mk<i_nop_t>()));
        break;
      case halt:
        p.push_back(instruction_data_t(i_mk<i_halt_t>()));
        break;
      default:
        assert(false);
    }

    return p;
  }

  program_t operator|(program_t a, program_t b)
  {
    program_t p;

    assert(a.size() == 1 && b.size() == 1);

    p.push_back(a.front());
    p.push_back(b.front());

    p.front().Bundle_end = false;

    return p;
  }

  program_t operator&&(program_t a, program_t b)
  {
    program_t p(a);

    p.insert(p.end(), b.begin(), b.end());

    return p;
  }

  std::ostream &operator <<(std::ostream &o, const program_t &p)
  {
    unsigned int pc = 0;
    bool print_pc = true;
    for(program_t::const_iterator i(p.begin()), ie(p.end()); i != ie; i++)
    {
      if (print_pc)
      {
        o << boost::format("\t%1$08x: ") % pc;
        print_pc = false;
      }

      i->print(o);

      if (i->Bundle_end)
      {
        o << "\n";
        print_pc = true;
      }
      else
      {
        o << " | ";
      }
      
      pc += 4;
    }
    
    return o;
  }
  
  /// Simple test program.
  void test1(bool debug)
  {
    ideal_method_cache_t imc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded"
    program_t p = (addi,p0, r1 == (r1, 2000)) | (addi,p0, r2 == (r2, 1111)) &&
                  (add ,p0, r3 == (r1, r2  )) | (sub, p0, r0 == (r1, r2  )) &&
                  !halt;

    simulator_t s(p, imm, imc, isc);
    s.run(1000, debug);

    // check that it was executed correctly
    assert(s.GPR.get(r0).get() == 889 &&
           s.GPR.get(r1).get() == 2000 &&
           s.GPR.get(r2).get() == 1111 &&
           s.GPR.get(r3).get() == 3111);
  }

  void test2(bool debug)
  {
    ideal_method_cache_t imc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded"
    program_t p = (addi,p0, r1 == (r1, 20000)) &&
                  (addi,p0, r2 == (r2, 1111 )) &&
                  (add ,p0, r3 == (r1, r2))    |   (sub ,p0, r0 == (r1, r2)) &&
                  !halt;

    simulator_t s(p, imm, imc, isc);
    s.run(10);

    // check that it was executed correctly
    assert(s.GPR.get(r0).get() == 18889 &&
           s.GPR.get(r1).get() == 20000 &&
           s.GPR.get(r2).get() == 1111 &&
           s.GPR.get(r3).get() == 21111);
  }

  void test3(bool debug)
  {
    ideal_method_cache_t imc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded"
    program_t p = (cneq,p0, p0 == (r1, r2)) | (br,p0, -8) &&
                  !halt;

    simulator_t s(p, imm, imc, isc);
    s.run(10, debug);

    // check that it was executed correctly
    assert(s.PRR.get(p0).get() == false);
  }

  void test4(bool debug)
  {
    lru_method_cache_t<64, 32, 20> lmc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded"
    program_t p = (jsri,p0, 24)              | (addi,p0, r2 == (r1, 40)) &&
                  (addi,p0, r1 == (r1,  8)) &&
                  (addi,p0, r1 == (r1, 12)) &&
                  (addi,p0, r1 == (r1, 16)) &&
                  (addi,p0, r1 == (r1, 20)) &&
                  (addi,p0, r1 == (r1, 24)) &&
                  (jsr ,p0, r2)             &&
                  !halt                     && 
                  (addi,p0, r1 == (r1, 36)) &&
                  (addi,p0, r1 == (r1, 40)) &&
                  !nop                      &&
                  !nop                      &&
                  !nop                      &&
                  (ret,p0)                  &&
                  !nop                      &&
                  !nop                      &&
                  !nop                      &&
                  !nop;

    simulator_t s(p, imm, lmc, isc);
    s.run(200, debug);

    // check that it was executed correctly
    assert(s.GPR.get(r1).get() == 64);
  }

  void test5(bool debug)
  {
    word_t tmp = 64;
    ideal_method_cache_t imc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded"
    program_t p = (addi,p0, r1 == (r1, 40)) | (addi,p0, r2 == (r2, 37))&&
                  (lwg,p0, r2 == (r1,  8))  | (swg,p0,  (r2,  8) == r1) &&
                  !halt;                     

    simulator_t s(p, imm, imc, isc);
    imm.write_fixed(48*4, tmp);
    s.run(20, debug);

    // check that it was executed correctly
    imm.read_fixed(45*4, tmp);
    assert(s.GPR.get(r2).get() == 64 && tmp == 40);
  }

  void test6(bool debug)
  {
    lru_method_cache_t<64, 32, 20> lmc;
    ideal_stack_cache_t isc;
    ideal_memory_t imm(NUM_MEMORY_BYTES);

    // the program to be "loaded" -- factorial
    // call conventions: r0 ... result        r1 ... argument
    program_t p = (addi,p0, rS == (rS, 1000)) && // setup SP
                  (addi,p0, r1 == (r1, 8))    && // setup argument
                  (jsri,p0, 16)               && // call -- RM is correct here
                  !halt                       && // done
                  (subi,p0, rS == (rS, 12))   && // allocate frame
                  (swg,p0, (rS, 8) == rM)     && // store RM
                  (sub,p0, r0 == (r0, r0))    && // clear register
                  (addi,p0, rM == (r0, 16))   && // setup RM for call
                  (addi,p0, r0 == (r0, 1))    && // initialize result
                  (cneq,p0, p1 == (r1, r0))   && // need recursion?
                  (swg,p0, (rS, 4) == rA)     && // store RA
                  (swg,p0, (rS, 12) == r1)    && // store argument
                  (subi,p1, r1 == (r1, 1))    && // subtract only if we recurs
                  (jsri,p1, 16)               && // recursive call
                  (lwg,p0, rA == (rS, 4))     && // reload RA
                  (lwg,p0, rM == (rS, 8))     && // reload RA
                  (lwg,p0, r1 == (rS, 12))    && // reload argument
                  (addi,p0, rS == (rS, 12))   && // free frame
                  (mul,p0, r0 == (r0, r1))    && // compute return value
                  (ret,p0)                    && // return -- using RA and RM
                  !nop                        && // some nops after return
                  !nop                        && 
                  !nop                        && 
                  !nop                        && 
                  !nop;

    simulator_t s(p, imm, lmc, isc);
    s.run(2000000, debug);

    // check that it was executed correctly
    assert(s.GPR.get(r0).get() == 40320);
  }

  void test7(bool debug)
  {
    patmos::byte_t bsc_mem[40];
    ideal_memory_t imm(NUM_MEMORY_BYTES);
    lru_method_cache_t<64, 32, 20> lmc;
    block_stack_cache_t<4, 4, 10, 1, true> bsc(&bsc_mem[40]);

    // the program to be "loaded" -- factorial
    // call conventions: r0 ... result        r1 ... argument
    program_t p = (reserve,p0, 10) &&
                  (lbs,p0, r0 == (r2, 5)) &&
                  (reserve,p0, 10) &&
                  (lbs,p0, r1 == (r2, 3)) &&
                  (free,p0, 10) &&
                  (lbs,p0, r2 == (r2, 3)) &&
                  (ensure,p0, 10) &&
                  (lbs,p0, r1 == (r2, 3)) &&
                  !halt;

    simulator_t s(p, imm, lmc, bsc);
    s.run(2000000, debug);

    // check that it was executed correctly
    assert(s.GPR.get(r0).get() == 8);
  }
}


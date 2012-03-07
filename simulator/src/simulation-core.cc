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
// Core simulation loop of the Patmos Simulator.
//

#include "simulation-core.h"
#include "data-cache.h"
#include "instruction.h"
#include "memory.h"
#include "method-cache.h"
#include "stack-cache.h"

#include <iostream>

namespace patmos
{
  simulator_t::simulator_t(memory_t &memory, memory_t &local_memory,
                           data_cache_t &data_cache,
                           method_cache_t &method_cache,
                           stack_cache_t &stack_cache) :
      Cycle(0), Memory(memory), Local_memory(local_memory),
      Data_cache(data_cache), Method_cache(method_cache),
      Stack_cache(stack_cache), BASE(0), PC(0), nPC(0), Stall(SIF)
  {
    // initialize one predicate register to be true, otherwise no instruction
    // will ever execute
    PRR.set(p0, true);

    // initialize the pipeline
    for(unsigned int i = 0; i < NUM_STAGES; i++)
    {
      for(unsigned int j = 0; j < NUM_SLOTS; j++)
      {
        Pipeline[i][j] = instruction_data_t();
      }
    }
  }

  void simulator_t::pipeline_invoke(Pipeline_t pst,
                                   void (instruction_data_t::*f)(simulator_t &),
                                   bool debug)
  {
    if (debug)
    {
      std::cerr << boost::format("%1% : ") % pst;
    }

    // invoke simulation functions
    for(unsigned int i = 0; i < NUM_SLOTS; i++)
    {
      // debug output
      if (debug)
      {
        if (i != 0)
        {
          std::cerr << " || ";
        }
        Pipeline[pst][i].print(std::cerr);
        std::cerr.flush();
      }

      // simulate the respective pipeline stage of the instruction
      (Pipeline[pst][i].*f)(*this);
    }

    if (debug)
    {
      std::cerr << "\n";
    }
  }

  void simulator_t::pipeline_flush(Pipeline_t pst)
  {
    for(unsigned int i = SIF; i <= (unsigned int)pst; i++)
    {
      for(unsigned int j = 0; j < NUM_SLOTS; j++)
      {
        Pipeline[i][j] = instruction_data_t();
      }
    }
  }

  void simulator_t::pipeline_stall(Pipeline_t pst)
  {
    Stall = std::max(Stall, pst);
  }

  void simulator_t::run(bool debug, uint64_t max_cycles)
  {
    try
    {
      for(uint64_t cycle = 0; cycle < max_cycles; cycle++, Cycle++)
      {
        // simulate decoupled load
        Decoupled_load.dMW(*this);

        if (debug)
        {
          std::cerr << "dMW: ";
          Decoupled_load.print(std::cerr);
          std::cerr << "\n";
        }

        // invoke simulation functions
        pipeline_invoke(SMW, &instruction_data_t::MW, debug);
        pipeline_invoke(SEX, &instruction_data_t::EX, debug);
        pipeline_invoke(SDR, &instruction_data_t::DR, debug);
        pipeline_invoke(SIF, &instruction_data_t::IF, debug);

        // commit results
        pipeline_invoke(SMW, &instruction_data_t::MW_commit);
        pipeline_invoke(SEX, &instruction_data_t::EX_commit);
        pipeline_invoke(SDR, &instruction_data_t::DR_commit);
        pipeline_invoke(SIF, &instruction_data_t::IF_commit);

        // move pipeline stages
        for (int i = SEX; i >= Stall; i--)
        {
          for (unsigned int j = 0; j < NUM_SLOTS; j++)
          {
            Pipeline[i + 1][j] = Pipeline[i][j];
          }
        }

        // decode the next instruction, only if we are not stalling.
        if (Stall == SIF)
        {
          unsigned int iw_size;

          // fetch the instruction word from the memory -- NO SIMULATION HERE,
          // just simple memory transfer.
          word_t iw[2];
          Memory.read_peek(PC, reinterpret_cast<byte_t*>(&iw), sizeof(iw));

          // decode the instruction word.
          iw_size = Decoder.decode(iw,  Pipeline[0]);

          // provide next program counter value
          nPC = PC + iw_size*4;

          // unknown instruction
          if (iw_size == 0)
          {
            simulation_exception_t::illegal(iw[0]);
          }
        }
        else if (Stall != NUM_STAGES- 1)
        {
          for(unsigned int i = 0; i < NUM_SLOTS; i++)
          {
            Pipeline[Stall + 1][i] = instruction_data_t();
          }
        }

        // reset the stall counter.
        Stall = SIF;

        // advance the time for the method cache, stack cache, and memory
        Memory.tick();
        Method_cache.tick();
        Stack_cache.tick();

        if (debug)
        {
          print(std::cerr);
        }
      }
    }
    catch (simulation_exception_t e)
    {
      switch (e.get_kind())
      {
        case simulation_exception_t::ILLEGAL:
        case simulation_exception_t::UNMAPPED:
        case simulation_exception_t::STACKEXCEEDED:
          // pass on to caller
          throw e;
        case simulation_exception_t::HALT:
          // simply return
          return;
      }
    }
  }

  /// Print the internal state of the simulator to an output stream.
  /// @param os An output stream.
  void simulator_t::print(std::ostream &os) const
  {
    os << boost::format("\nBASE: %1$08x   PC : %2$08x   Cyc: %3$08d   PRR: ")
       % BASE % PC % Cycle;

    // print values of predicate registers
    for(int p = NUM_PRR - 1; p >= 0; p--)
    {
      os << PRR.get((PRR_e)p).get();
    }
    os << "\n ";

    // print values of general purpose registers
    for(unsigned int r = r0; r < NUM_GPR; r++)
    {
      os << boost::format("r%1$-2d: %2$08x") % r % GPR.get((GPR_e)r).get();

      if ((r & 0x7) == 7)
      {
        os << "\n ";
      }
      else
      {
        os << "   ";
      }
    }
    os << "\n ";

    // print values of special purpose registers
    for(unsigned int s = s0; s < NUM_SPR; s++)
    {
      os << boost::format("s%1$-2d: %2$08x") % s % SPR.get((SPR_e)s).get();

      if ((s & 0x7) == 7)
      {
        os << "\n ";
      }
      else
      {
        os << "   ";
      }
    }
    os << "\n";

    // print state of method cache
    os << "Method Cache:\n";
    Method_cache.print(os);

    // print state of data cache
    os << "Data Cache:\n";
    Data_cache.print(os);

    // print state of stack cache
    os << "Stack Cache:\n";
    Stack_cache.print(os);

    // print state of main memory
    os << "Memory:\n";
    Memory.print(os);

    os << "\n";
  }

  std::ostream &operator<<(std::ostream &os, Pipeline_t p)
  {
    const static char* names[NUM_STAGES] = {"IF", "DR", "EX", "MW"};
    assert(names[p] != NULL);

    os << names[p];

    return os;
  }
}

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
#include "instruction.h"
#include "instructions.h"

#include <iostream>

namespace patmos
{
  simulator_t::simulator_t(const program_t &program, memory_t &memory,
                           method_cache_t &method_cache,
                           stack_cache_t &stack_cache) :
      Program(program), Memory(memory), Method_cache(method_cache),
      Stack_cache(stack_cache), Decoder(*this), PC(0), Stall(FE)
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
      std::cerr << pst << ": ";
    }
    
    for(unsigned int i = 0; i < NUM_SLOTS; i++)
    {
      // debug output
      if (debug)
      {
        if (i != 0)
        {
          std::cerr << " | ";
        }
        Pipeline[pst][i].print(std::cerr);
        std::cerr.flush();
      }
      
      // simulate the espective pipeline stage of the instruction
      (Pipeline[pst][i].*f)(*this);
    }
    
    if (debug)
    {
      std::cerr << "\n";
    }
  }

  void simulator_t::pipeline_flush(Pipeline_t pst)
  {
    for(unsigned int i = FE; i <= pst; i++)
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
  
  void simulator_t::run(uint64_t max_cycles, bool debug)
  {
    try
    {
      for(uint64_t cycle = 0; cycle < max_cycles; cycle++)
      {
        // invoke simulation functions
        pipeline_invoke(WB, &instruction_data_t::writeback, debug);
        pipeline_invoke(MEM, &instruction_data_t::memory, debug);
        pipeline_invoke(EX, &instruction_data_t::execute, debug);
        pipeline_invoke(DE, &instruction_data_t::decode, debug);
        pipeline_invoke(FE, &instruction_data_t::fetch, debug);

        // commit results
        pipeline_invoke(WB, &instruction_data_t::writeback_commit);
        pipeline_invoke(MEM, &instruction_data_t::memory_commit);
        pipeline_invoke(EX, &instruction_data_t::execute_commit);
        pipeline_invoke(DE, &instruction_data_t::decode_commit);
        pipeline_invoke(FE, &instruction_data_t::fetch_commit);

        // move pipeline stages
        for (int i = MEM; i >= Stall; i--)
        {
          for (unsigned int j = 0; j < NUM_SLOTS; j++)
          {
            Pipeline[i + 1][j] = Pipeline[i][j];
          }
        }

        // decode the next instruction, only if we are not stalling.
        if (Stall == FE)
        {
          Decoder.decode(Pipeline[0]);
        }
        else if (Stall != NUM_STAGES- 1)
        {
          for(unsigned int i = 0; i < NUM_SLOTS; i++)
          {
            Pipeline[Stall + 1][i] = instruction_data_t();
          }
        }

        // reset the stall counter.
        Stall = FE;

        // advance the time for the method cache, stack cache, and memory
        Memory.tick();
        Method_cache.tick();
        Stack_cache.tick();

        if (debug)
        {
          std::cerr << boost::format("\nPC : %1$08x   Cyc: %2$08d   PRR: ") % PC
                    % cycle;

          for(int p = NUM_PRR - 1; p >= 0; p--)
          {
            std::cerr << PRR.get((PRR_e)p).get();
          }
          std::cerr << "\n";

          for(unsigned int r = r0; r < NUM_GPR; r++)
          {
            std::cerr << boost::format("r%1$-2d: %2$08x") % r
                      % GPR.get((GPR_e)r).get();

            if ((r & 0x7) == 7)
            {
              std::cerr << "\n";
            }
            else
            {
              std::cerr << "   ";
            }
          }
          std::cerr << "\n";

          // print state of method cache
          Method_cache.print(std::cerr);
          Stack_cache.print(std::cerr);
          Memory.print(std::cerr);

          std::cerr << "\n";
        }
      }
    }
    catch (simulation_exception_t e)
    {
      switch (e)
      {
        case HALT:
          // simply return
          return;
      }
    }
  }
}

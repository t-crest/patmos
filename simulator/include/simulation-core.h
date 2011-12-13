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

#ifndef PATMOS_SIMULATOR_CORE_H
#define PATMOS_SIMULATOR_CORE_H

#include "decoder.h"
#include "registers.h"
#include "instruction.h"

#include <limits>

namespace patmos
{
  // forward definitions
  class memory_t;
  class stack_cache_t;
  class method_cache_t;
  
  /// Define the maximum number of slots in a bundle.
  static const unsigned int NUM_SLOTS = 2;
  
  /// Define the maximum number of slots in a bundle.
  static const unsigned int NUM_MEMORY_BYTES = 0x4000000;

  /// Definition of Pipeline stages.
  enum Pipeline_t
  {
    /// Fetch the bundle from instruction memory.
    FE = 0,
    /// Decode the instruction's format and its types.
    DE,
    /// Execute arithmetic operations and calculate addresses for memory 
    /// accesses.
    EX,
    /// Perform memory accesses, if any.
    MEM,
    /// Write the result back to the register files.
    WB,

    /// Index of last pipeline stage -- used to instantiate arrays etc.
    NUM_STAGES
  };

  /// Signal exceptions during simulation
  enum simulation_exception_t
  {

    /// A halt instruction was encountered.
    HALT
  };
  
  /// Main class representing the simulation of a single Patmos core.
  class simulator_t
  {
  public:
    /// The program to be simulated.
    const program_t &Program;

    /// The main memory used during the simulation.
    memory_t &Memory;

    /// The method cache used during the simulation.
    method_cache_t &Method_cache;

    /// The stack cache used during the simulation.
    stack_cache_t &Stack_cache;

    /// The decoder of the simulator.
    decoder_t Decoder;

    /// The program counter register.
    uword_t PC;

    /// The general purpose registers.
    GPR_t GPR;
    
    /// The predicate registers.
    PRR_t PRR;

    /// Counter up to which pipeline stage the processor stalls.
    Pipeline_t Stall;

    /// Active instructions in the pipeline stage.
    instruction_data_t Pipeline[NUM_STAGES][NUM_SLOTS];

    
    /// Perform a step of the simulation for a given pipeline.
    /// @param pst The pipeline stage.
    /// @param f The simulation/commit function to invoke.
    /// @param debug Flag indicating whether debug output should be printed.
    void pipeline_invoke(Pipeline_t pst,
                         void (instruction_data_t::*f)(simulator_t &),
                         bool debug = false);

    /// Flush the pipeline up to and including the given pipeline stage.
    /// @param pst The pipeline stage up to which instructions should be 
    /// flushed.
    void pipeline_flush(Pipeline_t pst);

    /// Stall the pipeline up to *not* including the given pipeline stage.
    /// @param pst The pipeline stage up to which instructions should be 
    /// stalled.
    void pipeline_stall(Pipeline_t pst);
  public:
    /// Construct a new instance of a Patmos-core simulator
    /// The simulator only retains the references of the arguments passed in the
    /// constructor, i.e., it does not clone them, proclaim ownership, etc.
    /// @param program The program to run.
    /// @param memory The main memory to use during the simulation.
    /// @param method_cache The method cache to use during the simulation.
    /// @param stack_cache The stack cache to use during the simulation.
    simulator_t(const program_t &program, memory_t &memory,
                method_cache_t &method_cache, stack_cache_t &stack_cache);

    /// Run the simulator.
    /// @param max_cycles The maximum number of cycles to run the simulation.
    /// @param debug Flag indicating whether debug output should be printed.
    void run(uint64_t max_cycles = std::numeric_limits<uint64_t>::max(),
             bool debug = false);
  };
}

#endif // PATMOS_SIMULATOR_CORE_H


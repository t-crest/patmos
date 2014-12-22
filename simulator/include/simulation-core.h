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
// Core simulation loop of the Patmos Simulator.
//

#ifndef PATMOS_SIMULATOR_CORE_H
#define PATMOS_SIMULATOR_CORE_H

#include "command-line.h"
#include "dbgstack.h"
#include "decoder.h"
#include "instruction.h"
#include "exception.h"
#include "profiling.h"

#include <set>
#include <limits>
#include <iostream>

namespace patmos
{
  // forward definitions
  class memory_t;
  class data_cache_t;
  class symbol_map_t;
  class stack_cache_t;
  class instr_cache_t;
  class binary_format_t;
  class rtc_t;
  class excunit_t;

  /// Define the maximum number of slots in a bundle.
  static const unsigned int NUM_SLOTS = 2;

  /// Define the number of bytes used for the global main memory.
  static const unsigned int NUM_MEMORY_BYTES = 0x4000000;

  /// Define the number of bytes in a block transferred on an access to the 
  /// global main memory.
  static const unsigned int NUM_MEMORY_BLOCK_BYTES = 16;

  /// Define the number of bytes used for the local memory.
  static const unsigned int NUM_LOCAL_MEMORY_BYTES = 0x800;

  /// Define the number of bytes used for the data cache.
  static const unsigned int NUM_DATA_CACHE_BYTES = 0x800;

  /// Define the number of bytes used for the stack cache.
  static const unsigned int NUM_STACK_CACHE_BYTES = 0x800;

  /// Define the number of bytes used for the method cache.
  static const unsigned int NUM_METHOD_CACHE_BYTES = 0x1000;

  /// Define the maximum number of methods that can be cached in the method cache.
  static const unsigned int NUM_METHOD_CACHE_MAX_METHODS = 16;
  
  /// Define the number of bytes in a block of the method cache.
  static const unsigned int NUM_METHOD_CACHE_BLOCK_BYTES = 8;
  
  /// General-purpose register holding the program's exit code when terminating.
  static const GPR_e GPR_EXIT_CODE_INDEX = r1;

  /// Definition of Pipeline stages.
  enum Pipeline_t
  {
    /// No stage.
    SXX = -1,
    /// Fetch the bundle from instruction memory.
    SIF = 0,
    /// Decode the instruction's format and read register operands.
    SDR,
    /// Execute arithmetic operations and calculate addresses for memory
    /// accesses.
    SEX,
    /// Perform memory accesses and register write, if any.
    SMW,

    /// Index of last pipeline stage -- used to instantiate arrays etc.
    NUM_STAGES
  };

  /// Main class representing the simulation of a single Patmos core.
  class simulator_t
  {
  private:
    /// Runtime statistics for an instruction class.
    struct instruction_stat_t
    {
      /// Number of times an instruction of the instruction class was fetched.
      unsigned int Num_fetched;

      /// Number of times an instruction of the instruction class was retired
      /// (s.t. the predicate evaluated to true)
      unsigned int Num_retired;

      /// Number of times an instruction of the instruction class was retired
      /// (s.t. the predicate evaluated to false)
      unsigned int Num_discarded;
      
      void reset() {
        Num_fetched = Num_retired = Num_discarded = 0;
      }
    };


    /// Delayslot counter for 'calls' debug-fmt
    unsigned int Dbg_cnt_delay;

    /// Remember call / return status for 'calls' debug-fmt
    bool Dbg_is_call;
    bool Dbg_is_intr;

    /// A vector containing instruction statistics.
    typedef std::vector<instruction_stat_t> instruction_stats_t;

  public:
    /// Cycle counter
    uint64_t Cycle;

    /// The main memory used during the simulation.
    memory_t &Memory;

    /// The local memory used during the simulation.
    memory_t &Local_memory;

    /// The data cache used during the simulation.
    data_cache_t &Data_cache;

    /// The method cache used during the simulation.
    instr_cache_t &Instr_cache;

    /// The stack cache used during the simulation.
    stack_cache_t &Stack_cache;

    /// A map to retrieve symbol information from addresses.
    symbol_map_t &Symbols;

    /// Stack frame dumps for debugging
    dbgstack_t Dbg_stack;

    /// Real time clock
    rtc_t *Rtc;

    /// Exception handler
    excunit_t &Exception_handler;

    /// The decoder of the simulator.
    decoder_t Decoder;

    /// The base address of the current method.
    uword_t BASE;

    /// The program counter register.
    uword_t PC;

    /// The next value for program counter register.
    uword_t nPC;
    
    /// Old value of the program counter, for debugging purposes only.
    uword_t Debug_last_PC;

    /// The general purpose registers.
    GPR_t GPR;

    /// The predicate registers.
    PRR_t PRR;

    /// The special purpose registers.
    SPR_t SPR;

    /// Counter up to which pipeline stage the processor stalls.
    Pipeline_t Stall;

    /// Signal to disable the IF stage.
    bool Disable_IF;
    
    /// Interrupt instruction
    instruction_t *Instr_INTR;
    
    /// Halt pseudo instruction.
    instruction_t *Instr_HALT;

    /// Active instructions in the pipeline stage.
    instruction_data_t Pipeline[NUM_STAGES][NUM_SLOTS];

    /// Keep track of delays for interrupt triggering
    int Delay_counter;
    
    /// If set to true, flush the pipeline and halt the simulation.
    bool Halt;
    
    /// Delay decoder when an interrupt has been executed
    int Exception_handling_counter;

    /// Flush caches when PC reaches this address.
    uword_t Flush_Cache_PC;
    
    /// Debug accesses to those addresses.
    std::set<uword_t> Debug_mem_address;
    
    /// Runtime statistics on all instructions, per pipeline
    instruction_stats_t Instruction_stats[NUM_SLOTS];

    /// Count number of pipeline bubbles retired.
    uint64_t Num_bubbles_retired[NUM_SLOTS];

    /// Number of stall cycles per pipeline stage
    uint64_t Num_stall_cycles[NUM_STAGES];

    /// Number of NOPs executed
    uint64_t Num_NOPs;
    
    /// Profiling information for function profiling
    profiling_t Profiling;

    /// Print the internal register state of the simulator to an output stream 
    /// (excluding memories and caches)
    /// @param os An output stream.
    /// @param debug_fmt The selected output format.
    /// @param nopc skip printing cycles and PC
    void print_registers(std::ostream &os, debug_format_e debug_fmt, 
                         bool nopc = false) const;

    /// Perform a step of the simulation for a given pipeline.
    /// @param pst The pipeline stage.
    /// @param f The simulation/commit function to invoke.
    /// @param debug_out Stream to print debug output.
    /// @param debug Flag indicating whether debug output should be printed.
    void pipeline_invoke(Pipeline_t pst,
                         void (instruction_data_t::*f)(simulator_t &),
                         bool debug = false,
                         std::ostream &debug_out = std::cerr);

    /// Flush the pipeline up to *not* including the given pipeline stage.
    /// @param pst The pipeline stage up to which instructions should be
    /// flushed.
    void pipeline_flush(Pipeline_t pst);

    /// Stall the pipeline up to *not* including the given pipeline stage.
    /// @param pst The pipeline stage up to which instructions should be
    /// stalled.
    void pipeline_stall(Pipeline_t pst);

    /// Check if the given pipeline stage is currently stalled, either by
    /// itself or any following stage.
    bool is_stalling(Pipeline_t pst) const;

    /// Halt the simulation. All instructions currently in flight will be 
    /// completed first.
    void halt();
    
    /// Check if the simulator has been requested to halt.
    bool is_halting() const;
    
    /// Track retiring instructions for stats.
    void track_retiring_instructions();

    /// Simulate the instruction fetch stage.
    void instruction_fetch();

  public:
    /// Construct a new instance of a Patmos-core simulator
    /// The simulator only retains the references of the arguments passed in the
    /// constructor, i.e., it does not clone them, proclaim ownership, etc.
    /// @param memory The main memory to use during the simulation.
    /// @param local_memory The local memory to use during the simulation.
    /// @param data_cache The data cache to use during the simulation.
    /// @param instr_cache The instruction cache to use during the simulation.
    /// @param stack_cache The stack cache to use during the simulation.
    /// @param symbols A mapping from addresses to symbols.
    simulator_t(memory_t &memory, memory_t &local_memory,
                data_cache_t &data_cache, instr_cache_t &instr_cache,
                stack_cache_t &stack_cache, symbol_map_t &symbols,
                excunit_t &excunit);

    // Destroy an instance of a Patms-core simulator
    ~simulator_t();

    /// Flush all data caches when reaching the given program counter.
    void flush_caches_at(uword_t address) { Flush_Cache_PC = address; }
    
    /// Print accesses to a 
    void debug_mem_address(uword_t address) { Debug_mem_address.insert(address); }
    
    /// Run the simulator.
    /// @param entry Initialize the method cache, PC, etc. to start execution
    /// from this entry address.
    /// @param debug_cycle Print debug trace starting at the given cycle.
    /// @param debug_fmt Format of the debug trace.
    /// @param debug_out Stream to print debug output.
    /// @param debug_nopc skip printing cycles and PC
    /// @param max_cycles The maximum number of cycles to run the simulation.
    /// @param profiling Enable profiling in the simulation run.
    /// @param collect_instr_stats
    void run(word_t entry = 0,
             uint64_t debug_cycle = std::numeric_limits<uint64_t>::max(),
             debug_format_e debug_fmt = DF_DEFAULT,
             std::ostream &debug_out = std::cerr, bool debug_nopc = false,
             uint64_t max_cycles = std::numeric_limits<uint64_t>::max(),
             bool collect_instr_stats = false);

    /// Print the instructions and their operands in a pipeline stage
    /// @param os An output stream.
    /// @param debug_fmt The stage to print.
    /// @param nopc skip printing cycles and PC
    void print_instructions(std::ostream &os, Pipeline_t stage, bool nopc) const;

    /// Print the internal state of the simulator to an output stream.
    /// @param os An output stream.
    /// @param debug_fmt The selected output format.
    /// @param nopc skip printing cycles and PC
    void print(std::ostream &os, debug_format_e debug_fmt, bool nopc);

    /// Print runtime statistics of the current simulation run to an output
    /// stream, using the current debug stack stats options.
    /// @param os An output stream.
    void print_stats(std::ostream &os) const;

    /// Print runtime statistics of the current simulation run to an output
    /// stream.
    /// @param os An output stream.
    void print_stats(std::ostream &os, const stats_options_t& options) const;

    /// Reset all simulation statistics.
    void reset_stats();
    
    /// Flush all caches.
    void flush_caches();
  };


  /// Print the name of the pipeline stage to the output stream.
  /// @param os The output stream.
  /// @param p The pipeline stage.
  /// @return The output stream.
  std::ostream &operator<<(std::ostream &os, Pipeline_t p);
}

#endif // PATMOS_SIMULATOR_CORE_H


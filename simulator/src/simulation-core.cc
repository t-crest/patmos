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
#include "symbol.h"
#include "interrupts.h"
#include "instructions.h"
#include "rtc.h"

#include <ios>
#include <iostream>
#include <iomanip>
#include <limits>

namespace patmos
{
  simulator_t::simulator_t(memory_t &memory, memory_t &local_memory,
                           data_cache_t &data_cache,
                           instr_cache_t &instr_cache,
                           stack_cache_t &stack_cache, symbol_map_t &symbols,
                           interrupt_handler_t &interrupt_handler)
    : Dbg_cnt_delay(0), 
      Cycle(0), Memory(memory), Local_memory(local_memory),
      Data_cache(data_cache), Instr_cache(instr_cache),
      Stack_cache(stack_cache), Symbols(symbols), Dbg_stack(*this),
      Interrupt_handler(interrupt_handler),
      BASE(0), PC(0), nPC(0), Debug_last_PC(0),
      Stall(SXX), Disable_IF(false), Is_decoupled_load_active(false), 
      Branch_counter(0), Halt(false), Interrupt_handling_counter(0),
      Flush_Cache_PC(std::numeric_limits<unsigned int>::max()), Num_NOPs(0)
  {
    // initialize one predicate register to be true, otherwise no instruction
    // will ever execute
    PRR.set(p0, true);
    // initialize negated predicates
    for (unsigned int p = pn1; p < NUM_PRRn; p++) {
	PRR.set((PRR_e)p, true);
    }

    // initialize the pipeline
    for(unsigned int i = 0; i < NUM_STAGES; i++)
    {
      Num_stall_cycles[i] = 0;
      for(unsigned int j = 0; j < NUM_SLOTS; j++)
      {
        Pipeline[i][j] = instruction_data_t();
      }
    }

    // Initialize instruction statistics
    for(unsigned int j = 0; j < NUM_SLOTS; j++)
    {
      Instruction_stats[j].resize(Decoder.get_num_instructions());

      Num_bubbles_retired[j] = 0;
    }

    // Create the interrupt instruction
    Instr_INTR       = new i_intr_t();
    Instr_INTR->ID   = -1;
    Instr_INTR->Name = "intr";
    
    Instr_HALT       = new i_halt_t();
    Instr_HALT->ID   = -2;
    Instr_HALT->Name = "halt";
  }



  simulator_t::~simulator_t()
  {
    delete Instr_INTR;
    delete Instr_HALT;
  }


  void simulator_t::pipeline_invoke(Pipeline_t pst,
                                   void (instruction_data_t::*f)(simulator_t &),
                                   bool debug, std::ostream &debug_out)
  {
    if (debug)
    {
      debug_out << pst << " : ";
    }

    // invoke simulation functions
    for(unsigned int i = 0; i < NUM_SLOTS; i++)
    {
      // debug output
      if (debug)
      {
        if (i != 0)
        {
          debug_out << " || ";
        }
        Pipeline[pst][i].print(debug_out, Symbols);
        debug_out.flush();
      }

      // simulate the respective pipeline stage of the instruction
      if (f) {
        (Pipeline[pst][i].*f)(*this);
      }
    }

    if (debug)
    {
      if (Stall == pst) debug_out << "            (stalling)";
      debug_out << "\n";
    }
  }

  void simulator_t::pipeline_flush(Pipeline_t pst)
  {
    for (unsigned int i = 0; i < pst; i++)
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

  bool simulator_t::is_stalling(Pipeline_t pst) const
  {
    return Stall >= pst;
  }

  void simulator_t::halt() 
  {
    Halt = true;
  }
  
  bool simulator_t::is_halting() const 
  {
    return Halt;
  }
  
  void simulator_t::track_retiring_instructions()
  {
    if (Stall != NUM_STAGES-1)
    {
      for(unsigned int j = 0; j < NUM_SLOTS; j++)
      {
        instruction_data_t &ops = Pipeline[NUM_STAGES-1][j];
        
        if (ops.I && ops.I->ID >= 0)
        {
          // get instruction statistics
          instruction_stat_t &stat = Instruction_stats[j][ops.I->ID];

          // update instruction statistics
          if (ops.DR_Pred)
            stat.Num_retired++;
          else
            stat.Num_discarded++;
        }
        else
          Num_bubbles_retired[j]++;
      }
    }
  }



  void simulator_t::instruction_fetch()
  {
    // we get a pointer to the instructions of the IF stage, for easier
    // reference
    instruction_data_t *instr_SIF = Pipeline[SIF];

    if (Halt) {
      // When we are halting, just fill the pipeline with halt instructions
      // halt when we flushed the whole pipeline. 
      instr_SIF[0] = instruction_data_t::mk_CFLrt(*Instr_HALT, p0, 1, r0, r0);

      for(unsigned int i = 1; i < NUM_SLOTS; i++)
      {
        instr_SIF[i] = instruction_data_t();
      }
      
      return;
    }
    
    // Fetch the instruction word from the method cache.
    // NB: We fetch in each cycle, as preparation for supporting a standard
    //     I-Cache in addition.
    word_t iw[NUM_SLOTS];
    if (!Instr_cache.fetch(BASE, PC, iw))
    {
      // Stall the whole pipeline
      pipeline_stall(SMW);
    } else {
      // Disable fetching until we filled the pipeline with a bubble at IF 
      // again. This is to prevent re-fetching an instruction while we update
      // the method cache.
      Disable_IF = true;
    }

    // Decode the next instruction, or service an interrupt,
    // only if we are not stalling.
    if (Stall == SXX)
    {

      if (Interrupt_handler.interrupt_pending() &&
          Branch_counter == 0 &&
          Interrupt_handling_counter == 0)
      {
        interrupt_t &interrupt = Interrupt_handler.get_interrupt();

        instr_SIF[0] = instruction_data_t::mk_CFLi(*Instr_INTR, p0,
                                                   interrupt.Address, interrupt.Address, 0);
        instr_SIF[0].Address = PC;

        for(unsigned int i = 1; i < NUM_SLOTS; i++)
        {
          instr_SIF[i] = instruction_data_t();
        }

        // Handling interrupt, next CPU cycle no new instructions have to be decoded
        Interrupt_handling_counter = 3;

      }
      else if (Interrupt_handling_counter > 0)
      {
        // Putting more empty instrutions after an interrupt
        for(unsigned int i = 0; i < NUM_SLOTS; i++)
        {
          instr_SIF[i] = instruction_data_t();
        }
        Interrupt_handling_counter--;

      }
      else
      {
        // decode the instruction word.
        unsigned int iw_size = Decoder.decode(iw, instr_SIF);

        // unknown instruction: throw exception
        if (iw_size == 0)
        {
          simulation_exception_t::illegal(from_big_endian<big_word_t>(iw[0]));
        }

        if(instr_SIF[0].I->is_flow_control())
          Branch_counter = instr_SIF[0].I->get_delay_slots(instr_SIF[0]);
        else if (Branch_counter)
          Branch_counter--;


        for(unsigned int j = 0; j < NUM_SLOTS; j++)
        {
          // assign fetch address to new instructions
          instr_SIF[j].Address = PC;

          // track instructions fetched
          if (instr_SIF[j].I)
            Instruction_stats[j][instr_SIF[j].I->ID].Num_fetched++;
        }

        // Detect NOPs as "subi r0 = ...;"
        if (Decoder.is_NOP(&instr_SIF[0]) && !instr_SIF[1].I) 
        {
          Num_NOPs++;
        }

        // provide next program counter value (as incremented PC)
        nPC = PC + iw_size*4;
      }

    }
  }

  void simulator_t::run(word_t entry, uint64_t debug_cycle,
                        debug_format_e debug_fmt, std::ostream &debug_out,
                        bool debug_nopc, uint64_t max_cycles,
                        bool collect_instr_stats)
  {

    // do some initializations before executing the first instruction.
    if (Cycle == 0)
    {
      BASE = PC = nPC = entry;
      Instr_cache.initialize(entry);
      Profiling.initialize(entry);
      Dbg_stack.initialize(entry);
    }

    try
    {
      // start of main simulation loop
      for(uint64_t cycle = 0; cycle < max_cycles; cycle++, Cycle++)
      {
        bool debug = (Cycle >= debug_cycle);
        bool debug_pipeline = debug && (debug_fmt >= DF_LONG);
        
        // reset the stall counter.
        Stall = SXX;

        // Simulate the instruction fetch stage first.
        // MW stage might need the nPC from instruction fetch for return info.
        if (!Disable_IF) {
          instruction_fetch();
        }

        // simulate decoupled load
        Decoupled_load.dMW(*this);

        if (debug_pipeline)
        {
          debug_out << "dMW: ";
          Decoupled_load.print(debug_out, Symbols);
          debug_out << "\n";
        }

        // invoke simulation functions
        pipeline_invoke(SMW, &instruction_data_t::MW, debug_pipeline);
        pipeline_invoke(SEX, &instruction_data_t::EX, debug_pipeline);
        pipeline_invoke(SDR, &instruction_data_t::DR, debug_pipeline);
        // invoke IF only for printing
        pipeline_invoke(SIF, NULL, debug_pipeline);
        
        // print instructions in EX stage
        if (debug && debug_fmt == DF_INSTRUCTIONS)
        {
          print_instructions(debug_out, SEX, debug_nopc);
        }

        track_retiring_instructions();

        // Move pipeline stages and insert bubbles after stalling stage.
        // If Stall == SXX, we do not stall, but a bubble is inserted in SIF,
        // which is later replaced by the fetched instruction.
        for (int i = SMW; i >= Stall+1; i--)
        {
          for (unsigned int j = 0; j < NUM_SLOTS; j++)
          {
            Pipeline[i][j] = (i==Stall+1)
                                ? instruction_data_t() // insert bubble
                                : Pipeline[i-1][j];    // get previous stage
          }
        }

        // Update the Program counter. Either nPC was updated in the fetch stage
        // or it was overwritten by a CFL instruction in EX/MW stage.
        if (!is_stalling(SIF)) {
          // As soon as we reach the flush PC, flush, so that we do not hit this
          // again when we stall.
          if (PC != nPC && nPC == Flush_Cache_PC) {
            flush_caches();
          }
          
          PC = nPC;
          // We just inserted a bubble at SIF before, enable fetching for 
          // this new instruction.
          Disable_IF = false;
        }

        // if we are stalling in MW, reset the bypass in EX so that it can be
        // filled by the stalled EX stage again (needs to be done for all
        // stalled bypasses).
        if (is_stalling(SEX)) {
          for(unsigned int i = 0; i < NUM_SLOTS; i++)
          {
            Pipeline[SEX][i].GPR_EX_Rd.reset();
          }
        }

        // track pipeline stalls
        Num_stall_cycles[Stall]++;

        // advance the time for the method cache, stack cache, and memory
        Rtc->tick();
        Memory.tick();
        Instr_cache.tick();
        Stack_cache.tick();

        if (debug)
        {
          print(debug_out, debug_fmt, debug_nopc);
        }

        // Collect stats for instructions
        if (collect_instr_stats) {
          for (unsigned int j = 0; j < NUM_SLOTS; j++)
          {
            if (Pipeline[SMW][j].I && Pipeline[SMW][j].I->ID >= 0) {
              // I am too lazy now to remove all the const's..
              instruction_t &I(Decoder.get_instruction(Pipeline[SMW][j].I->ID));
              I.collect_stats(*this, Pipeline[SMW][j]);
            }
          }
        }
      } // end of simulation loop
    }
    catch (simulation_exception_t e)
    {
      Profiling.finalize(Cycle);

      // pass on to caller
      e.set_cycle(Cycle, PC);
      throw e;
    }

    Profiling.finalize(Cycle);
  }

  void simulator_t::print_registers(std::ostream &os,
                                    debug_format_e debug_fmt, bool nopc) const
  {
    if (debug_fmt == DF_SHORT)
    {
      for(unsigned int r = r0; r < NUM_GPR; r++)
      {
        os << boost::format(" r%1$-2d: %2$08x") % r % GPR.get((GPR_e)r).get();
      }
      os << "\n";
    }
    else
    {
      if (!nopc) {
        os << boost::format("\nCyc : %1%\n PRR: ") % Cycle;
      } else {
        os << "PRR: ";
      }

      // print values of predicate registers
      unsigned int sz_value = 0;
      for(int p = NUM_PRR - 1; p >= 0; p--)
      {
        bit_t pred_value = PRR.get((PRR_e)p).get();
        sz_value |= pred_value << p;
        os << pred_value;
      }

      if (!nopc) {
        os << boost::format("  BASE: %1$08x   PC : %2$08x   ")
          % BASE % PC;

        Symbols.print(os, PC);
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

      // print values of special purpose registers -- special handling of SZ.
      os << boost::format("s0 : %1$08x   ") % sz_value;
      for(unsigned int s = s1; s < NUM_SPR; s++)
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
    }
  }

  void simulator_t::print_instructions(std::ostream &os, Pipeline_t stage, 
                                       bool nopc) const 
  {
      std::ostringstream oss;
      symbol_map_t emptymap;
      for(unsigned int i = 0; i < NUM_SLOTS; i++) {
        std::ostringstream instr;
        Pipeline[stage][i].print(instr, emptymap);
        
        if (i != 0) oss << " || ";
        oss << boost::format("%1% %|30t|") % instr.str();
      }

      if (nopc) {
        os << boost::format("%1% %|75t|") % oss.str();
      } else {
        os << boost::format("%1$08x %2$9d %3% %|75t|") 
                    % Pipeline[stage][0].Address % Cycle % oss.str();
      }

      if (is_stalling(stage)) {
        // Avoid peeking into memory while memory stage is still working by
        // not printing out operands in this case
        os << "stalling";
        if (Disable_IF) os << ", IF disabled";
      } else {
        for(unsigned int i = 0; i < NUM_SLOTS; i++) {
          if (!Pipeline[stage][i].I) continue;
          if (i != 0) os << " || ";
          Pipeline[stage][i].print_operands(*this, os, Symbols);
        }
      }

      os << "\n";      
  }
  
  /// Read a GPR register at the EX stage.
  /// @param s The parent simulator.
  /// @param op The register operand.
  /// @return The register value, considering by-passing from the EX, and MW
  /// stages.
  static inline word_t read_GPR_post_EX(const simulator_t &s, GPR_e reg)
  {
    // Note: we are between cycles, so the EX bypass has been updated, 
    // the EX instructions have been moved to MW, but the MW bypass is not yet set.
    return s.Pipeline[SMW][1].GPR_EX_Rd.get(
           s.Pipeline[SMW][0].GPR_EX_Rd.get(
              s.GPR.get(reg))).get();
  }

  void simulator_t::print(std::ostream &os, debug_format_e debug_fmt, bool nopc)
  {
    if (debug_fmt == DF_TRACE)
    {
      // CAVEAT: this trace mode is used by platin's 'analyze-trace' module
      // do not change without adapting platin
      if (!is_stalling(SIF) && !is_halting())
      {
        // Boost::format is unacceptably slow (adpcm.elf):
        //  => no debugging output:  1.6s
        //  => os with custom formatting: 2.4s
        //  => boost::format: 10.6s !!
        uword_t addr = Pipeline[SMW][0].Address;
        if (addr) {
          os << std::hex << std::setw(8) << std::setfill('0') << addr << ' '
             << std::dec << Cycle << '\n' << std::setfill(' ');
        }
        // os << boost::format("%1$08x %2%\n") % PC % Cycle;
      }
      return;
    }
    else if (debug_fmt == DF_INSTRUCTIONS) 
    {
      // already done before
      return;
    }
    else if (debug_fmt == DF_BLOCKS) {
      if (!is_stalling(SIF) && !is_halting()) {
        // Check if we either hit a new block, or if we did some jump or return
        // somewhere into a block
        if (Symbols.contains(PC) || 
            PC < Debug_last_PC || PC > Debug_last_PC + NUM_SLOTS * 4)
        {
          if (!nopc) {
            os << boost::format("%1$08x %2$9d ") % PC % Cycle;
          }
          Symbols.print(os, PC);
          os << "\n";
        }
        // Remember the current PC to check for jumps
        Debug_last_PC = PC;
      }
      return;
    }
    else if (debug_fmt == DF_CALLS || debug_fmt == DF_CALLS_INDENT) {
      if (Dbg_cnt_delay == 1) {
        if (is_stalling(SMW)) return;
        
        if (Dbg_is_intr) {
          // Anything operands we can print for an interrupt call?
        } else if (Dbg_is_call) {
          os << " args: " << boost::format("r3 = %1$08x, r4 = %2$08x, ") 
                % read_GPR_post_EX(*this, r3) % read_GPR_post_EX(*this, r4);
          os << boost::format("r5 = %1$08x, r6 = %2$08x, r7 = %3$08x, r8 = %4$08x") 
                % read_GPR_post_EX(*this, r5) % read_GPR_post_EX(*this, r6)
                % read_GPR_post_EX(*this, r7) % read_GPR_post_EX(*this, r8);
        } else {
          os << " retval: " << boost::format("r1 = %1$08x, r2 = %2$08x") 
                % read_GPR_post_EX(*this, r1) % read_GPR_post_EX(*this, r2);
        }
        os << "\n";
        Dbg_cnt_delay = 0;
      }
      else if (Dbg_cnt_delay > 1) {
        if (!is_stalling(SMW)) {
          Dbg_cnt_delay--;
        }
      }
      else if (Pipeline[SMW][0].I && Pipeline[SMW][0].DR_Pred &&
               Pipeline[SMW][0].I->is_flow_control()) {
        std::string name = Pipeline[SMW][0].I->Name;
        Dbg_cnt_delay = 0;
        Dbg_is_intr = false;
        if (name == "ret") {
          Dbg_cnt_delay = 3;
          Dbg_is_call = false;
        }
        else if (name == "call" || name == "callr") {
          Dbg_cnt_delay = 3;
          Dbg_is_call = true;
        }
        else if (name == "intr") {
          Dbg_cnt_delay = 3;
          Dbg_is_intr = true;
        }
        if (Dbg_cnt_delay) {
          if (!nopc) {
            os << boost::format("%1$08x %2$9d ") % PC % Cycle;
          }
          
          if (debug_fmt == DF_CALLS_INDENT) {
            for (unsigned i = 0; i < Dbg_stack.size(); i++) { 
              os << "  ";
            }
          }
          os << (Dbg_is_intr ? "interrupt" : (Dbg_is_call ? "call" : "return"));
          os << " from ";
          Symbols.print(os, Pipeline[SMW][0].Address, true);
          os << " to ";
          Symbols.print(os, Pipeline[SMW][0].EX_Address, true);
        }
      }
    }
    else
    {
      // print register values
      print_registers(os, debug_fmt, nopc);

      if (debug_fmt == DF_ALL)
      {
        // print state of method cache
        os << "Method Cache:\n";
        Instr_cache.print(os);

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
    }
  }

  void simulator_t::flush_caches() 
  {
    Instr_cache.flush_cache();
    Data_cache.flush_cache();
    // TODO flush the stack cache
  }
  
  void simulator_t::reset_stats()
  {
    // TODO reset the statistics in the pipeline stages in the correct cycles.
    // Send a control signal down the pipeline, resetting statistics on the way.
    
    for(unsigned int i = 0; i < NUM_STAGES; i++)
    {
      Num_stall_cycles[i] = 0;
    }

    for(unsigned int j = 0; j < NUM_SLOTS; j++)
    {
      for (unsigned int k = 0; k < Decoder.get_num_instructions(); k++) { 
        Instruction_stats[j][k].reset();
      }

      Num_bubbles_retired[j] = 0;
    }
    
    Num_NOPs = 0;
    
    Instr_cache.reset_stats();
    Data_cache.reset_stats();
    Stack_cache.reset_stats();
    Memory.reset_stats();
    Profiling.reset_stats(Cycle);
  }
  
  void simulator_t::print_stats(std::ostream &os, bool short_stats,
                                bool instr_stats) const
  {
    // print register values
    if (!short_stats) {
      print_registers(os, DF_DEFAULT);
    }

    uint64_t num_total_fetched[NUM_SLOTS];
    uint64_t num_total_retired[NUM_SLOTS];
    uint64_t num_total_discarded[NUM_SLOTS];
    uint64_t num_total_bubbles[NUM_SLOTS];
    
    uint64_t sum_fetched = 0;
    uint64_t sum_discarded = 0;
    
    if (!short_stats) {
      os << boost::format("\n\nInstruction Statistics:\n   %1$15s:") % "operation";
    }
    
    for (unsigned int i = 0; i < NUM_SLOTS; i++) {
      num_total_fetched[i] = num_total_retired[i] = num_total_discarded[i] = 0;
      num_total_bubbles[i] = 0;
      num_total_bubbles[i] += Num_bubbles_retired[i];
    
      if (!short_stats) {
        os << boost::format(" %1$10s %2$10s %3$10s")
            % "#fetched" % "#retired" % "#discarded";
      }
    }
    os << "\n";
          
    for(unsigned int i = 0; i < Instruction_stats[0].size(); i++)
    {
      // get instruction and statistics on it
      const instruction_t &I(Decoder.get_instruction(i));

      if (!short_stats) {
        os << boost::format("   %1$15s:") % I.Name;
      }
      
      for (unsigned int j = 0; j < NUM_SLOTS; j++) {
        const instruction_stat_t &S(Instruction_stats[j][i]);

        // If we reset the statistics counters, we might have some 
        // instructions that were in flight at the reset and thus are 
        // counted as discarded but not as fetched
        //assert(S.Num_fetched >= (S.Num_retired + S.Num_discarded));
       
        if (!short_stats) {
          os << boost::format(" %1$10d %2$10d %3$10d")
            % S.Num_fetched % S.Num_retired % S.Num_discarded;
        }

        // collect summary
        num_total_fetched[j] += S.Num_fetched;
        num_total_retired[j] += S.Num_retired;
        num_total_discarded[j] += S.Num_discarded;
      }
      
      if (!short_stats && instr_stats) {
        os << "\t";
        I.print_stats(*this, os, Symbols);
      }
      
      if (!short_stats) {
        os << "\n";
      }
    }

    // summary over all instructions
    if (!short_stats) {
      os << boost::format("   %1$15s:") % "all";
    }
    for (unsigned int j = 0; j < NUM_SLOTS; j++) {
      if (!short_stats) {
        os << boost::format(" %1$10d %2$10d %3$10d")
         % num_total_fetched[j] % num_total_retired[j] % num_total_discarded[j];
      }
          
      sum_fetched += num_total_fetched[j];
      sum_discarded += num_total_discarded[j];
    }
    if (!short_stats) {
      os << "\n";
      os << boost::format("   %1$15s:") % "bubbles";
      for (unsigned int j = 0; j < NUM_SLOTS; j++) {
        os << boost::format(" %1$10s %2$10d %3$10s") % "-" % num_total_bubbles[j] % "-";
      }          
      os << "\n";
    }

    
    uint64_t sum_stalls = 0;
    
    if (!short_stats) {
      os << "\nStall Cycles:\n";
    }
    for (int i = SIF; i < NUM_STAGES; i++)
    {
      if (!short_stats) {
        os << boost::format("   %1%: %2%\n")
          % (Pipeline_t)i % Num_stall_cycles[i];
      }
         
      sum_stalls += Num_stall_cycles[i];
    }
    
    // Cycle statistics
    
    // Note that by definition of a NOP it is not bundled
    uint64_t num_operations = sum_fetched - Num_NOPs;
    uint64_t num_instructions = num_total_fetched[0] - Num_NOPs;

    // Should we count only retired instructions?
    float cpo_nops = (float)Cycle / (float)sum_fetched;
    float cpi_nops = (float)Cycle / (float)num_total_fetched[0];
    float cpo = (float)Cycle / (float)num_operations;
    float cpi = (float)Cycle / (float)num_instructions;
    
    os << boost::format("\nCycles per Instruction (w/  NOPs): %1$8.4f"
                        "\nCycles per Operation   (w/  NOPs): %2$8.4f"
                        "\nCycles per Instruction (w/o NOPs): %3$8.4f"
                        "\nCycles per Operation   (w/o NOPs): %4$8.4f")
        % cpi_nops % cpo_nops % cpi % cpo;

    os << "\n\n                   total     % cycles";
    os << boost::format("\nCycles:       %1$10d") 
          % Cycle;
    os << boost::format("\nInstructions: %1$10d  %2$10.2f%%"
                        "\nNOPs:         %3$10d  %4$10.2f%%"
                        "\nStalls:       %5$10d  %6$10.2f%%")
          % num_instructions % (100.0 * (float)num_instructions / (float)Cycle)
          % Num_NOPs % (100.0 * (float)Num_NOPs / (float)Cycle)
          % sum_stalls % (100.0 * (float)sum_stalls / (float)Cycle);
    
    os << "\n\n                   total        % ops";
    os << boost::format("\nOperations:   %1$10d  %2$10.2f%%"
                        "\nNOPs:         %3$10d  %4$10.2f%%"
                        "\nBundled:      %5$10d  %6$10.2f%%"
                        "\nDiscarded:    %7$10d  %8$10.2f%%")
          % num_operations % (100.0 * (float)num_operations / (float)sum_fetched)
          % Num_NOPs % (100.0 * (float)Num_NOPs / (float)sum_fetched)
          % (sum_fetched - num_total_fetched[0])
                     % (100.0 * (float)(sum_fetched - num_total_fetched[0]) / 
                                                            (float)sum_fetched)
          % sum_discarded % (100.0 * (float)sum_discarded / (float)sum_fetched);

    os << "\n\n                            ";
    for (int i = 0; i < NUM_SLOTS; i++) {
      os << "      slot " << i;
    }
    os << "\nSlot utilization (w/ NOPs): ";
    for (int i = 0; i < NUM_SLOTS; i++) {
      os << boost::format(" %1$10.2f%%") 
         % (100.0 * (float)num_total_fetched[i] / (float)num_total_fetched[0] );
    }
          
    // print statistics of method cache
    os << "\n\nInstruction Cache Statistics:\n";
    Instr_cache.print_stats(*this, os, short_stats);

    // print statistics of data cache
    os << "\n\nData Cache Statistics:\n";
    Data_cache.print_stats(*this, os, short_stats);

    // print statistics of stack cache
    os << "\n\nStack Cache Statistics:\n";
    Stack_cache.print_stats(*this, os, short_stats);

    // print statistics of main memory
    os << "\n\nMain Memory Statistics:\n";
    Memory.print_stats(*this, os, short_stats);

    // print profiling information
    if (!short_stats) {
      Profiling.print(os, Symbols);
    }

    os << "\n";
  }


  std::ostream &operator<<(std::ostream &os, Pipeline_t p)
  {
    if (p == SXX) {
      return os;
    }
    
    const static char* names[NUM_STAGES] = {"IF", "DR", "EX", "MW"};
    assert(names[p] != NULL);

    os << names[p];

    return os;
  }
}

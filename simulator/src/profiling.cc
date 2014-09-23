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
// Helper class for function profiling.
//


#include "profiling.h"

//#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/format.hpp>


namespace patmos
{

  void profiling_t::initialize(uword_t e, uint64_t cycle)
  {
    entry = e;
    reset_cycle = cycle;
    enter(entry, cycle);
  }


  void profiling_t::finalize(uint64_t cycle)
  {
    // empty the callstack
    while (!stack.empty()) {
      leave(cycle);
    }
  }


  bool profiling_t::empty() const
  {
    return cycles_map.empty();
  }


  void profiling_t::enter(uword_t addr, uint64_t cycle)
  {
    //std::cerr << "PUSH " << std::hex << addr << "\n";
    // create entry for function on demand
    if (!cycles_map.count(addr)) {
      prof_funcinfo_t callee = {0};
      callee.min = (uint64_t) -1U;
      cycles_map[addr] = callee;
    }
    // add self-cycles to caller (current function)
    if (!stack.empty()) {
      // If we reset the profiling info, use the entered function as new entry
      if (!cycles_map.count(stack.back())) {
        entry = addr;
      } else {
        cycles_map[stack.back()].self += cycle-last_cycle;
      }
    }
    // switch to callee
    stack.push_back(addr);
    // update last_cycle
    last_cycle = cycle;

    // update callee info
    prof_funcinfo_t *callee = &cycles_map[addr];
    callee->num_calls++;
    if (callee->depth == 0)
      callee->enter_cycle = cycle;
    callee->depth++;
    if (callee->depth > callee->maxdepth)
      callee->maxdepth = callee->depth;
  }


  void profiling_t::leave(uint64_t cycle)
  {

    //std::cerr << "POP " << std::hex << stack.back() << "\n";

    if (stack.empty()) {
      // TODO Something nasty with longjump happened. Can we recover somehow?
      // TODO at least print a warning?
      return;
    }
    
    cycles_map_t::iterator it = cycles_map.find(stack.back());
    
    prof_funcinfo_t *callee;
    
    if (it == cycles_map.end()) {
      // No entry found, we return from a caller where we reset the stats in the 
      // meantime.
      prof_funcinfo_t new_callee = {0};
      new_callee.min = (uint64_t) -1U;
      new_callee.enter_cycle = reset_cycle;
      new_callee.depth = 1;
      cycles_map[stack.back()] = new_callee;
      
      // We just use the caller as the entry function for now, it will be 
      // updated while we go up the call stack to the root.
      entry = stack.back();
      
      callee = &cycles_map[stack.back()];
    } 
    else {
      // update callee info (current function)
      callee = &it->second;
    }

    // self cycles
    callee->self += cycle-last_cycle;

    // record cycles spent down the calltree
    callee->depth--;
    if (callee->depth == 0) {
      uint64_t diff = cycle - callee->enter_cycle;
      callee->total += diff;
      if (diff < callee->min)
        callee->min = diff;
      if (diff > callee->max)
        callee->max = diff;
    }

    // return to caller
    stack.pop_back();
    // update last_cycle
    last_cycle = cycle;

  }


  std::ostream &profiling_t::print(std::ostream &os, symbol_map_t &sym) const
  {
    uint64_t total = cycles_map.at(entry).total;

    os << "\n\nProfiling information:\n\n Function\n"
      "  #calls       min         max           avg      "
      "cycles (abs)  cycles (rel)\n";

    for(std::map<uword_t, prof_funcinfo_t>::const_iterator
            i = cycles_map.begin(), e = cycles_map.end(); i != e; ++i)
    {
      std::stringstream func_name;

      if (sym.contains(i->first)) {
        sym.print(func_name, i->first, true);
      } else {
        func_name << boost::format("%#x") % i->first;
      }

      const prof_funcinfo_t *entry = &i->second;

      os << "  " << func_name.str() << "\n";
      // self (flat)
      os << boost::format("%|46t|%16d     %8.4f%%\n")
        % entry->self
        % ((double)(entry->self*100.0) / (double)total);
      // cumulative (calltree)
      os << boost::format("  %4d  %10d  %10d  %12.1f  %16d     %8.4f%%\n")
        % entry->num_calls
        % entry->min
        % entry->max
        % ((double)entry->total / (double)entry->num_calls)
        % entry->total
        % ((double)(entry->total*100.0) / (double)total);
    }
    os << "\n";
    return os;
  }
  
  void profiling_t::reset_stats(uint64_t cycle) 
  {
    reset_cycle = cycle;
    cycles_map.clear();
  }

} // end namespace patmos

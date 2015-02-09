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
// Implementation of the instruction SPM..
//
#include "instr-spm.h"

#include "memory.h"
#include "exception.h"
#include "simulation-core.h"

#include <cmath>
#include <ostream>

#include <boost/format.hpp>

using namespace patmos;

bool instr_spm_t::fetch(simulator_t &s, uword_t base, uword_t address, word_t iw[NUM_SLOTS])
{
  if (address < Size) {
    // Just read directly from global memory without any latency.
    // TODO we should simulate copying the data to a local buffer though
    Memory->read_peek(s, address, reinterpret_cast<byte_t*>(iw), 
                      sizeof(word_t)*NUM_SLOTS);
    return true;
  } else {
    return Cache->fetch(s, base, address, iw);
  }
}

bool instr_spm_t::load_method(simulator_t &s, word_t address, word_t offset)
{
  if (address < Size) {
    Num_loads++;
    Method_stats[address]++;
    return true;
  } else {
    return Cache->load_method(s, address, offset);
  }
}

bool instr_spm_t::is_available(simulator_t &s, word_t address)
{
  if (address < Size) {
    return true;
  } else {
    return Cache->is_available(s, address);
  }
}


void instr_spm_t::print_stats(const simulator_t &s, std::ostream &os, 
                                   const stats_options_t& options)
{
  //os << "                              total        max.\n";
  os << "                              total\n";
  os << boost::format("   I-SPM calls/returns : %1$10d\n")
   % Num_loads;
  
  if (!options.short_stats && Num_loads > 0) {
    os << "\n";
    os << "       Method:        #hits\n";
    
    for(method_stats_t::iterator i(Method_stats.begin()),
        ie(Method_stats.end()); i != ie; i++)
    {
      os << boost::format("   0x%1$08x:   %2$10d       %3%\n")
        % i->first % i->second % s.Symbols.find(i->first);
    }
  }
   
  os << "\n";
  
  Cache->print_stats(s, os, options);
}

void instr_spm_t::reset_stats() 
{
  Method_stats.clear();
  Num_loads = 0;
  
  Cache->reset_stats();
}

void instr_spm_t::flush_cache()
{
  // Note: we do not want to flush the I-SPM here (if we would Implement
  // the SPM as a local buffer), otherwise flushing the I$ through the 
  // control-bits would also flush the I-SPM.
  
  Cache->flush_cache();
}

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

#ifndef PATMOS_PROFILING_H
#define PATMOS_PROFILING_H

#include <ostream>
#include <vector>
#include <map>

#include "symbol.h"

namespace patmos
{

  /// Profiling information for functions.
  class profiling_t
  {
    private:

      /// prof_funcinfo_t - Data collected for profiling a function.
      typedef struct {
        int num_calls;
        int depth;
        int maxdepth;
        uint64_t enter_cycle;
        uint64_t self;
        uint64_t min;
        uint64_t max;
        uint64_t total;
      } prof_funcinfo_t;

      /// entry - Top-level program entry point.
      uword_t entry;

      typedef std::map<uword_t, prof_funcinfo_t> cycles_map_t;
      
      /// cycles_map - Map of function addr -> cycle count.
      cycles_map_t cycles_map;

      /// stack - Call stack.
      std::vector<uword_t> stack;

      /// last_cycle - Value of the cycle counter on last update.
      uint64_t last_cycle;
      
      /// reset_cycle - Value of the cycle counter on last reset or 
      ///               initialization.
      uint64_t reset_cycle;

    public:
      /// Constructor
      profiling_t() : last_cycle(0), reset_cycle(0)
      {
      }

      /// initialize - Initialize profiling.
      void initialize(uword_t entry, uint64_t cycle=0);

      /// finalize - Finalize profiling
      void finalize(uint64_t cycle);

      /// empty - Returns tue if there is no profiling information collected.
      bool empty() const;

      /// enter - Enter a function with a given base address.
      void enter(uword_t addr, uint64_t cycle);

      /// leave - Leave current function.
      void leave(uint64_t cycle);

      /// print - Print profiling information to a given stream, using
      /// given symbols.
      std::ostream &print(std::ostream &os, symbol_map_t &sym) const;
      
      void reset_stats(uint64_t cycle);
  };

} // end namespace patmos


#endif // PATMOS_PROFILING_H

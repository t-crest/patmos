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

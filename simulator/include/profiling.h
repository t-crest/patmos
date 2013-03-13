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

//#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
#include <map>


#include <boost/format.hpp>

#include "symbol.h"

namespace patmos
{
  /// Profiling information for functions
  class profiling_t
  {
    private:
      /// cycles_map - Map of function addr -> cycle count.
      std::map<uword_t, uint64_t> cycles_map;

      /// stack - Call stack.
      std::vector<uword_t> stack;

      /// last_cycle - Value of the cycle counter on last update.
      uint64_t last_cycle;

    public:
      /// Constructor
      profiling_t() : last_cycle(0)
      {
      }

      /// initialize - Initialize profiling.
      void initialize(uword_t entry, uint64_t cycle=0)
      {
        enter(entry, cycle);
      }

      /// finalize - Finalize profiling
      void finalize(uint64_t cycle)
      {
        // empty the callstack
        while (!stack.empty()) {
          leave(cycle);
        }
      }

      /// empty - Returns tue if there is no profiling information collected.
      bool empty() const
      {
        return cycles_map.empty();
      }

      /// enter - Enter a function with a given base address.
      void enter(uword_t addr, uint64_t cycle)
      {
        //std::cerr << "PUSH " << std::hex << addr << "\n";
        // create entry for function on demand
        if (!cycles_map.count(addr)) {
          cycles_map[addr] = 0;
        }
        // add cycles to caller (current function)
        if (!stack.empty()) {
          cycles_map[stack.back()] += cycle-last_cycle;
        }
        // switch to callee
        stack.push_back(addr);
        // update last_cycle
        last_cycle = cycle;
      }

      /// leave - Leave current function.
      void leave(uint64_t cycle)
      {
        //std::cerr << "POP " << std::hex << stack.back() << "\n";
        // add cycles to callee (current function)
        cycles_map[stack.back()] += cycle-last_cycle;
        // return to caller
        stack.pop_back();
        // update last_cycle
        last_cycle = cycle;
      }

      std::ostream &print(std::ostream &os, symbol_map_t &sym) const
      {
        uint64_t total = 0;
        for(std::map<uword_t, uint64_t>::const_iterator i = cycles_map.begin(),
                                                  e = cycles_map.end();
                                                  i != e; ++i)
        {
          total += i->second;
        }

        os << "\n\nProfiling information:\n\n"
              "  Function                                 "
              "cycles (abs)    cycles (rel)\n";

        for(std::map<uword_t, uint64_t>::const_iterator i = cycles_map.begin(),
                                                  e = cycles_map.end();
                                                  i != e; ++i)
        {
          std::stringstream func_name;

          if (sym.contains(i->first)) {
            func_name << sym.find(i->first);
          } else {
            func_name << boost::format("%d") % i->first;
          }

          os << boost::format("  %s: %|35t|%20d        %7.4f%%\n")
            % func_name.str()
            % i->second
            % ((double)(i->second*100.0) / (double)total);
        }
        os << "\n";
      }
  };
}


#endif // PATMOS_PROFILING_H

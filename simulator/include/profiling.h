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

      /// counter - Pointer to current cycle counter.
      uint64_t *counter;

    public:
      /// Constructor
      profiling_t() : counter(0)
      {
      }

      /// empty - Returns tue if there is no profiling information collected.
      bool empty() const
      {
        return cycles_map.empty();
      }

      /// call - Call a function with a given base address.
      void call(uword_t addr)
      {
        if (!cycles_map.count(addr)) {
          cycles_map[addr] = 0;
        }
        stack.push_back(addr);
        counter = &cycles_map[addr];
      }

      /// ret - Leave current function.
      void ret()
      {
        stack.pop_back();
        counter = &cycles_map[stack.back()];
      }

      /// bump - Bump cycle counter for current function.
      void bump()
      {
        if (counter) {
          (*counter)++;
        }
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

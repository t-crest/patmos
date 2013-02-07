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
// Provide information to map addresses to symbols.
//

#include "symbol.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace patmos
{
  static bool operator <(const symbol_info_t &a, const symbol_info_t &b)
  {
    if (a.Address == b.Address)
      return a.Size < b.Size;
    else
      return a.Address < b.Address;
  }

  void symbol_map_t::sort()
  {
    std::sort(Symbols.begin(), Symbols.end());
    Is_sorted = true;
  }

  void symbol_map_t::add(const symbol_info_t &symbol)
  {
    Symbols.push_back(symbol);
    Is_sorted = false;
  }

  std::string symbol_map_t::find(word_t address) const
  {
    std::stringstream ss;
    print(ss, address);
    return ss.str();
  }

  std::ostream &symbol_map_t::print(std::ostream &os, word_t address) const
  {
    assert(Is_sorted);

    // ok, ignore this one.
    if (address == 0)
      return os;

    // find enclosing symbol
    // TODO; use binary search here
    const symbol_info_t *enclosing = NULL;
    const symbol_info_t *bb = NULL;
    for(symbols_t::const_iterator i(Symbols.begin()), ie(Symbols.end());
        i != ie; i++)
    {
      if (i->Address <= address && address < i->Address + i->Size &&
          i->Size != 0)
      {
        os << (enclosing ? ':' : '<') << i->Name;
        enclosing = &*i;
      }
      else if (enclosing && i->Address <= address && i->Size == 0)
      {
        bb = &*i;
      }
      else if (i->Address == address)
      {
        assert(!enclosing);
	os << '<' << i->Name;
        enclosing = &*i;
      }
      else if (address < i->Address)
        break;
    }

    // print the symbol information
    word_t offset = 0;
    if (enclosing)
    {
      offset = address - enclosing->Address;
    }

    if (bb)
    {
      assert(enclosing);
      os << ':' << bb->Name;
      offset = address - bb->Address;
    }

    if (enclosing)
    {
      if (offset)
      {
        os << " + 0x" << std::hex << offset;
      }
      os << '>';
    }

    return os;
  }
}

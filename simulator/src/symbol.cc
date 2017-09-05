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
// Provide information to map addresses to symbols.
//

#include "symbol.h"

#include <cassert>

#include <algorithm>
#include <sstream>

#include <iostream>
#include <boost/format.hpp>

namespace patmos
{
  bool reloc_info_t::get_value(symbol_map_t &symbols, word_t &value,
                               word_t PC) const
  {
    word_t result = symbols.find(SymA);

    if (result == -1) {
      return false;
    }

    if (!SymB.empty()) {
      word_t valueB = symbols.find(SymB);
      if (valueB == -1) {
        return false;
      }
      result -= valueB;
    }

    if (Relative) {
      // PC is always in bytes!
      result -= PC;
    }

    // Shift to correct units of immediate
    result = (result >> Shift);

    // Should we apply the shift to the addend as well?? For pure constants, we
    // do *not* shift. For expressions like 'x - 4' it may make sense to have
    // them in bytes.. Now we have the addend in the unit of pure constants.
    result += Addend;

    value = Size < 32 ? (result & ((1 << Size)-1)) : result;
    value <<= Offset;

    return true;
  }

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

  bool symbol_map_t::contains(word_t address) const
  {
    symbol_info_t val(address, 0, false, "");

    return std::binary_search(Symbols.begin(), Symbols.end(), val);
  }

  bool symbol_map_t::contains(std::string symbol) const
  {
    return find(symbol) != -1;
  }


  bool symbol_map_t::covers(word_t symbol, word_t address) const
  {
    if (address < symbol) return false;

    symbol_info_t val(symbol, 0, false, "");
    symbols_t::const_iterator pos = std::lower_bound(Symbols.begin(), Symbols.end(), val);
    while (pos != Symbols.end()) {
      if (pos->Address != symbol) return false;
      if (pos->Address <= address && address < pos->Address + pos->Size)
        return true;
      pos++;
    }

    return false;
  }

  std::string symbol_map_t::find(word_t address) const
  {
    std::stringstream ss;
    print(ss, address);
    return ss.str();
  }

  word_t symbol_map_t::find(std::string symbol) const
  {
    for (symbols_t::const_iterator i(Symbols.begin()), ie(Symbols.end());
         i != ie; i++)
    {
      if (i->Name == symbol)
        return i->Address;
    }
    return -1;
  }

  std::ostream &symbol_map_t::print(std::ostream &os, word_t address, bool func_only) const
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
        if (!func_only || i->IsFunc) {
          os << (enclosing ? ':' : '<') << i->Name;
          enclosing = &*i;
        }
      }
      else if (enclosing && i->Address <= address && i->Size == 0)
      {
        // Found a BB symbol inside a function
        if (!func_only) {
          if (bb && bb->Address == address) {
            os << ":" << bb->Name;
          }
          bb = &*i;
        }
      }
      else if (i->Address == address)
      {
        assert(!enclosing);
        if (!func_only || i->IsFunc) {
	  os << '<' << i->Name;
          enclosing = &*i;
        }
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
      if (offset && !func_only)
      {
        os << " + 0x" << std::hex << offset << std::dec;
      }
      os << '>';
    }

    return os;
  }
}

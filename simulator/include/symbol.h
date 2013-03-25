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

#ifndef PATMOS_SYMBOL_H
#define PATMOS_SYMBOL_H

#include "basic-types.h"

#include <string>

#include <boost/tuple/tuple.hpp>

namespace patmos
{
  /// Symbol information, start address, size
  struct symbol_info_t
  {
  public:
    /// Address of the symbol.
    word_t Address;

    /// Size in bytes occupied by the symbol.
    word_t Size;

    /// If the symbol is a function
    bool IsFunc;

    /// Name of the symbol.
    std::string Name;

    /// construct a symbol info record.
    symbol_info_t(word_t address, word_t size, bool isfunc,
        const std::string &name) :
      Address(address), Size(size), IsFunc(isfunc), Name(name)
    {
    }

  };

  /// Provide a mapping from addresses to symbol names.
  class symbol_map_t
  {
  private:

    /// List of symbol names.
    typedef std::vector<symbol_info_t> symbols_t;

    /// List of known symbols, The list is sorted after initialization.
    symbols_t Symbols;

    /// Flag indicating whether the symbol list is sorted.
    bool Is_sorted;
  public:
    /// Construct a symbol map.
    symbol_map_t() : Is_sorted(true)
    {
    }

    /// Sort the symbol list.
    void sort();

    /// Append an entry to the symbol map.
    /// the entry is simply appended at the end of the map, i.e., the sort
    /// function has to be invoked once all symbols were added to the map.
    /// @param symbol The symbol to add.
    void add(const symbol_info_t &symbol);

    /// Check if the map contains a symbol for an address.
    /// @param address The address for which the symbol should be searched.
    /// @return True if the map contains at least one symbol for that address.
    bool contains(word_t address) const;

    /// Find a symbol given a specific address.
    /// \see print
    /// @param address The address for which symbol information should be
    /// retrieved.
    /// @return A string representing symbol information.
    std::string find(word_t address) const;

    /// Print symbol information given a specific address to a stream.
    /// The symbol table is searched for the symbol covering the address and the
    /// name of the symbol is returned. In case labels of basic blocks within
    /// functions are available the name of the enclosing function and the name
    /// of the basic block are concatenated, e.f., for basic block 'bar' in
    /// function 'foo' 'foo:bar' is returned.
    /// @param os An output stream.
    /// @param address The address for which symbol information should be
    /// retrieved.
    /// @param func_only If true, print only symbols with function attribute
    /// @return A string representing symbol information.
    std::ostream &print(std::ostream &os, word_t address, bool func_only = false) const;
  };
}

#endif // PATMOS_SYMBOL_H



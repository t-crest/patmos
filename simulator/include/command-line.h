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
// Helper to parse and print command-line options, e.g., for memory/cache sizes 
// using unit prefixes.
//

#ifndef PATMOS_COMMAND_LINE_H
#define PATMOS_COMMAND_LINE_H

#include "symbol.h"

#include <istream>
#include <ostream>
#include <boost/iterator/iterator_concepts.hpp>

namespace patmos
{
  /// Parsing debug output format options from the command-line.
  enum debug_format_e
  {
    DF_SHORT,
    DF_TRACE,
    DF_INSTRUCTIONS,
    DF_BLOCKS,
    DF_CALLS,
    DF_DEFAULT,
    DF_LONG,
    DF_ALL
  };

  /// Parse a debug output format from a string in a stream
  /// @param in An input stream to read from.
  /// @param df The debug format.
  std::istream &operator >>(std::istream &in, debug_format_e &df);

  /// Write a debug format option as a string to an output stream.
  /// @param os An output stream.
  /// @param df The debug format.
  std::ostream &operator <<(std::ostream &os, debug_format_e df);

  /// Parsing set-associative cache kinds as command-line options.
  enum set_assoc_policy_e
  {
    SAC_IDEAL,
    SAC_NO,
    SAC_DM,
    SAC_LRU,
    SAC_FIFO
  };
  struct set_assoc_cache_type
  {
    set_assoc_policy_e policy;
    unsigned associativity;
    set_assoc_cache_type() {};
    set_assoc_cache_type(set_assoc_policy_e policy_, unsigned associativity_) :
      policy(policy_), associativity(associativity_) {};
  };

  /// Parse a set-associative cache kind from a string in a stream
  /// @param in An input stream to read from.
  /// @param dck The set-associative cache kind.
  std::istream &operator >>(std::istream &in, set_assoc_cache_type &dck);

  /// Write a set-associative cache kind as a string to an output stream.
  /// @param os An output stream.
  /// @param mck The set-associative cache kind.
  std::ostream &operator <<(std::ostream &os, set_assoc_cache_type dck);

  /// Parsing instruction cache kinds as command-line options.
  enum instr_cache_e
  {
    IC_MCACHE,
    IC_ICACHE
  };

  /// Parse a instruction cache kind from a string in a stream
  /// @param in An input stream to read from.
  /// @param mck The method cache kind.
  std::istream &operator >>(std::istream &in, instr_cache_e &ick);

  /// Write a instruction cache kind as a string to an output stream.
  /// @param os An output stream.
  /// @param mck The method cache kind.
  std::ostream &operator <<(std::ostream &os, instr_cache_e ick);

  /// Parsing method cache kinds as command-line options.
  enum method_cache_e
  {
    MC_IDEAL,
    MC_LRU,
    MC_FIFO
  };

  /// Parse a method cache kind from a string in a stream
  /// @param in An input stream to read from.
  /// @param mck The method cache kind.
  std::istream &operator >>(std::istream &in, method_cache_e &mck);

  /// Write a method cache kind as a string to an output stream.
  /// @param os An output stream.
  /// @param mck The method cache kind.
  std::ostream &operator <<(std::ostream &os, method_cache_e mck);

  /// Parsing stack cache kinds as command-line options.
  enum stack_cache_e
  {
    SC_IDEAL,
    SC_BLOCK,
    SC_DCACHE,
    SC_LBLOCK
  };

  /// Parse a stack cache kind from a string in a stream
  /// @param in An input stream to read from.
  /// @param sck The stack cache kind.
  std::istream &operator >>(std::istream &in, stack_cache_e &sck);

  /// Write a stack cache kind as a string to an output stream.
  /// @param os An output stream.
  /// @param sck The stack cache kind.
  std::ostream &operator <<(std::ostream &os, stack_cache_e sck);

  /// Parsing memory/cache sizes as command-line options.
  class byte_size_t
  {
    private:
      unsigned int V;

    public:
      /// Construct a new byte size.
      /// @param v The initial value.
      byte_size_t(unsigned int v = 0) : V(v)
      {
      }

      /// Return the value of the byte size object.
      /// @return The value of the byte size object.
      unsigned int value() const
      {
        return V;
      }
  };

  /// Read the size of a memory or cache in bytes, allowing unit prefixes k, m,
  /// and g, or kb, mb, and gb, in all combinations of lower and upper-case
  /// letters.
  /// @param in An input stream to read from.
  /// @param bs The result size in bytes.
  std::istream &operator >>(std::istream &in, byte_size_t &bs);

  /// Write the size of a memory or cache in bytes, using unit prefixes k, m, and
  /// g.
  /// @param os An output stream.
  /// @param bs The size in bytes.
  std::ostream &operator <<(std::ostream &os, const byte_size_t &bs);
  
    /// Parsing addresses as command-line options.
  class address_t
  {
    private:
      unsigned int V;
      
      std::string symbol;

    public:
      /// Construct a new byte address.
      /// @param v The initial value.
      address_t(unsigned int v = 0) : V(v)
      {
      }

      void set_symbol(const std::string &sym) {
        symbol = sym;
      }
      
      /// Return the value of the address object.
      /// @return The value of the address object.
      unsigned int value() const
      {
        return V;
      }
      
      unsigned int parse(symbol_map_t &symbols)
      {
        if (V) return V;
        V = symbols.find(symbol);
        return V;
      }
  };

  /// Read an address as hex or decimal
  /// @param in An input stream to read from.
  /// @param a The result address.
  std::istream &operator >>(std::istream &in, address_t &a);

  /// Write an address as hex number.
  /// @param os An output stream.
  /// @param a The address.
  std::ostream &operator <<(std::ostream &os, const address_t &a);
}

#endif // PATMOS_COMMAND_LINE_H


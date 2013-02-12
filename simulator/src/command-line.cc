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

#include "command-line.h"

#include <algorithm>
#include <cctype>

#include <boost/program_options.hpp>

namespace patmos
{
  std::istream &operator >>(std::istream &in, debug_format_e &df)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "short")
      df = DF_SHORT;
    else if(kind == "trace")
      df = DF_TRACE;
    else if(kind == "instr")
      df = DF_INSTRUCTIONS;
    else if (kind == "blocks")
      df = DF_BLOCKS;
    else if(kind == "trace-stack")
      df = DF_TRACE_STACK;
    else if(kind == "default")
      df = DF_DEFAULT;
    else if(kind == "long")
      df = DF_LONG;
    else if(kind == "all")
      df = DF_ALL;
    else throw boost::program_options::validation_error(
                  boost::program_options::validation_error::invalid_option_value,
                  "Unknown debug output option: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, debug_format_e df)
  {
    switch(df)
    {
      case DF_SHORT:
        os << "short"; break;
      case DF_TRACE:
        os << "trace"; break;
      case DF_INSTRUCTIONS:
        os << "instr"; break;
      case DF_BLOCKS:
	os << "blocks"; break;
      case DF_TRACE_STACK:
        os << "trace-stack"; break;
      case DF_DEFAULT:
        os << "default"; break;
      case DF_LONG:
        os << "long"; break;
      case DF_ALL:
        os << "all"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, data_cache_e &dck)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "ideal")
      dck = DC_IDEAL;
    else if(kind == "no")
      dck = DC_NO;
    else if(kind == "lru2")
      dck = DC_LRU2;
    else if(kind == "lru4")
      dck = DC_LRU4;
    else if(kind == "lru8")
      dck = DC_LRU8;
    else throw boost::program_options::validation_error(
                 boost::program_options::validation_error::invalid_option_value,
                 "Unknown data cache kind: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, data_cache_e dck)
  {
    switch(dck)
    {
      case DC_IDEAL:
        os << "ideal"; break;
      case DC_NO:
        os << "no"; break;
      case DC_LRU2:
        os << "lru2"; break;
      case DC_LRU4:
        os << "lru4"; break;
      case DC_LRU8:
        os << "lru8"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, method_cache_e &mck)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "ideal")
      mck = MC_IDEAL;
    else if(kind == "lru")
      mck = MC_LRU;
    else if(kind == "fifo")
      mck = MC_FIFO;
    else throw boost::program_options::validation_error(
                 boost::program_options::validation_error::invalid_option_value,
                 "Unknown method cache kind: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, method_cache_e mck)
  {
    switch(mck)
    {
      case MC_IDEAL:
        os << "ideal"; break;
      case MC_LRU:
        os << "lru"; break;
      case MC_FIFO:
        os << "fifo"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, stack_cache_e &sck)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "ideal")
      sck = SC_IDEAL;
    else if(kind == "block")
      sck = SC_BLOCK;
    else throw boost::program_options::validation_error(
                 boost::program_options::validation_error::invalid_option_value,
                 "Unknown stack cache kind: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, stack_cache_e sck)
  {
    switch(sck)
    {
      case SC_IDEAL:
        os << "ideal"; break;
      case SC_BLOCK:
        os << "block"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, byte_size_t &bs)
  {
    unsigned int v;
    in >> v;

    std::string tmp, unit;
    if (!in.eof())
    {
      in >> tmp;

      unit.resize(tmp.size());
      std::transform(tmp.begin(), tmp.end(), unit.begin(), ::tolower);
    }

    if (unit.empty())
      bs = v;
    else if (unit == "k" || unit == "kb")
      bs = v << 10;
    else if (unit == "m" || unit == "mb")
      bs = v << 20;
    else if (unit == "g" || unit == "gb")
      bs = v << 30;
    else throw boost::program_options::validation_error(
                  boost::program_options::validation_error::invalid_option_value,
                  "Unknown unit: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, const byte_size_t &bs)
  {
    unsigned int v = bs.value();

    if ((v & 0x3fffffff) == 0)
      os << (v >> 10) << "g";
    else if ((v & 0xfffff) == 0)
      os << (v >> 20) << "m";
    else if ((v & 0x3ff) == 0)
      os << (v >> 10) << "k";
    else
      os << v;

    return os;
  }
}

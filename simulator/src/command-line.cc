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
// Helper to parse and print command-line options, e.g., for memory/cache sizes
// using unit prefixes.
//

#include "command-line.h"

#include <algorithm>
#include <cctype>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

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
    else if (kind == "calls")
      df = DF_CALLS;
    else if (kind == "calls-indent")
      df = DF_CALLS_INDENT;
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
      case DF_CALLS:
        os << "calls"; break;
      case DF_CALLS_INDENT:
        os << "calls-indent"; break;
      case DF_DEFAULT:
        os << "default"; break;
      case DF_LONG:
        os << "long"; break;
      case DF_ALL:
        os << "all"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, debug_cache_e &dc)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "none")
      dc = DC_NONE;
    else if(kind == "miss")
      dc = DC_MISS;
    else if(kind == "all")
      dc = DC_ALL;
    else throw boost::program_options::validation_error(
                  boost::program_options::validation_error::invalid_option_value,
                  "Unknown debug-cache option: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, debug_cache_e dc)
  {
    switch(dc)
    {
      case DC_NONE:
        os << "none"; break;
      case DC_MISS:
        os << "miss"; break;
      case DC_ALL:
        os << "all"; break;
    }
    return os;
  }

  std::istream &operator >>(std::istream &in, mem_check_e &mck)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "none")
      mck = MCK_NONE;
    else if(kind == "warn")
      mck = MCK_WARN;
    else if(kind == "err" || kind == "error")
      mck = MCK_ERROR;
    else if (kind == "warn-addr")
      mck = MCK_WARN_ADDR;
    else if (kind == "err-addr" || kind == "error-addr")
      mck = MCK_ERROR_ADDR;
    else throw boost::program_options::validation_error(
                  boost::program_options::validation_error::invalid_option_value,
                  "Unknown mem-check option: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, mem_check_e mck)
  {
    switch(mck)
    {
      case MCK_NONE:
        os << "none"; break;
      case MCK_WARN:
        os << "warn"; break;
      case MCK_ERROR:
        os << "err"; break;
      case MCK_WARN_ADDR:
        os << "warn-addr"; break;
      case MCK_ERROR_ADDR:
        os << "err-addr"; break;
    }

    return os;
  }

  std::istream &operator >>(std::istream &in, set_assoc_cache_type &dck)
  {
    std::string tmp, kind;
    std::string assoc = "1";
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "ideal")
    {
      dck.policy = SAC_IDEAL;
    }
    else if(kind == "no")
    {
      dck.policy = SAC_NO;
    }
    else if(kind == "dm")
    {
      dck.policy = SAC_DM;
    }
    else if(kind == "lru")
    {
      dck.policy = SAC_LRU;
      dck.associativity = 0;
      return in;
    }
    else if(kind == "fifo")
    {
      dck.policy = SAC_FIFO;
      dck.associativity = 0;
      return in;
    }
    else if(kind.substr(0,3) == "lru")
    {
      dck.policy = SAC_LRU;
      assoc = kind.substr(3,8);
    }
    else if(kind.substr(0,4) == "fifo")
    {
      dck.policy = SAC_FIFO;
      assoc = kind.substr(4,8);
    }
    else
    {
      throw boost::program_options::validation_error(
        boost::program_options::validation_error::invalid_option_value,
        "Unknown set-associative cache kind: " + tmp);
    }
    std::istringstream is(assoc);
    is >> dck.associativity;
    const unsigned max_assoc = 256;
    unsigned valid_assoc;
    for (valid_assoc = 1; valid_assoc <= max_assoc; valid_assoc *= 2)
    {
      if (dck.associativity == valid_assoc)
        break;
    }
    if (valid_assoc > max_assoc)
      throw boost::program_options::validation_error(
        boost::program_options::validation_error::invalid_option_value,
        "Invalid associativity (power of two <= 256)" + tmp);
    return in;
  }

  std::ostream &operator <<(std::ostream &os, set_assoc_cache_type dck)
  {
    switch(dck.policy)
    {
      case SAC_IDEAL:
        os << "ideal"; break;
      case SAC_NO:
        os << "no"; break;
      case SAC_DM:
        os << "dm"; break;
      case SAC_LRU:
        os << "lru";
        if (dck.associativity) os << dck.associativity;
        break;
      case SAC_FIFO:
        os << "fifo";
        if (dck.associativity) os << dck.associativity;
        break;
    }

    return os;
  }

    std::istream &operator >>(std::istream &in, instr_cache_e &ick)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "mcache")
      ick = IC_MCACHE;
    else if(kind == "icache")
      ick = IC_ICACHE;
    else throw boost::program_options::validation_error(
                 boost::program_options::validation_error::invalid_option_value,
                 "Unknown instruction cache kind: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, instr_cache_e ick)
  {
    switch(ick)
    {
      case IC_MCACHE:
        os << "mcache"; break;
      case IC_ICACHE:
        os << "icache"; break;
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

  std::istream &operator >>(std::istream &in, main_memory_kind_e &gmk)
  {
    std::string tmp, kind;
    in >> tmp;

    kind.resize(tmp.size());
    std::transform(tmp.begin(), tmp.end(), kind.begin(), ::tolower);

    if(kind == "simple")
      gmk = GM_SIMPLE;
    else if(kind == "ddr3")
      gmk = GM_RAMUL_DDR3;
    else if(kind == "ddr4")
      gmk = GM_RAMUL_DDR4;
    else if(kind == "lpddr3")
      gmk = GM_RAMUL_LPDDR3;
    else if(kind == "lpddr4")
      gmk = GM_RAMUL_LPDDR4;
    else throw boost::program_options::validation_error(
                 boost::program_options::validation_error::invalid_option_value,
                 "Unknown main memory kind: " + tmp);

    return in;
  }

  std::ostream &operator <<(std::ostream &os, main_memory_kind_e gmk)
  {
    switch(gmk)
    {
      case GM_SIMPLE:
        os << "simple"; break;
      case GM_RAMUL_DDR3:
        os << "ddr3"; break;
      case GM_RAMUL_DDR4:
        os << "ddr4"; break;
      case GM_RAMUL_LPDDR3:
        os << "lpddr3"; break;
      case GM_RAMUL_LPDDR4:
        os << "lpddr4"; break;
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
    else if(kind == "dcache")
      sck = SC_DCACHE;
    else if(kind == "ablock")
      sck = SC_ABLOCK;
    else if(kind == "lblock")
      sck = SC_LBLOCK;
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
      case SC_DCACHE:
        os << "dcache"; break;
      case SC_ABLOCK:
        os << "ablock"; break;
      case SC_LBLOCK:
        os << "lblock"; break;
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

    if (v == 0)
      os << "0";
    else if ((v & 0x3fffffff) == 0)
      os << (v >> 10) << "g";
    else if ((v & 0xfffff) == 0)
      os << (v >> 20) << "m";
    else if ((v & 0x3ff) == 0)
      os << (v >> 10) << "k";
    else
      os << v;

    return os;
  }

  std::istream &operator >>(std::istream &in, address_t &a)
  {
    unsigned int v;

    std::string tmp;
    in >> tmp;

    std::stringstream s;
    if (tmp.size() > 2 && tmp.substr(0, 2) == "0x") {
      s << std::hex << tmp.substr(2);
    } else if (tmp.size() > 1 && tmp.substr(0, 1) == "0") {
      // TODO this might be misleading!! warn about that
      s << std::oct << tmp.substr(1);
    } else if (tmp[0] >= '0' && tmp[0] <= '9') {
      s << tmp;
    } else {
      // We do not know how to resolve the symbol yet..
      a.set_symbol(tmp);
      return in;
    }

    s >> v;
    a = v;

    return in;
  }

  std::ostream &operator <<(std::ostream &os, const address_t &a)
  {
    unsigned int v = a.value();

    os << boost::format("0x%1$x") % v;

    return os;
  }
}

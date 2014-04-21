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
// Basic interface to read binary files.
//


#ifndef PATMOS_LOADER_H
#define PATMOS_LOADER_H

#include "memory.h"
#include "symbol.h"

#include <vector>
#include <iostream>

#include <gelf.h>

#include <boost/concept_check.hpp>

namespace patmos
{
  class simulator_t;
  
  struct section_info_t 
  {
    /// Offset in the binary of the section
    uword_t offset;
    
    /// Address in memory of the section
    uword_t addr;
    
    /// Size of the section
    uword_t size;
    
    section_info_t(uword_t offset, uword_t addr, uword_t size) 
    : offset(offset), addr(addr), size(size) 
    {}
  };
  
  /// List of sections as (start-address, end-address) pairs. End-address is 
  /// the next address after the last valid address in the section.
  typedef std::vector<section_info_t> section_list_t;
  
  class loader_t 
  {
  private:
    bool Is_ELF;
    
  protected:
    std::vector<char> buf;
    
    loader_t(std::istream &is, bool elf);
      
  public:
    virtual ~loader_t() {}
    
    bool is_ELF() { return Is_ELF; }
    
    uword_t get_binary_size() { return buf.size(); }
    
    /// @return The entry point of the elf executable.
    virtual uword_t get_program_entry() = 0;
    
    /// @param symbols Map to store symbol information, if available.
    /// @param text List of text sections in memory.
    virtual void load_symbols(symbol_map_t &sym, section_list_t &text) = 0;
    
    /// @param m The main memory to load to.
    virtual void load_to_memory(simulator_t &s, memory_t &m) = 0;
    
    /// Read a word from the binary and convert it to big-endian.
    /// @param offset the offset in the binary file to read from.
    virtual uword_t read_word(uword_t offset);
  };
  
  class elf_loader_t : public loader_t 
  {
  private:
    Elf *elf;
    
    uword_t entry;
    
  public:
    explicit elf_loader_t(std::istream &is);
    virtual ~elf_loader_t();
    
    virtual uword_t get_program_entry() { return entry; }
    
    virtual void load_symbols(symbol_map_t &sym, section_list_t &text);
    
    virtual void load_to_memory(simulator_t &s, memory_t &m);    
  };
  
  class bin_loader_t : public loader_t
  {
  public:
    explicit bin_loader_t(std::istream &is) : loader_t(is, false) {}
    
    virtual uword_t get_program_entry() { return 0x4; }
    
    virtual void load_symbols(symbol_map_t &sym, section_list_t &text);
    
    virtual void load_to_memory(simulator_t &s, memory_t &m);
  };
  
  bool is_elf(std::istream &is);
  
  /// Create a new file loader from a stream.
  /// @param is The input stream to read from.
  loader_t *create_loader(std::istream &is);
}

#endif // PATMOS_LOADER_H

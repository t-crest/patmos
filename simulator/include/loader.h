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

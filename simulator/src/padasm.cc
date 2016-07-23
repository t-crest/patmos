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
// Main file of a simple Patmos disassembler.
//

#include "basic-types.h"
#include "decoder.h"
#include "instruction.h"
#include "streams.h"
#include "symbol.h"
#include "loader.h"
#include "decoder.h"

#include <boost/format.hpp>

#include <fstream>
#include <iostream>

class instruction_printer_t : public patmos::decoder_callback_t
{
private:
  patmos::uword_t num_errors;
  std::ostream &out;
  
public:
  instruction_printer_t(std::ostream &out) : num_errors(0), out(out) {}
  
  patmos::uword_t get_errors() { return num_errors; }
  
  virtual int process_bundle(patmos::uword_t addr, 
                             patmos::instruction_data_t *bundle, 
                             unsigned slots, patmos::symbol_map_t &symbols)
  {
    if (slots == 0) {
      num_errors ++;
      return 1;
    }
    // decode bundle
    out << boost::format("  0x%1$08x:  ") % addr;

    for (unsigned int i = 0; i < slots; i++) {
      if (i > 0) out << " || ";
      bundle[i].print(out, symbols);  
    }
    out << ";\n";
    
    return 0;
  }
};

int main(int argc, char **argv)
{
  patmos::decoder_t padasm;
  patmos::symbol_map_t symbols;
  patmos::section_list_t text;

  patmos::uword_t words = 0;
  int retcode = 0;
    
  // check arguments
  if (argc != 3)
  {
    std::cerr << "Usage: padasm <input> <output>\n";
    return 1;
  }

  // open streams
  try
  {
    std::istream &in = *patmos::get_stream<std::ifstream>(argv[1], std::cin);
    std::ostream &out = *patmos::get_stream<std::ofstream>(argv[2], std::cout);

    patmos::loader_t *loader = patmos::create_loader(in);
    loader->load_symbols(symbols, text);

    patmos::decoder_t decoder;
    instruction_printer_t printer(out);
    
    for (patmos::section_list_t::iterator t = text.begin(), te = text.end(); 
         t != te; t++)
    {
      int rc = decoder.decode(*loader, *t, symbols, printer);
      if (rc != 0) retcode = rc;
      words += t->size;
    }
    
    // some status messages
    std::cerr << boost::format("Disassembled: %1% words\nErrors : %2%\n")
              % words % printer.get_errors();

    delete loader;
              
    // free streams
    patmos::free_stream(&in);
    patmos::free_stream(&out);
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
    return -1;
  }

  return retcode;
}

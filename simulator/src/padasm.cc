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

    for (int i = 0; i < slots; i++) {
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
    patmos::free_stream(&in, std::cin);
    patmos::free_stream(&out, std::cout);
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
    return -1;
  }

  return retcode;
}

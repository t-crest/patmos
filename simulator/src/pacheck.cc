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
// Main file of a simple Patmos binary checker.
//

#include "basic-types.h"
#include "decoder.h"
#include "instruction.h"
#include "streams.h"
#include "symbol.h"
#include "memory.h"
#include "loader.h"

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>

using namespace patmos;

class instruction_checker_t : public decoder_callback_t
{
  int delayslot;
  bool is_branch_delayslot;
  bool is_call_delayslot;
  
  unsigned errors;
public:
  instruction_checker_t()
  : delayslot(0), is_branch_delayslot(false), is_call_delayslot(false),
    errors(0)
  {}
  
  void print_error(uword_t addr, std::string msg) {
    std::cerr << " 0x" << std::hex << addr << std::dec << ": " << msg << "\n";
    errors++;
  }
  
  /// Check a stream of bundles for errors, either from a binary or from a trace
  virtual int process_bundle(uword_t addr, instruction_data_t *bundle, 
                             unsigned slots, symbol_map_t &sym)
  {
    if (slots == 0) return 1;
    bool error = false;
    
    if (delayslot > 1) {
      delayslot--;
    } else {
      delayslot = 0;
      is_branch_delayslot = false;
      is_call_delayslot = false;
    }
    
    // check for errors in delay slots
    if (bundle[0].I) {
      if (bundle[0].I->is_flow_control()) {
        if (is_branch_delayslot && delayslot > 0) {
          print_error(addr, "Control flow instruction inside branch delay slot.");
          error = true;
        }
        is_branch_delayslot = true;
        is_call_delayslot = bundle[0].I->is_call();
        delayslot = bundle[0].I->get_delay_slots(bundle[0]) + 1;
      }
      
    }
    
    if (slots > 1 && !bundle[1].I && is_call_delayslot) {
      print_error(addr, "Long load found in call delay slot.");
    }
    if (slots > 1 && bundle[1].I) {
      if (bundle[1].I->is_flow_control()) {
        print_error(addr, "Control flow instruction in second slot.");
      }
      if (is_call_delayslot) {
        print_error(addr, "Instruction in second slot in call delay slot.");
      }
    }
    
    return error ? 1 :0;
  }
};


int main(int argc, char **argv)
{
  boost::program_options::options_description cmdline_options;
  cmdline_options.add_options()
    ("help,h", "produce help message")
    ("binary,b", boost::program_options::value<std::string>()->default_value("-"), "binary or elf-executable file (stdin: -)")
    ("verbose,v", "be verbosive");

  boost::program_options::positional_options_description pos;
  pos.add("binary", 1);

  // process command-line options
  boost::program_options::variables_map vm;
  try
  {
    boost::program_options::store(
                          boost::program_options::command_line_parser(argc, argv)
                            .options(cmdline_options).positional(pos).run(), vm);
    boost::program_options::notify(vm);

    // help message
    if (vm.count("help")) {
      std::cout << cmdline_options << "\n";
      return 1;
    }
  }
  catch(boost::program_options::error &e)
  {
    std::cerr << cmdline_options << "\n" << e.what() << "\n\n";
    return 1;
  }

  std::string binary  = vm["binary"].as<std::string>();
  bool        verbose = vm.count("verbose") != 0;
  
  // open streams
  try
  {
    symbol_map_t sym;
    section_list_t text;
    
    std::istream *in = get_stream<std::ifstream>(binary, std::cin);

    // load input program    
    loader_t *loader = create_loader(*in);
    loader->load_symbols(sym, text);

    // free streams
    free_stream(in);

    decoder_t decoder;
    instruction_checker_t instruction_checker;
    
    int retcode = 0;
    
    for (section_list_t::iterator t = text.begin(), te = text.end(); 
         t != te; t++)
    {
      int rc = decoder.decode(*loader, *t, sym, instruction_checker);
      if (rc != 0) retcode = rc;
    }
    
    delete loader;
    
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
    return -1;
  }

  return 0;
}

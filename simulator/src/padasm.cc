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

#include <boost/format.hpp>

#include <fstream>
#include <iostream>

/// Get a word from the input stream.
/// @param in The input stream.
/// @param w A reference where the read value should be stored to.
static void get_word(std::istream &in, patmos::word_t &w)
{
  in.read(reinterpret_cast<char*>(&w), sizeof(patmos::word_t));
}

int main(int argc, char **argv)
{
  bool fetch_full = true;
  unsigned int offset = 0;
  unsigned int num_errors = 0;

  patmos::decoder_t padasm;
  patmos::word_t bundle[2];
  patmos::instruction_data_t id[2];

  // check arguments
  if (argc != 3)
  {
    std::cerr << "Usage: padasm <input> <output>\n";
    return 1;
  }

  // open streams
  std::istream &in = patmos::get_stream<std::ifstream>(argv[1], std::cin);
  std::ostream &out = patmos::get_stream<std::ofstream>(argv[2], std::cout);

  while (!in.eof())
  {
    // read next bundle
    if(fetch_full)
    {
      get_word(in, bundle[0]);
      if (in.eof())
      {
        bundle[1] = 0;
      }
      else
      {
        get_word(in, bundle[1]);
      }
    }
    else
    {
      bundle[0] = bundle[1];
      get_word(in, bundle[1]);
    }

    // decode bundle
    switch (padasm.decode(bundle, id))
    {
      case 0:
        std::cerr << boost::format("Unknown instruction in bundle: "
                                   "0x%1$08x: 0x%2$08x\n")
                  % (offset * 4) % bundle[0];

        num_errors++;

        offset++;
        fetch_full = false;
        break;
      case 1:
        id[0].print(out);
        out << ";\n";

        offset++;
        fetch_full = false;
        break;
      case 2:
        id[0].print(out);
        out << " || ";
        id[1].print(out);
        out << ";\n";

        offset += 2;
        fetch_full = true;
        break;

      default:
        // we should never get here
        assert(false);
        abort();
    }
  }

  // some status messages
  std::cerr << boost::format("Disassembled: %1% words\nErrors : %2%\n")
            % offset % num_errors;

  // free streams
  patmos::free_stream(in, std::cin);
  patmos::free_stream(out, std::cout);

  return 0;
}

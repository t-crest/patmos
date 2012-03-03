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
// Main file of the Patmos Simulator.
//

#include "assembler.h"
#include "data-cache.h"
#include "instruction.h"
#include "method-cache.h"
#include "simulation-core.h"
#include "stack-cache.h"
#include "streams.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv)
{
  // setup simulation framework
  patmos::ideal_stack_cache_t isc;
  patmos::ideal_method_cache_t imc;
  patmos::ideal_memory_t imm(patmos::NUM_MEMORY_BYTES);
  patmos::ideal_data_cache_t idc(imm);
  patmos::ideal_memory_t ilm(patmos::NUM_LOCAL_MEMORY_BYTES);

  patmos::simulator_t s(imm, ilm, idc, imc, isc);


  // check arguments
  if (argc != 3)
  {
    std::cerr << "Usage: pasim <input> <output>\n";
    return 1;
  }

  // open streams
  std::istream &in = patmos::get_stream<std::ifstream>(argv[1], std::cin);
  std::ostream &out = patmos::get_stream<std::ofstream>(argv[2], std::cout);

  // load input program
  std::streamsize offset = 0;
  while (!in.eof())
  {
    patmos::byte_t buf[128];

    // read into buffer
    in.read(reinterpret_cast<char*>(&buf[0]), sizeof(buf));

    // check how much was read
    std::streamsize count = in.gcount();
    assert((count <= 128) && (offset + count < patmos::NUM_MEMORY_BYTES));

    // write into main memory
    imm.write_peek(offset, buf, count);

    offset += count;
  }

  // some output
  std::cerr << boost::format("Loaded: %1% bytes\n") % offset;

  try
  {
    s.run();
    s.print(out);
  }
  catch (patmos::simulation_exception_t e)
  {
    switch(e)
    {
      case patmos::ILLEGAL:
        std::cerr << boost::format("Illegal instruction: %1%: %2%\n")
                  % s.PC % s.Exception_status;
        break;
      case patmos::HALT:
        break;
      default:
        std::cerr << "Unknown simulation error.\n";
    }
  }
  // free streams
  patmos::free_stream(in, std::cin);
  patmos::free_stream(out, std::cout);

  return 0;
}
